#ifndef VIDEO_H
#define VIDEO_H

#include <string>
#include <vector>
#include <memory>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

class Video {
public:
    Video(const std::string& filename);
    Video(const Video& other);
    ~Video();
    std::vector<std::unique_ptr<AVFrame>> extract_iframes();

private:
    AVFormatContext* format_context = nullptr;
    const AVCodec* codec = nullptr;
    AVCodecContext* codec_context = nullptr;
    AVFrame* frame = nullptr;
    int video_stream_index;

    int process_packet(AVPacket* packet, std::vector<std::unique_ptr<AVFrame>>& i_frames);
    int process_iframe(AVFrame* frame, std::vector<std::unique_ptr<AVFrame>>& i_frames);
};

#endif // VIDEO_H
