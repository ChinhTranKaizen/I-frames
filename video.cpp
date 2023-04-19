#include "video.h"
#include <stdexcept>
#include <vector>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

// Constructor that initializes the video object by opening the input file and decoding its video stream.
// It takes a filename string as input and throws a runtime_error if an error occurs during initialization.
Video::Video(const std::string& filename) {

    // Initialize member variables to null or default values.
    format_context = nullptr;
    codec = nullptr;
    codec_context = nullptr;
    frame = nullptr;
    video_stream_index = -1;

    // Open the input file and check for errors.
    if (avformat_open_input(&format_context, filename.c_str(), nullptr, nullptr) < 0) {
        throw std::runtime_error("Error opening video file");
    }

    // Find the stream information and check for errors.
    if (avformat_find_stream_info(format_context, nullptr) < 0) {
        avformat_close_input(&format_context);
        throw std::runtime_error("Error finding stream information");
    }

    // Find the video stream index.
    for (unsigned int i = 0; i < format_context->nb_streams; ++i) {
        if (format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = static_cast<int>(i);
            break;
        }
    }

    // Throw an error if the video stream index could not be found.
    if (video_stream_index == -1) {
        avformat_close_input(&format_context);
        throw std::runtime_error("Error finding video stream");
    }

    // Find the codec and check for errors.
    codec = avcodec_find_decoder(format_context->streams[video_stream_index]->codecpar->codec_id);
    if (!codec) {
        avformat_close_input(&format_context);
        throw std::runtime_error("Error finding codec");
    }

    // Allocate and initialize the codec context and check for errors.
    codec_context = avcodec_alloc_context3(codec);
    if (!codec_context) {
        avformat_close_input(&format_context);
        throw std::runtime_error("Error allocating codec context");
    }

    // Copy codec parameters from the input stream to the codec context and check for errors.
    if (avcodec_parameters_to_context(codec_context, format_context->streams[video_stream_index]->codecpar) < 0) {
        avcodec_free_context(&codec_context);
        avformat_close_input(&format_context);
        throw std::runtime_error("Error copying codec parameters");
    }

    // Open the codec and check for errors.
    if (avcodec_open2(codec_context, codec, nullptr) < 0) {
        avcodec_free_context(&codec_context);
        avformat_close_input(&format_context);
        throw std::runtime_error("Error opening codec");
    }

    // Allocate the frame and check for errors.
    frame = av_frame_alloc();
    if (!frame) {
        avcodec_close(codec_context);
        avcodec_free_context(&codec_context);
        avformat_close_input(&format_context);
        throw std::runtime_error("Error allocating frame");
    }
}

Video::Video(const Video& other) {
    // Initialize member variables to null or default values.
    format_context = nullptr;
    codec = nullptr;
    codec_context = nullptr;
    frame = nullptr;
    video_stream_index = -1;

    // Open the input file and check for errors.
    if (avformat_open_input(&format_context, other.format_context->url, nullptr, nullptr) < 0) {
        throw std::runtime_error("Error opening video file");
    }

    // Find the stream information and check for errors.
    if (avformat_find_stream_info(format_context, nullptr) < 0) {
        avformat_close_input(&format_context);
        throw std::runtime_error("Error finding stream information");
    }

    // Find the video stream index.
    for (unsigned int i = 0; i < format_context->nb_streams; ++i) {
        if (format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = static_cast<int>(i);
            break;
        }
    }

    if (video_stream_index == -1) {
        avformat_close_input(&format_context);
        throw std::runtime_error("Error finding video stream");
    }

    // Find the codec and check for errors.
    codec = avcodec_find_decoder(format_context->streams[video_stream_index]->codecpar->codec_id);
    if (!codec) {
        avformat_close_input(&format_context);
        throw std::runtime_error("Error finding codec");
    }

    // Allocate and initialize the codec context and check for errors.
    codec_context = avcodec_alloc_context3(codec);
    if (!codec_context) {
        avformat_close_input(&format_context);
        throw std::runtime_error("Error allocating codec context");
    }

    if (avcodec_parameters_to_context(codec_context, format_context->streams[video_stream_index]->codecpar) < 0) {
        avcodec_free_context(&codec_context);
        avformat_close_input(&format_context);
        throw std::runtime_error("Error copying codec parameters");
    }

    if (avcodec_open2(codec_context, codec, nullptr) < 0) {
        avcodec_free_context(&codec_context);
        avformat_close_input(&format_context);
        throw std::runtime_error("Error opening codec");
    }

    // Allocate the frame and check for errors.
    frame = av_frame_alloc();
    if (!frame) {
        avcodec_close(codec_context);
        avcodec_free_context(&codec_context);
        avformat_close_input(&format_context);
        throw std::runtime_error("Error allocating frame");
    }
}


