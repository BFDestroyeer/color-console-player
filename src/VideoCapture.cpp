#include "VideoCapture.hpp"

VideoCapture::VideoCapture(const std::string &mediaFilePath) {
    // 1. Открываем файл
    formatContext = nullptr;
    if (avformat_open_input(&formatContext,mediaFilePath.c_str(), nullptr, nullptr) != 0) {
        return;
    }

    // 2. Ищем информацию о потоках
    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        return;
    }

    // 3. Находим видео-поток
    videoStreamIndex = -1;
    const AVCodec *codec = nullptr;
    AVCodecParameters *codec_params = nullptr;

    for (unsigned int i = 0; i < formatContext->nb_streams; i++) {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            codec_params = formatContext->streams[i]->codecpar;
            codec = avcodec_find_decoder(codec_params->codec_id);
            break;
        }
    }

    if (videoStreamIndex == -1) return;

    // 4. Настраиваем контекст декодера
    codecContext = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codecContext, codec_params);
    if (avcodec_open2(codecContext, codec, nullptr) < 0) return;

    // 5. Готовим инструменты для конвертации (SwsContext)
    // Видео обычно в YUV420P, а OpenCV нужен BGR
    transcoderContext = sws_getContext(
        codecContext->width, codecContext->height, codecContext->pix_fmt,
        codecContext->width, codecContext->height, AV_PIX_FMT_BGR24,
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );

    packet = av_packet_alloc();
    frame = av_frame_alloc();
    bgrFrame = av_frame_alloc();

    // Буфер для кадра BGR
    int num_bytes = av_image_get_buffer_size(AV_PIX_FMT_BGR24, codecContext->width, codecContext->height, 1);
    buffer = (uint8_t *) av_malloc(num_bytes * sizeof(uint8_t));
    av_image_fill_arrays(
        bgrFrame->data,
        bgrFrame->linesize,
        buffer, AV_PIX_FMT_BGR24,
        codecContext->width,
        codecContext->height,
        1
    );

    isFrameReady = false;
    std::thread(
        [&] {
            while (true) {
                bool ready = false;
                while (av_read_frame(formatContext, packet) >= 0) {
                    if (packet->stream_index == videoStreamIndex) {
                        // Отправляем пакет в декодер
                        if (avcodec_send_packet(codecContext, packet) >= 0) {
                            while (avcodec_receive_frame(codecContext, frame) >= 0) {
                                // Конвертируем из родного формата (YUV) в BGR для OpenCV
                                sws_scale(transcoderContext, frame->data, frame->linesize, 0, codecContext->height,
                                          bgrFrame->data, bgrFrame->linesize);

                                // Создаем cv::Mat на основе данных из кадра FFmpeg
                                opencvFrame = cv::Mat(codecContext->height, codecContext->width, CV_8UC3,
                                                      bgrFrame->data[0], bgrFrame->linesize[0]);
                                position = frame->pts * av_q2d(
                                                     formatContext->streams[videoStreamIndex]->time_base) * 1000.0;

                                // Показываем результат
                                // cv::imshow("FFmpeg + OpenCV", outputFrame);
                                // if (cv::waitKey(1) == 27) break; // ESC для выхода
                                isFrameReady = true;
                                while (isFrameReady) {
                                }
                            }
                        }
                    }
                    av_packet_unref(packet);
                    if (ready) {
                        return true;
                    }
                }
                return false;
            }
        }
    ).detach();
}

VideoCapture::~VideoCapture() {
    av_free(buffer);
    av_frame_free(&frame);
    av_frame_free(&bgrFrame);
    av_packet_free(&packet);
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    sws_freeContext(transcoderContext);
}


bool VideoCapture::read(cv::Mat &outputFrame, double &outputPosition) {
    while (!isFrameReady) {
    }
    outputFrame = std::move(opencvFrame);
    outputPosition = position;
    isFrameReady = false;
    return frameReadResult;
}

double VideoCapture::getFrameRate() const {
    return av_q2d(formatContext->streams[videoStreamIndex]->avg_frame_rate);
}

