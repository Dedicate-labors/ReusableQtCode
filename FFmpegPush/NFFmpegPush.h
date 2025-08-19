#pragma once
#include <opencv2/opencv.hpp>


class AVFormatContext;
class AVCodecContext;
class SwsContext;
class AVFrame;
class AVStream;
class AVPacket;
class NFFmpegPush {
public:
	NFFmpegPush(const char* outputUrl, int width, int height, int fps = 30, const char* encoder = "h264_nvenc");
	~NFFmpegPush();

	// 核心接口：推入一帧图像（返回0成功，负数失败）
	int pushStream(cv::Mat &frame);

private:
	bool init();                // 初始化FFmpeg资源
	void cleanup();             // 清理资源

private:
	AVFormatContext* fmt_ctx;   // 输出上下文
	AVCodecContext* codec_ctx;  // 编码器上下文
	SwsContext* sws_ctx;        // 图像转换器
	AVFrame* av_frame;          // 编码帧缓存
	AVStream* av_stream;		// 输出流
	AVPacket* pkt;              // 编码数据包

	const char* output_url;     // RTMP地址
	const char* encoder_name;   // 编码器名称
	int width, height, fps;     // 视频参数
	int64_t pts_counter;        // 时间戳计数器
	bool initialized;           // 初始化标志
};

// 用法展示
//void _test_ffmpeg_push();
