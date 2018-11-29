#include "libavutil/opt.h"
#include "libavutil/imgutils.h"
#include "libavutil/avassert.h"
#include "libavfilter/avfilter.h"
#include "libavfilter/formats.h"
#include "libavfilter/internal.h"
#include "libavfilter/video.h"

#include "libavutil/pixfmt.h"
#include "libavcodec/avcodec.h"
#include "libavutil/frame.h"
#include "libavformat/avformat.h"
#include "libavfilter/scale.h"
#include "libavutil/parseutils.h"

#include <dlfcn.h>

typedef int(*PF_Scale_i420)(const uint8_t* src_y,
                            const uint8_t* src_u,
                            const uint8_t* src_v,
                            int src_width,
                            int src_height,
                            uint8_t* dst_y,
                            uint8_t* dst_u,
                            uint8_t* dst_v,
                            int dst_width,
                            int dst_height);

typedef struct ScaleContext {
    const AVClass *class;
    AVDictionary *opts;

    /**
    * New dimensions. Special values are:
    *   0 = original width/height
    *  -1 = keep original aspect
    *  -N = try to keep aspect but make sure it is divisible by N
    */
    int w, h;
    char *size_str;

    char *w_expr;               ///< width  expression string
    char *h_expr;               ///< height expression string
    char *flags_str;
} ScaleContext;

typedef struct ThreadData {
    AVFrame *in, *out;
} ThreadData;


static void *handle = NULL;
static PF_Scale_i420 pf_Scale_i420 = NULL;

static int do_conversion(AVFilterContext *ctx, void *arg, int jobnr,
                         int nb_jobs) {
    ThreadData *td = arg;
    AVFrame *dst = td->out;
    AVFrame *src = td->in;
    //YUV scale
    return pf_Scale_i420(src->data[0], src->data[1], src->data[2], src->width, src->height,
                         dst->data[0], dst->data[1], dst->data[2], dst->width, dst->height);
}

static int filter_frame(AVFilterLink *link, AVFrame *in) {
    AVFilterContext *avctx = link->dst;
    AVFilterLink *outlink = avctx->outputs[0];
    AVFilterContext *ctx = outlink->src;
    ScaleContext *scale = ctx->priv;
    AVFrame *out;
    ThreadData td;

    //same w and h
    if ((ctx->inputs[0]->w == scale->w) && (ctx->inputs[0]->h == scale->h)) {
        return ff_filter_frame(outlink, in);
    }

    //allocate a new buffer, data is null
    out = ff_get_video_buffer(outlink, outlink->w, outlink->h);
    if (!out) {
        av_frame_free(&in);
        return AVERROR(ENOMEM);
    }

    av_frame_copy_props(out, in);
    out->width = outlink->w;
    out->height = outlink->h;

    td.in = in;
    td.out = out;
    avctx->internal->execute(avctx, do_conversion, &td, NULL, FFMIN(outlink->h, avctx->graph->nb_threads));

    av_frame_free(&in);

    return ff_filter_frame(outlink, out);
}

static av_cold int config_props(AVFilterLink *outlink) {
    AVFilterContext *ctx = outlink->src;
    ScaleContext *scale = ctx->priv;
    AVFilterLink *inlink = outlink->src->inputs[0];
    int ret;

    if ((ret = ff_scale_eval_dimensions(ctx, scale->w_expr, scale->h_expr, inlink, outlink, &scale->w, &scale->h)) < 0) {
        return ret;
    }

    outlink->w = scale->w;
    outlink->h = scale->h;
    av_log(NULL, AV_LOG_INFO, "configure output, w h = (%d %d), format %d \n", outlink->w, outlink->h, outlink->format);

    return 0;
}

static int process_command(AVFilterContext *ctx, const char *cmd, const char *args,
                           char *res, int res_len, int flags) {
    ScaleContext *scale = ctx->priv;
    int ret;

    if (!strcmp(cmd, "width") || !strcmp(cmd, "w")
            || !strcmp(cmd, "height") || !strcmp(cmd, "h")) {

        int old_w = scale->w;
        int old_h = scale->h;
        AVFilterLink *outlink = ctx->outputs[0];

        av_opt_set(scale, cmd, args, 0);
        if ((ret = config_props(outlink)) < 0) {
            scale->w = old_w;
            scale->h = old_h;
        }
    } else
        ret = AVERROR(ENOSYS);

    return ret;
}

