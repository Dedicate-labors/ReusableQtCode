#include "NRTMPVideoCapture.h"


NRTMPVideoCapture::NRTMPVideoCapture() :
	format_ctx_(nullptr),
	codec_ctx_(nullptr),
	codec_params_(nullptr),
	codec_(nullptr),
	video_stream_index_(-1),
	packet_(nullptr),
	frame_(nullptr),
	sw_frame_(nullptr),
	hw_device_ctx_(nullptr),
	sws_ctx_(nullptr),
	initialized_(false),
	hw_pix_fmt_(AV_PIX_FMT_NONE)
{
}

NRTMPVideoCapture::~NRTMPVideoCapture()
{
	release();
}

bool NRTMPVideoCapture::open(const std::string& url)
{
	// 初始化FFmpeg
	avformat_network_init();

	// 打开输入流
	if (avformat_open_input(&format_ctx_, url.c_str(), nullptr, nullptr) != 0) {
		std::cerr << "无法打开输入流: " << url << std::endl;
		return false;
	}

	// 获取流信息
	if (avformat_find_stream_info(format_ctx_, nullptr) < 0) {
		std::cerr << "无法获取流信息" << std::endl;
		return false;
	}

	// 查找视频流
	video_stream_index_ = -1;
	for (unsigned int i = 0; i < format_ctx_->nb_streams; i++) {
		if (format_ctx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			video_stream_index_ = i;
			break;
		}
	}

	if (video_stream_index_ == -1) {
		std::cerr << "未找到视频流" << std::endl;
		return false;
	}

	// 获取编解码器参数
	codec_params_ = format_ctx_->streams[video_stream_index_]->codecpar;

	// 查找硬件解码器
	codec_ = avcodec_find_decoder_by_name("h264_cuvid");
	if (!codec_) {
		std::cerr << "未找到h264_cuvid解码器，尝试使用h264_nvenc" << std::endl;
		codec_ = avcodec_find_decoder_by_name("h264_nvenc");
	}

	if (!codec_) {
		std::cerr << "未找到NVIDIA硬件解码器，尝试使用软件解码器" << std::endl;
		codec_ = avcodec_find_decoder(codec_params_->codec_id);
	}

	if (!codec_) {
		std::cerr << "未找到合适的解码器" << std::endl;
		return false;
	}

	// 创建解码器上下文
	codec_ctx_ = avcodec_alloc_context3(codec_);
	if (!codec_ctx_) {
		std::cerr << "无法分配编解码器上下文" << std::endl;
		return false;
	}

	// 复制编解码器参数到上下文
	if (avcodec_parameters_to_context(codec_ctx_, codec_params_) < 0) {
		std::cerr << "无法复制编解码器参数到上下文" << std::endl;
		return false;
	}

	// 初始化硬件设备
	if (init_hardware_device() < 0) {
		std::cerr << "无法初始化硬件设备" << std::endl;
		return false;
	}

	// 打开解码器
	if (avcodec_open2(codec_ctx_, codec_, nullptr) < 0) {
		std::cerr << "无法打开解码器" << std::endl;
		return false;
	}

	// 分配帧和包
	frame_ = av_frame_alloc();
	sw_frame_ = av_frame_alloc();
	packet_ = av_packet_alloc();

	if (!frame_ || !sw_frame_ || !packet_) {
		std::cerr << "无法分配帧或包" << std::endl;
		return false;
	}

	initialized_ = true;
	return true;
}

bool NRTMPVideoCapture::read(cv::Mat& output)
{
	if (!initialized_) {
		std::cerr << "RTMP捕获器未初始化" << std::endl;
		return false;
	}

	while (true) {
		// 读取包
		int ret = av_read_frame(format_ctx_, packet_);
		if (ret < 0) {
			// 文件结束或错误
			return false;
		}

		// 只处理视频流
		if (packet_->stream_index == video_stream_index_) {
			// 发送包到解码器
			ret = avcodec_send_packet(codec_ctx_, packet_);
			if (ret < 0) {
				std::cerr << "发送包到解码器失败" << std::endl;
				av_packet_unref(packet_);
				continue;
			}

			// 接收解码后的帧
			ret = avcodec_receive_frame(codec_ctx_, frame_);
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
				av_packet_unref(packet_);
				continue;
			}
			else if (ret < 0) {
				std::cerr << "解码错误" << std::endl;
				av_packet_unref(packet_);
				continue;
			}

			// 处理硬件帧
			AVFrame* tmp_frame = frame_;
			if (frame_->format == hw_pix_fmt_) {
				// 从硬件拷贝到软件帧
				if (av_hwframe_transfer_data(sw_frame_, frame_, 0) < 0) {
					std::cerr << "硬件帧传输失败" << std::endl;
					av_frame_unref(frame_);
					av_packet_unref(packet_);
					continue;
				}
				tmp_frame = sw_frame_;
			}

			// 转换为OpenCV Mat
			if (convert_to_mat(tmp_frame, output)) {
				av_frame_unref(frame_);
				av_packet_unref(packet_);
				return true;
			}

			av_frame_unref(frame_);
		}

		av_packet_unref(packet_);
	}

	return false;
}

