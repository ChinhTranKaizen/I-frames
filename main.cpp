#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <memory>
#include "video.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

int main(int argc, char* argv[]) {
    // Check that the user has provided an input video file.
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_video>" << std::endl;
        return 1;
    }

    // Create a new Video object with the provided input video filename.
    Video video(argv[1]);

    try {
        // Extract all the I-frames from the video and store them in a vector of unique pointers to AVFrames.
        std::vector<std::unique_ptr<AVFrame>> i_frames = video.extract_iframes();

        // Print the timestamp of each I-frame found.
        for (const auto& frame : i_frames) {
            std::cout << "I-frame found at timestamp: " << frame->pts << std::endl;
        }
    } catch (const std::runtime_error& e) {
        // Handle any errors that occur during the extraction process.
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    // Return 0 to indicate successful completion of the program.
    return 0;
}