static av_cold int init_dict(AVFilterContext *ctx, AVDictionary **opts) {
    ScaleContext *scale = ctx->priv;
    int ret;

    const char *DLL_NAME = "libyuv_scale.so";
    handle = dlopen(DLL_NAME, RTLD_LAZY);
    if (NULL == handle) {
        av_log(NULL, AV_LOG_ERROR, "load library %s failed, error %s\n", DLL_NAME, dlerror());
        return AVERROR(EINVAL);
    }

    pf_Scale_i420 = (PF_Scale_i420)dlsym(handle, "Scale_i420");
    if (NULL == pf_Scale_i420) {
        av_log(NULL, AV_LOG_ERROR, "no symbol found in library, %p\n", pf_Scale_i420);
        dlclose(handle);
        return AVERROR(EINVAL);
    }

    if (scale->size_str && (scale->w_expr || scale->h_expr)) {
        av_log(ctx, AV_LOG_ERROR,
               "Size and width/height expressions cannot be set at the same time.\n");
        dlclose(handle);
        return AVERROR(EINVAL);
    }

    if (scale->w_expr && !scale->h_expr)
        FFSWAP(char *, scale->w_expr, scale->size_str);

    if (scale->size_str) {
        char buf[32];
        if ((ret = av_parse_video_size(&scale->w, &scale->h, scale->size_str)) < 0) {
            av_log(ctx, AV_LOG_ERROR,
                   "Invalid size '%s'\n", scale->size_str);
            return ret;
        }
        snprintf(buf, sizeof(buf) - 1, "%d", scale->w);
        av_opt_set(scale, "w", buf, 0);
        snprintf(buf, sizeof(buf) - 1, "%d", scale->h);
        av_opt_set(scale, "h", buf, 0);
    }
    if (!scale->w_expr)
        av_opt_set(scale, "w", "iw", 0);
    if (!scale->h_expr)
        av_opt_set(scale, "h", "ih", 0);

    av_log(ctx, AV_LOG_VERBOSE, "w:%s h:%s flags:'%s'\n",
           scale->w_expr, scale->h_expr, (char *)av_x_if_null(scale->flags_str, ""));

    scale->opts = *opts;
    *opts = NULL;

    return 0;
}

static av_cold void uninit(AVFilterContext *ctx) {
    ScaleContext *scale = ctx->priv;
    av_dict_free(&scale->opts);
    dlclose(handle);
}

//currently we just support the most common YUV420, can add more if needed
static int query_formats(AVFilterContext *ctx) {
    static const enum AVPixelFormat pix_fmts[] = {
        AV_PIX_FMT_YUV420P,
        AV_PIX_FMT_NONE
    };
    AVFilterFormats *fmts_list = ff_make_format_list(pix_fmts);
    if (!fmts_list)
        return AVERROR(ENOMEM);
    return ff_set_common_formats(ctx, fmts_list);
}


#define OFFSET(x) offsetof(ScaleContext, x)
#define FLAGS AV_OPT_FLAG_VIDEO_PARAM|AV_OPT_FLAG_FILTERING_PARAM

static const AVOption yuvscale_options[] = {
    { "w",     "Output video width",          OFFSET(w_expr),    AV_OPT_TYPE_STRING,.flags = FLAGS },
    { "width", "Output video width",          OFFSET(w_expr),    AV_OPT_TYPE_STRING,.flags = FLAGS },
    { "h",     "Output video height",         OFFSET(h_expr),    AV_OPT_TYPE_STRING,.flags = FLAGS },
    { "height","Output video height",         OFFSET(h_expr),    AV_OPT_TYPE_STRING,.flags = FLAGS },
    { NULL }
};

static const AVClass yuvscale_class = {
    .class_name = "yuvscale",
    .item_name = av_default_item_name,
    .option = yuvscale_options,
    .version = LIBAVUTIL_VERSION_INT,
    .category = AV_CLASS_CATEGORY_FILTER,
};

static const AVFilterPad avfilter_vf_yuvscale_inputs[] = {
    {
        .name = "yuvscale_inputpad",
        .type = AVMEDIA_TYPE_VIDEO,
        .filter_frame = filter_frame,
    },
    { NULL }
};

static const AVFilterPad avfilter_vf_yuvscale_outputs[] = {
    {
        .name = "yuvscale_outputpad",
        .type = AVMEDIA_TYPE_VIDEO,
        .config_props = config_props,
    },
    { NULL }
};

AVFilter ff_vf_yuvscale = {
    .name = "yuvscale",
    .description = NULL_IF_CONFIG_SMALL("yuv scale"),
    .priv_size = sizeof(ScaleContext),
    .priv_class = &yuvscale_class,
    .init_dict = init_dict,
    .uninit = uninit,
    .query_formats = query_formats,
    .inputs = avfilter_vf_yuvscale_inputs,
    .outputs = avfilter_vf_yuvscale_outputs,
    .process_command = process_command,
};