Video::~Video() {
    // Free the frame.
    av_frame_free(&frame);

    // Close the codec and free the codec context.
    if (codec_context) {
        avcodec_close(codec_context);
        avcodec_free_context(&codec_context);
    }

    // Close the input file and free the format context.
    if (format_context) {
        avformat_close_input(&format_context);
        format_context = nullptr;
    }
}

// This method extracts all the keyframes (I-frames) from the video stream and
// returns them as a vector of unique pointers to AVFrame structures.
std::vector<std::unique_ptr<AVFrame>> Video::extract_iframes() {
    std::vector<std::unique_ptr<AVFrame>> i_frames; // Initialize an empty vector to hold the I-frames.
    AVPacket packet; // Initialize a packet to read frames from the video stream.
    int ret; // Initialize a variable to hold the return value of functions.

    // Read each packet from the video stream until the end is reached.
    while ((ret = av_read_frame(format_context, &packet)) == 0) {
        // If the packet belongs to the video stream, process it.
        if (packet.stream_index == video_stream_index) {
            // Process the packet to extract I-frames and add them to the vector.
            ret = process_packet(&packet, i_frames);
            if (ret < 0) {
                av_packet_unref(&packet);
                throw std::runtime_error("Error processing packet");
            }
        }
        av_packet_unref(&packet); // Release the packet to free memory.
    }

    // Check if there was an error reading the frames from the input file.
    if (ret != AVERROR_EOF) {
        throw std::runtime_error("Error reading frames from input file");
    }

    return i_frames; // Return the vector of I-frames.
}

/**
 * Process a packet to extract the I-frames and store them in the given vector.
 * @param packet The packet to process.
 * @param i_frames The vector to store the extracted I-frames.
 * @return 0 if successful, otherwise a negative error code.
 */
int Video::process_packet(AVPacket* packet, std::vector<std::unique_ptr<AVFrame>>& i_frames) {
    // Send the packet to the decoder.
    int ret = avcodec_send_packet(codec_context, packet);
    if (ret < 0) {
        return ret;
    }

    // Receive frames from the decoder until there are no more frames or an error occurs.
    while (ret >= 0) {
        // Allocate a new frame.
        AVFrame* frame = av_frame_alloc();
        if (!frame) {
            return AVERROR(ENOMEM);
        }

        // Receive a frame from the decoder.
        ret = avcodec_receive_frame(codec_context, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            // The decoder needs more packets or there are no more frames to receive.
            av_frame_free(&frame);
            return 0;
        }
        else if (ret < 0) {
            // An error occurred while receiving the frame.
            av_frame_free(&frame);
            return ret;
        }

        // Check if the frame is an I-frame.
        if (frame->pict_type == AV_PICTURE_TYPE_I) {
            // Process the I-frame.
            ret = process_iframe(frame, i_frames);
            if (ret < 0) {
                // An error occurred while processing the I-frame.
                av_frame_free(&frame);
                return ret;
            }
        }

        // Free the frame.
        av_frame_free(&frame);
    }

    return 0;
}

/**
 * Clones the given frame and adds it to the vector of I-frames.
 * 
 * @param frame The frame to be cloned and added to the vector.
 * @param i_frames The vector of I-frames.
 * @return 0 if successful, or an AVERROR code on failure.
 */
int Video::process_iframe(AVFrame* frame, std::vector<std::unique_ptr<AVFrame>>& i_frames) {
    // Clone the frame.
    std::unique_ptr<AVFrame> cloned_frame(av_frame_clone(frame));
    if (!cloned_frame) {
        return AVERROR(ENOMEM);
    }

    // Add the cloned frame to the vector of I-frames.
    i_frames.push_back(std::move(cloned_frame));
    return 0;
}