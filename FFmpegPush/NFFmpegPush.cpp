#include "NFFmpegPush.h"
#include <stdio.h>
#include <stdexcept>

#ifdef __cplusplus
extern "C" {
#include "libavcodec\avcodec.h"
#include "libavformat\avformat.h"
#include "libswscale\swscale.h"
#include "libavutil\timestamp.h"
#include "libavutil\imgutils.h"
#include "libswscale\swscale.h"
#include "libswresample\swresample.h"
#include "libavutil\samplefmt.h"
#include "libavutil\opt.h"
#include "libavutil\frame.h"
#include "libavutil\channel_layout.h"
#include "libavutil\common.h"
#include "libavutil\mathematics.h"
#include "libavutil\error.h"
}
#endif

NFFmpegPush::NFFmpegPush(const char* outputUrl, int w, int h, int frameRate, const char* encoder)
	: output_url(outputUrl), 
	width(w), 
	height(h), 
	fps(frameRate), 
	encoder_name(encoder),
	fmt_ctx(nullptr), codec_ctx(nullptr), sws_ctx(nullptr), av_frame(nullptr), pkt(nullptr), av_stream(nullptr),
	pts_counter(0), initialized(false) 
{
	avformat_network_init();
	if (!init()) 
	{
		cleanup();
		throw std::runtime_error("FFmpeg initialization failed!");
	}
}

NFFmpegPush::~NFFmpegPush() {
	if (initialized) 
	{
		// 写入尾部和清理
		if (fmt_ctx && !(fmt_ctx->oformat->flags & AVFMT_NOFILE)) {
			av_write_trailer(fmt_ctx);
		}
		cleanup();
	}
	avformat_network_deinit();
}

bool NFFmpegPush::init() {
	// 1. 创建输出上下文
	if (avformat_alloc_output_context2(&fmt_ctx, nullptr, "flv", output_url) < 0) {
		return false;
	}

	// 2. 查找编码器
	const AVCodec* codec = avcodec_find_encoder_by_name(encoder_name);
	if (!codec) {
		return false;
	}

	// 3. 设置编码器参数
	codec_ctx = avcodec_alloc_context3(codec);
	codec_ctx->width = width;
	codec_ctx->height = height;
	codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
	codec_ctx->time_base = { 1, fps };
	codec_ctx->framerate = { fps, 1 };
	codec_ctx->bit_rate = 2000000;  // 2Mbps
	codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	codec_ctx->flags |= AV_CODEC_FLAG_LOW_DELAY;
	//codec_ctx->gop_size = fps;      // 关键帧间隔1秒

	// NVIDIA硬编码专用参数
	av_opt_set(codec_ctx->priv_data, "rc", "cbr", 0);
	av_opt_set(codec_ctx->priv_data, "cbr", "1", 0);
	av_opt_set(codec_ctx->priv_data, "preset", "ll", 0);
	av_opt_set(codec_ctx->priv_data, "delay", "0", 0); // 编码器延迟帧设为0（最低延迟）
	av_opt_set(codec_ctx->priv_data, "tune", "zerolatency", 0); // 禁用编码器内部缓冲

	// 4. 打开编码器
	if (avcodec_open2(codec_ctx, codec, nullptr) < 0) {
		return false;
	}

	// 5. 创建输出流
	av_stream = avformat_new_stream(fmt_ctx, codec);
	av_stream->time_base = codec_ctx->time_base;
	if (avcodec_parameters_from_context(av_stream->codecpar, codec_ctx) < 0){
		return false;
	}

	// 6. 打开输出连接
	if (!(fmt_ctx->oformat->flags & AVFMT_NOFILE)) {
		if (avio_open(&fmt_ctx->pb, output_url, AVIO_FLAG_WRITE) < 0) {
			return false;
		}
	}

	if (codec_ctx->extradata && codec_ctx->extradata_size > 0) {
		// 检查SPS/PPS是否已生成
		printf("检查SPS/PPS是否已生成: OK, Extradata size: %d\n", codec_ctx->extradata_size);
	}

	// 7. 写入头部
	if (avformat_write_header(fmt_ctx, nullptr) < 0) {
		return false;
	}

	// 8. 初始化图像转换和帧缓存
	sws_ctx = sws_getContext(width, height, AV_PIX_FMT_BGR24,
		width, height, AV_PIX_FMT_YUV420P,
		SWS_BICUBIC, nullptr, nullptr, nullptr);

	av_frame = av_frame_alloc();
	av_frame->format = AV_PIX_FMT_YUV420P;
	av_frame->width = width;
	av_frame->height = height;
	av_frame_get_buffer(av_frame, 32);

	pkt = av_packet_alloc();
	initialized = true;
	return true;
}

