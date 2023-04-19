# Video Processing with FFmpeg and C++

This project demonstrates the ability to employ the FFmpeg library for video processing using C++14. The main goal of this project is to extract keyframes (I-frames) from a video file and display their timestamps. This project uses FFmpeg version 4.4.4 "Rao".

## Dependencies

- FFmpeg 4.4.4 "Rao" (https://ffmpeg.org/download.html)
- C++14 compatible compiler (e.g., GCC)

## How to Compile

To compile the code, use the following command:

```sh
gcc -fdiagnostics-color=always -g -I C:\ffmpeg\include -L C:\ffmpeg\lib main.cpp video.cpp -o main.exe -lmingw32 -mconsole -lstdc++ -lavcodec -lavformat -lavutil -lswscale -lswresample -lpsapi
```

This command assumes that the FFmpeg library is installed at C:\ffmpeg. Make sure to adjust the include and library paths if your installation is in a different location.

##Usage

After compiling the code, you can run the program with the following command:

```sh
main.exe <input_video>
```

Replace `<input_video>` with the path to the video file you want to process. The program will output the timestamps of all the I-frames found in the video.

##Project Structure:

The project consists of two main files:

- `main.cpp`: Contains the main function that processes the input video and displays the I-frame timestamps.
- `video.cpp`: Contains the implementation of the Video class, which is responsible for handling video files, extracting I-frames, and managing resources.

##Features
-Well-structured and organized code, with separate files for the main function and the Video class.
-Object-oriented programming with a custom Video class for better encapsulation and maintainability.
-Thorough error handling with checks for potential issues and clear error messages.
-Effective use of the FFmpeg libraries for video processing tasks.
-Clear and concise comments that explain the purpose of each function and the logic behind specific code blocks.
