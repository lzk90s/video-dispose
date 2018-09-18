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
#include "libswscale/swscale.h"
#include "libavformat/avformat.h"

#include "vfilter.h"


typedef struct TransformContext {
    const AVClass *class;
    int backUp;
    //add some private data if you want
} TransformContext;

typedef struct ThreadData {
    AVFrame *in, *out;
} ThreadData;


#if 0
static int i420p_to_bgr24(const AVFrame *src, AVFrame *dst) {
    if (src->format != AV_PIX_FMT_YUV420P) {
        av_log(NULL, AV_LOG_ERROR, "i420p_to_bgr24 only for yuv420p, but the format is %d\n", src->format);
        return AVERROR(EINVAL);
    }

    int w = src->width, h = src->height;
    enum AVPixelFormat src_pixfmt = (enum AVPixelFormat)src->format;
    enum AVPixelFormat dst_pixfmt = AV_PIX_FMT_BGR24;   // to bgr24

    // allocate memory for data, release by caller
    int buf_len = av_image_get_buffer_size(dst_pixfmt, w, h, 1);
    int8_t * buffer = (int8_t*)av_malloc(buf_len);
    if (NULL == buffer) {
        av_log(NULL, AV_LOG_ERROR, "No memory");
        return ENOMEM;
    }
    av_image_fill_arrays(dst->data, dst->linesize, buffer, dst_pixfmt, w, h, 1);

    // convert
    struct SwsContext *convert_ctx = NULL;
    convert_ctx = sws_getContext(w, h, src_pixfmt, w, h, dst_pixfmt, SWS_POINT, NULL, NULL, NULL);
    sws_scale(convert_ctx, src->data, src->linesize, 0, h, dst->data, dst->linesize);
    sws_freeContext(convert_ctx);

    // copy extra data
    dst->format = dst_pixfmt;
    dst->width = src->width;
    dst->height = src->height;
    dst->key_frame = src->key_frame;
    dst->pict_type = src->pict_type;
    dst->pts = src->pts;
    dst->pkt_pts = src->pkt_pts;
    dst->pkt_dts = src->pkt_dts;
    dst->pkt_pos = src->pkt_pos;
    dst->pkt_size = src->pkt_size;

    return 0;
}

static int bgr24_to_i420p(const  AVFrame *src, AVFrame *dst) {
    if (src->format != AV_PIX_FMT_BGR24) {
        av_log(NULL, AV_LOG_ERROR, "bgr24_to_i420p only for bgr24, but the format is %d\n", src->format);
        return -1;
    }

    int w = src->width, h = src->height;
    enum AVPixelFormat src_pixfmt = (enum AVPixelFormat)src->format;
    enum AVPixelFormat dst_pixfmt = AV_PIX_FMT_YUV420P;

    // allocate memory, release by caller
    int buf_len = av_image_get_buffer_size(dst_pixfmt, w, h, 1);
    int8_t * buffer = (int8_t*)av_malloc(buf_len);
    if (NULL == buffer) {
        av_log(NULL, AV_LOG_ERROR, "No memory");
        return ENOMEM;
    }
    av_image_fill_arrays(dst->data, dst->linesize, buffer, dst_pixfmt, w, h, 1);

    // convert
    struct SwsContext *convert_ctx = NULL;
    convert_ctx = sws_getContext(w, h, src_pixfmt, w, h, dst_pixfmt, SWS_POINT, NULL, NULL, NULL);
    sws_scale(convert_ctx, src->data, src->linesize, 0, h, dst->data, dst->linesize);
    sws_freeContext(convert_ctx);

    // copy extra data
    dst->format = dst_pixfmt;
    dst->width = src->width;
    dst->height = src->height;
    dst->key_frame = src->key_frame;
    dst->pict_type = src->pict_type;
    dst->pts = src->pts;
    dst->pkt_pts = src->pkt_pts;
    dst->pkt_dts = src->pkt_dts;
    dst->pkt_pos = src->pkt_pos;
    dst->pkt_size = src->pkt_size;

    return 0;
}
#endif

static void image_copy_plane(uint8_t *dst, int dst_linesize,
                             const uint8_t *src, int src_linesize,
                             int bytewidth, int height) {
    if (!dst || !src)
        return;
    av_assert0(abs(src_linesize) >= bytewidth);
    av_assert0(abs(dst_linesize) >= bytewidth);
    for (; height > 0; height--) {
        memcpy(dst, src, bytewidth);
        dst += dst_linesize;
        src += src_linesize;
    }
}