int NFFmpegPush::pushStream(cv::Mat &frame) 
{
	if (!initialized || frame.empty() || frame.cols != width || frame.rows != height) {
		return -1;
	}

	// 转换BGR -> YUV420P
	uint8_t* src_data[1] = { frame.data };
	int src_linesize[1] = { static_cast<int>(frame.step) };
	sws_scale(sws_ctx, src_data, src_linesize, 0, height,
		av_frame->data, av_frame->linesize);

	// 设置时间戳
	av_frame->pts = pts_counter++;

	// 编码帧
	if (avcodec_send_frame(codec_ctx, av_frame) < 0) {
		return -2;
	}

	// 获取并推送编码包
	while (true) 
	{
		int ret = avcodec_receive_packet(codec_ctx, pkt);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) 
		{
			// 这里一定会退出while循环
			av_packet_unref(pkt);
			break;
		} 
		else if (ret < 0) 
		{
			std::cerr << "Error during encoding" << std::endl;
			av_packet_unref(pkt);
			return -3;
		}

		pkt->stream_index = av_stream->index; // 关联到对应的流
		av_packet_rescale_ts(pkt, codec_ctx->time_base, av_stream->time_base);
		if ((ret = av_interleaved_write_frame(fmt_ctx, pkt)) < 0) 
		{
			char errbuf[1024];
			av_strerror(ret, errbuf, sizeof(errbuf));
			fprintf(stderr, "[SRS推流失败] 错误码: %d, 详情: %s\n", ret, errbuf);
			av_packet_unref(pkt);
			return -4;
		}

		av_packet_unref(pkt);
	}

	return 0;
}

void NFFmpegPush::cleanup() 
{
	if (sws_ctx) sws_freeContext(sws_ctx);
	if (av_frame) av_frame_free(&av_frame);
	if (pkt) av_packet_free(&pkt);
	if (codec_ctx) avcodec_free_context(&codec_ctx);
	if (fmt_ctx && !(fmt_ctx->oformat->flags & AVFMT_NOFILE)) {
		avio_closep(&fmt_ctx->pb);
	}
	if (fmt_ctx) avformat_free_context(fmt_ctx);
}

//void _test_ffmpeg_push()
//{
//	cv::VideoCapture cap("rtmp://192.168.18.251:1935/live/livestream1");
//	if (!cap.isOpened()) {
//		std::cerr << "无法打开视频流！" << std::endl;
//		return;
//	}
//	cv::Mat matFrame;
//	cap.read(matFrame);
//
//	NFFmpegPush pusher("rtmp://192.168.18.251:1935/live/zxl", matFrame.cols, matFrame.rows, 25, "h264_nvenc");
//	while (true)
//	{
//		if (!cap.read(matFrame)) break;
//
//		if (pusher.pushStream(matFrame) != 0) {
//			std::cerr << "Push frame failed!" << std::endl;
//			break;
//		}
//		// 模拟25fps->应该是40ms，但因为缓冲该时间下无法即时填满，导致视频播放不流畅，改为25ms
//		//cv::waitKey(25);
//
//		// 如果禁用了编码器缓冲
//		cv::waitKey(5);
//	}
//}
