#include "VideoCapture.hpp"

extern "C" {
#include <libavutil/imgutils.h>
}

#include <thread>

VideoCapture::VideoCapture(const std::string &mediaFilePath) {
    if (avformat_open_input(&formatContext,mediaFilePath.c_str(), nullptr, nullptr) != 0) {
        return;
    }

    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        return;
    }
    videoStreamIndex = std::numeric_limits<uint64_t>::max();
    const AVCodec *codec = nullptr;
    const AVCodecParameters *codecParameters = nullptr;
    for (uint64_t i = 0; i < formatContext->nb_streams; i++) {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            codecParameters = formatContext->streams[i]->codecpar;
            codec = avcodec_find_decoder(codecParameters->codec_id);
            break;
        }
    }
    if (videoStreamIndex == std::numeric_limits<uint64_t>::max()) {
        return;
    }

    codecContext = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codecContext, codecParameters);
    if (avcodec_open2(codecContext, codec, nullptr) < 0) {
        return;
    }

    transcoderContext = sws_getContext(
        codecContext->width, codecContext->height, codecContext->pix_fmt,
        codecContext->width, codecContext->height, AV_PIX_FMT_BGR24,
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );

    packet = av_packet_alloc();
    frame = av_frame_alloc();
    bgrFrame = av_frame_alloc();

    const int bufferSize = av_image_get_buffer_size(AV_PIX_FMT_BGR24, codecContext->width, codecContext->height, 1);
    buffer = new uint8_t[bufferSize];
    av_image_fill_arrays(
        bgrFrame->data,
        bgrFrame->linesize,
        buffer,
        AV_PIX_FMT_BGR24,
        codecContext->width,
        codecContext->height,
        1
    );

    isFrameReady = false;
    std::thread(
        [&] {
            while (true) {
                while (av_read_frame(formatContext, packet) >= 0) {
                    if (packet->stream_index == videoStreamIndex) {
                        if (avcodec_send_packet(codecContext, packet) >= 0) {
                            while (avcodec_receive_frame(codecContext, frame) >= 0) {
                                position = frame->pts * av_q2d(formatContext->streams[videoStreamIndex]->time_base) * 1000.0;
                                isFrameReady = true;
                                while (isFrameReady) {
                                }
                            }
                        }
                    }
                    av_packet_unref(packet);
                }
                frameReadResult = false;
            }
        }
    ).detach();
}

VideoCapture::~VideoCapture() {
    delete[] buffer;
    av_frame_free(&frame);
    av_frame_free(&bgrFrame);
    av_packet_free(&packet);
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    sws_freeContext(transcoderContext);
}


bool VideoCapture::read(cv::Mat &outputFrame, double &outputPosition, int32_t width, int32_t height) {
    if (currentWidth != width || currentHeight != height) {
        delete[] buffer;
        av_frame_free(&bgrFrame);
        sws_freeContext(transcoderContext);

        const int bufferSize = av_image_get_buffer_size(AV_PIX_FMT_BGR24, width, height, 1);
        buffer = new uint8_t[bufferSize];
        bgrFrame = av_frame_alloc();
        av_image_fill_arrays(
            bgrFrame->data,
            bgrFrame->linesize,
            buffer,
            AV_PIX_FMT_BGR24,
            width,
            height,
            1
        );

        transcoderContext = sws_getContext(
        codecContext->width, codecContext->height, codecContext->pix_fmt,
        width, height, AV_PIX_FMT_BGR24,
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );

        currentWidth = width;
        currentHeight = height;
    }

    while (!isFrameReady) {
    }

    sws_scale(
        transcoderContext,
        frame->data,
        frame->linesize,
        0,
        codecContext->height,
        bgrFrame->data,
        bgrFrame->linesize
    );
    opencvFrame = cv::Mat(
        height,
        width,
        CV_8UC3,
        bgrFrame->data[0],
        bgrFrame->linesize[0]
    );

    outputFrame = std::move(opencvFrame);
    outputPosition = position;
    isFrameReady = false;
    return frameReadResult;
}

double VideoCapture::getFrameRate() const {
    return av_q2d(formatContext->streams[videoStreamIndex]->avg_frame_rate);
}

int32_t VideoCapture::getOriginalWidth() const {
    return codecContext->width;
}

int32_t VideoCapture::getOriginalHeight() const {
    return codecContext->height;
}