//for YUV data, frame->data[0] save Y, frame->data[1] save U, frame->data[2] save V
static int frame_copy_video(AVFrame *dst, const AVFrame *src) {
    int i, planes;

    if (dst->width  > src->width ||
            dst->height > src->height)
        return AVERROR(EINVAL);

    planes = av_pix_fmt_count_planes(dst->format);
    //make sure data is valid
    for (i = 0; i < planes; i++)
        if (!dst->data[i] || !src->data[i])
            return AVERROR(EINVAL);

    const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(dst->format);
    int planes_nb = 0;
    for (i = 0; i < desc->nb_components; i++)
        planes_nb = FFMAX(planes_nb, desc->comp[i].plane + 1);

    for (i = 0; i < planes_nb; i++) {
        int h = dst->height;
        int bwidth = av_image_get_linesize(dst->format, dst->width, i);
        if (bwidth < 0) {
            av_log(NULL, AV_LOG_ERROR, "av_image_get_linesize failed\n");
            return AVERROR(EINVAL);
        }
        if (i == 1 || i == 2) {
            h = AV_CEIL_RSHIFT(dst->height, desc->log2_chroma_h);
        }
        image_copy_plane(dst->data[i], dst->linesize[i],
                         src->data[i], src->linesize[i],
                         bwidth, h);
    }
    return 0;
}

static int do_conversion(AVFilterContext *ctx, void *arg, int jobnr,
                         int nb_jobs) {
    //TransformContext *privCtx = ctx->priv;
    ThreadData *td = arg;
    AVFrame *dst = td->out;
    AVFrame *src = td->in;

    // video filter, data[0] (bgr24 data)
    VFilter_Routine(src->data[0], src->width, src->height);

    // copy video
    frame_copy_video(dst, src);

    return 0;
}

static int filter_frame(AVFilterLink *link, AVFrame *in) {
    AVFilterContext *avctx = link->dst;
    AVFilterLink *outlink = avctx->outputs[0];
    AVFrame *out;

    //allocate a new buffer, data is null
    out = ff_get_video_buffer(outlink, outlink->w, outlink->h);
    if (!out) {
        av_frame_free(&in);
        return AVERROR(ENOMEM);
    }

    av_frame_copy_props(out, in);
    out->width  = outlink->w;
    out->height = outlink->h;

    ThreadData td;
    td.in = in;
    td.out = out;
    int res;
    if(res = avctx->internal->execute(avctx, do_conversion, &td, NULL, FFMIN(outlink->h, avctx->graph->nb_threads))) {
        return res;
    }

    av_frame_free(&in);

    return ff_filter_frame(outlink, out);
}

static av_cold int config_output(AVFilterLink *outlink) {
    AVFilterContext *ctx = outlink->src;
    //TransformContext *privCtx = ctx->priv;

    //you can modify output width/height here
    outlink->w = ctx->inputs[0]->w;
    outlink->h = ctx->inputs[0]->h;
    av_log(NULL, AV_LOG_DEBUG, "configure output, w h = (%d %d), format %d \n", outlink->w, outlink->h, outlink->format);

    return 0;
}

static av_cold int init(AVFilterContext *ctx) {
    av_log(NULL, AV_LOG_DEBUG, "init \n");
    //TransformContext *privCtx = ctx->priv;
    //init something here if you want
    return 0;
}

static av_cold void uninit(AVFilterContext *ctx) {
    av_log(NULL, AV_LOG_DEBUG, "uninit \n");
    //TransformContext *privCtx = ctx->priv;
    //uninit something here if you want
}

//currently we just support the most common YUV420, can add more if needed
static int query_formats(AVFilterContext *ctx) {
    static const enum AVPixelFormat pix_fmts[] = {
        AV_PIX_FMT_BGR24,
        AV_PIX_FMT_NONE
    };
    AVFilterFormats *fmts_list = ff_make_format_list(pix_fmts);
    if (!fmts_list)
        return AVERROR(ENOMEM);
    return ff_set_common_formats(ctx, fmts_list);
}


#define OFFSET(x) offsetof(TransformContext, x)
#define FLAGS AV_OPT_FLAG_VIDEO_PARAM|AV_OPT_FLAG_FILTERING_PARAM

static const AVOption algo_options[] = {
    { "backUp", "a backup parameters, NOT use so far", OFFSET(backUp), AV_OPT_TYPE_STRING, {.str = "0"}, CHAR_MIN, CHAR_MAX, FLAGS },
    { NULL }

};// TODO: add something if needed

static const AVClass algo_class = {
    .class_name       = "algo",
    .item_name        = av_default_item_name,
    .option           = algo_options,
    .version          = LIBAVUTIL_VERSION_INT,
    .category         = AV_CLASS_CATEGORY_FILTER,
};

static const AVFilterPad avfilter_vf_algo_inputs[] = {
    {
        .name         = "algo_inputpad",
        .type         = AVMEDIA_TYPE_VIDEO,
        .filter_frame = filter_frame,
    },
    { NULL }
};

static const AVFilterPad avfilter_vf_algo_outputs[] = {
    {
        .name = "algo_outputpad",
        .type = AVMEDIA_TYPE_VIDEO,
        .config_props = config_output,
    },
    { NULL }
};

AVFilter ff_vf_algo = {
    .name           = "algo",
    .description    = NULL_IF_CONFIG_SMALL("cut a part of video"),
    .priv_size      = sizeof(TransformContext),
    .priv_class     = &algo_class,
    .init          = init,
    .uninit        = uninit,
    .query_formats = query_formats,
    .inputs         = avfilter_vf_algo_inputs,
    .outputs        = avfilter_vf_algo_outputs,
};
