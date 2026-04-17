#include "VideoCapture.hpp"

VideoCapture::VideoCapture(const std::string &mediaFilePath) {
    // 1. Открываем файл
    format_ctx = nullptr;
    if (avformat_open_input(&format_ctx,
                            "/media/bfdestroyeer/Локальный диск/Media/TV Series/Elfen Lied (2004) {tmdb-42671} (S)/Season 01/Elfen Lied S01E01.mkv",
                            nullptr, nullptr) != 0) {
        return;
    }

    // 2. Ищем информацию о потоках
    if (avformat_find_stream_info(format_ctx, nullptr) < 0) return;

    // 3. Находим видео-поток
    video_stream_idx = -1;
    const AVCodec *codec = nullptr;
    AVCodecParameters *codec_params = nullptr;

    for (unsigned int i = 0; i < format_ctx->nb_streams; i++) {
        if (format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_idx = i;
            codec_params = format_ctx->streams[i]->codecpar;
            codec = avcodec_find_decoder(codec_params->codec_id);
            break;
        }
    }

    if (video_stream_idx == -1) return;

    // 4. Настраиваем контекст декодера
    codec_ctx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codec_ctx, codec_params);
    if (avcodec_open2(codec_ctx, codec, nullptr) < 0) return;

    // 5. Готовим инструменты для конвертации (SwsContext)
    // Видео обычно в YUV420P, а OpenCV нужен BGR
    sws_ctx = sws_getContext(
        codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
        codec_ctx->width, codec_ctx->height, AV_PIX_FMT_BGR24,
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );

    packet = av_packet_alloc();
    frame = av_frame_alloc();
    frame_bgr = av_frame_alloc();

    // Буфер для кадра BGR
    int num_bytes = av_image_get_buffer_size(AV_PIX_FMT_BGR24, codec_ctx->width, codec_ctx->height, 1);
    buffer = (uint8_t *) av_malloc(num_bytes * sizeof(uint8_t));
    av_image_fill_arrays(frame_bgr->data, frame_bgr->linesize, buffer, AV_PIX_FMT_BGR24, codec_ctx->width,
                         codec_ctx->height, 1);
}

VideoCapture::~VideoCapture() {
    av_free(buffer);
    av_frame_free(&frame);
    av_frame_free(&frame_bgr);
    av_packet_free(&packet);
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&format_ctx);
    sws_freeContext(sws_ctx);
}


bool VideoCapture::read(cv::Mat &outputFrame, double &outputPosition) {
    bool ready = false;
    while (av_read_frame(format_ctx, packet) >= 0) {
        if (packet->stream_index == video_stream_idx) {
            // Отправляем пакет в декодер
            if (avcodec_send_packet(codec_ctx, packet) >= 0) {
                while (avcodec_receive_frame(codec_ctx, frame) >= 0) {
                    // Конвертируем из родного формата (YUV) в BGR для OpenCV
                    sws_scale(sws_ctx, frame->data, frame->linesize, 0, codec_ctx->height,
                              frame_bgr->data, frame_bgr->linesize);

                    // Создаем cv::Mat на основе данных из кадра FFmpeg
                    outputFrame = cv::Mat(codec_ctx->height, codec_ctx->width, CV_8UC3, frame_bgr->data[0], frame_bgr->linesize[0]);
                    outputPosition = frame->pts * av_q2d(format_ctx->streams[video_stream_idx]->time_base) * 1000.0;

                    // Показываем результат
                    // cv::imshow("FFmpeg + OpenCV", outputFrame);
                    // if (cv::waitKey(1) == 27) break; // ESC для выхода
                    if (ready == true) {
                        std::cout << "ERROR" << std::endl;
                    }
                    ready = true;
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

double VideoCapture::getFrameRate() const {
    return av_q2d(format_ctx->streams[video_stream_idx]->avg_frame_rate);
}