void NRTMPVideoCapture::release()
{
	if (sws_ctx_) {
		sws_freeContext(sws_ctx_);
		sws_ctx_ = nullptr;
	}

	if (packet_) {
		av_packet_free(&packet_);
	}

	if (frame_) {
		av_frame_free(&frame_);
	}

	if (sw_frame_) {
		av_frame_free(&sw_frame_);
	}

	if (codec_ctx_) {
		avcodec_free_context(&codec_ctx_);
	}

	if (format_ctx_) {
		avformat_close_input(&format_ctx_);
	}

	if (hw_device_ctx_) {
		av_buffer_unref(&hw_device_ctx_);
	}

	avformat_network_deinit();
	initialized_ = false;
}

bool NRTMPVideoCapture::isOpened() const
{
	return initialized_;
}

int NRTMPVideoCapture::init_hardware_device()
{
	enum AVHWDeviceType type;
	type = av_hwdevice_find_type_by_name("cuda");
	if (type == AV_HWDEVICE_TYPE_NONE) {
		std::cerr << "未找到CUDA设备，尝试使用其他硬件设备" << std::endl;
		type = av_hwdevice_find_type_by_name("dxva2");
	}

	if (type == AV_HWDEVICE_TYPE_NONE) {
		std::cerr << "未找到任何硬件设备，使用软件解码" << std::endl;
		return 0;
	}

	// 获取硬件像素格式
	for (int i = 0;; i++) {
		const AVCodecHWConfig* config = avcodec_get_hw_config(codec_, i);
		if (!config) {
			std::cerr << "解码器不支持硬件加速" << std::endl;
			break;
		}
		if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
			config->device_type == type) {
			hw_pix_fmt_ = config->pix_fmt;
			break;
		}
	}

	if (hw_pix_fmt_ == AV_PIX_FMT_NONE) {
		std::cerr << "未找到合适的硬件像素格式" << std::endl;
		return -1;
	}

	// 创建硬件设备上下文
	int ret = av_hwdevice_ctx_create(&hw_device_ctx_, type, nullptr, nullptr, 0);
	if (ret < 0) {
		std::cerr << "无法创建硬件设备上下文" << std::endl;
		return ret;
	}

	codec_ctx_->hw_device_ctx = av_buffer_ref(hw_device_ctx_);
	return 0;
}

bool NRTMPVideoCapture::convert_to_mat(AVFrame* frame, cv::Mat& output)
{
	if (frame->format == AV_PIX_FMT_NONE) {
		return false;
	}

	// 初始化转换上下文
	if (!sws_ctx_) {
		sws_ctx_ = sws_getContext(
			frame->width, frame->height, (AVPixelFormat)frame->format,
			frame->width, frame->height, AV_PIX_FMT_BGR24,
			SWS_BILINEAR, nullptr, nullptr, nullptr);

		if (!sws_ctx_) {
			std::cerr << "无法创建转换上下文" << std::endl;
			return false;
		}
	}

	// 创建目标AVFrame
	AVFrame* bgr_frame = av_frame_alloc();
	bgr_frame->format = AV_PIX_FMT_BGR24;
	bgr_frame->width = frame->width;
	bgr_frame->height = frame->height;

	if (av_frame_get_buffer(bgr_frame, 0) < 0) {
		std::cerr << "无法分配帧缓冲区" << std::endl;
		av_frame_free(&bgr_frame);
		return false;
	}

	// 转换像素格式
	sws_scale(sws_ctx_, frame->data, frame->linesize, 0, frame->height,
		bgr_frame->data, bgr_frame->linesize);

	// 转换为OpenCV Mat
	output = cv::Mat(frame->height, frame->width, CV_8UC3, bgr_frame->data[0], bgr_frame->linesize[0]).clone();

	av_frame_free(&bgr_frame);
	return true;
}
