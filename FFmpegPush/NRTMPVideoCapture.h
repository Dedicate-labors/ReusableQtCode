#include <string>
#include <iostream>
#include <opencv2/opencv.hpp>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
}

class NRTMPVideoCapture {
private:
	AVFormatContext* format_ctx_;
	AVCodecContext* codec_ctx_;
	AVCodecParameters* codec_params_;
	const AVCodec* codec_;
	int video_stream_index_;
	AVPacket* packet_;
	AVFrame* frame_;
	AVFrame* sw_frame_;
	AVBufferRef* hw_device_ctx_;
	SwsContext* sws_ctx_;
	bool initialized_;

	enum AVPixelFormat hw_pix_fmt_;

public:
	NRTMPVideoCapture();

	~NRTMPVideoCapture();

	bool open(const std::string& url);

	bool read(cv::Mat& output);

	void release();

	bool isOpened() const;

private:
	int init_hardware_device();

	bool convert_to_mat(AVFrame* frame, cv::Mat& output);
};