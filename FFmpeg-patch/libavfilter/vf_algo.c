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

#include <dlfcn.h>

typedef int32_t (*PF_VFilter_Init)(void);
typedef int32_t (*PF_VFilter_Destroy)(void);
typedef int32_t (*PF_VFilter_Routine)(uint32_t channelId, uint8_t *y, uint8_t *u, uint8_t *v, uint32_t width,
                                      uint32_t height);

static void *handle = NULL;
static PF_VFilter_Init pf_VFilter_Init = NULL;
static PF_VFilter_Destroy pf_VFilter_Destroy = NULL;
static PF_VFilter_Routine pf_VFilter_Routine = NULL;

typedef struct AlgoContext {
    const AVClass *class;
    int cid;	// channel id
    //add some private data if you want
} AlgoContext;

static int filter_frame(AVFilterLink *link, AVFrame *in) {
    AVFilterContext *avctx = link->dst;
    AVFilterLink *outlink = avctx->outputs[0];
    AlgoContext *privCtx = avctx->priv;

    // filter routine
    // YUV420P, y= avframe->data[0], u=avframe->data[1], v=avframe->data[2]
    // 传递ffmpeg默认的yuv420p格式到下层，不在ffmpeg中转成bgr24，是因为ffmpg的sws格式转换性能比较差，在下层使用libyuv转换
    pf_VFilter_Routine(privCtx->cid, in->data[0], in->data[1], in->data[2], in->width, in->height);

    return ff_filter_frame(outlink, in);
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
    int ret = 0;
    AlgoContext *privCtx = ctx->priv;
    const char *DLL_NAME = "libvideo_filter.so";

    uint64_t channel_id = 0;
    av_opt_get_int(privCtx, "cid", 0, (int64_t*)&channel_id);

    av_log(NULL, AV_LOG_DEBUG, "init \n");

    av_log(NULL, AV_LOG_INFO, "Channel id %ld\n", channel_id);
    privCtx->cid = (int)channel_id;

    handle = dlopen(DLL_NAME, RTLD_LAZY);
    if (NULL == handle) {
        av_log(NULL, AV_LOG_ERROR, "load library %s failed, error %s\n", DLL_NAME, dlerror());
        return AVERROR(EINVAL);
    }

    pf_VFilter_Init = (PF_VFilter_Init)dlsym(handle, "VFilter_Init");
    pf_VFilter_Destroy = (PF_VFilter_Destroy)dlsym(handle, "VFilter_Destroy");
    pf_VFilter_Routine = (PF_VFilter_Routine)dlsym(handle, "VFilter_Routine");

    if (NULL == pf_VFilter_Init || NULL == pf_VFilter_Destroy || NULL == pf_VFilter_Routine) {
        av_log(NULL, AV_LOG_ERROR, "no symbol found in library, %p, %p, %p\n", pf_VFilter_Init, pf_VFilter_Destroy,
               pf_VFilter_Routine);
        return AVERROR(EINVAL);
    }

    ret = pf_VFilter_Init();
    if (0 != ret) {
        av_log(NULL, AV_LOG_ERROR, "failed to init vf\n");
        return AVERROR(ret);
    }

    return 0;
}

static av_cold void uninit(AVFilterContext *ctx) {
    av_log(NULL, AV_LOG_DEBUG, "uninit \n");
    //TransformContext *privCtx = ctx->priv;
    if (NULL != pf_VFilter_Destroy) {
        pf_VFilter_Destroy();
    }
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


#define OFFSET(x) offsetof(AlgoContext, x)
#define FLAGS AV_OPT_FLAG_VIDEO_PARAM|AV_OPT_FLAG_FILTERING_PARAM

static const AVOption algo_options[] = {
    { "cid", "the video channel id", OFFSET(cid), AV_OPT_TYPE_INT, { .i64 = -1 }, INT_MIN, INT_MAX, FLAGS },
    { NULL }

};

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
    .priv_size      = sizeof(AlgoContext),
    .priv_class     = &algo_class,
    .init           = init,
    .uninit         = uninit,
    .query_formats  = query_formats,
    .inputs         = avfilter_vf_algo_inputs,
    .outputs        = avfilter_vf_algo_outputs,
};
