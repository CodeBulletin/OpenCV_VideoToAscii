#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <SFML/Audio.hpp>
#include <windows.h>

#include <iostream>
#include <thread>
#include <future>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <cmath>

#define _CRT_SECURE_NO_WARNING_

using namespace cv;
const std::string ASCII_LIST = " .',:;c1xokXdO0K$";

HANDLE hOut;
COORD Position;

long frameCount = 0;
int N;

const int processor_count = 16;

float get_linear_value(float val) {
    if (val <= 0.04045) return val / 12.92;
    else return std::powf((val + 0.055) / 1.055, 2.4);
}

float get_luman(float b, float g, float r) {
    float luma = 255.0 * (0.375 * get_linear_value(r / 255.0) + 0.5 * get_linear_value(g / 255.0) + 0.125 * get_linear_value(b / 255.0));
    return luma;
}

float get_grayscale1(float b, float g, float r) {
    float luma = (0.375 * r + 0.5 * g + 0.125 * b);
    return luma;
}

float get_grayscale2(float b, float g, float r) {
    float luma = (std::min({ r, g, b }) + std::max({ r, g, b })) / 2;
    return luma;
}

float get_grayscale3(float b, float g, float r) {
    float luma = (r + g + b) / 3;
    return luma;
}


std::string ascii(const Mat& img) {
    std::string ls;
    int L = ASCII_LIST.size();
    int n = 255/L;

    for (int i = 0; i < img.rows; i++) {
        for (int j = 0; j < img.cols; j++) {
            int index = i * img.cols * 3 + j * 3;

            float val = get_grayscale2(img.data[index + 0], img.data[index + 1], img.data[index + 2]);

            ls += ASCII_LIST[min(L - 1, int(val / n))];
        }
        if (i != img.size[0] - 1) {
            ls += "\n";
        }
    }
    return ls;
}

std::string asciiMT(const Mat& img) {
    std::string str;
    std::vector<std::future<std::string>> threads;
    for (int i = 0; i < img.rows; i+=N) {
        Mat tile = img(Range(i, min(i + N, img.rows)), Range(0, img.cols));
        threads.push_back(std::async(std::launch::async, ascii, tile));
    }
    for (auto& th : threads) {
        str += th.get();
    }
    return str;
}

void clearscreen()
{
#ifdef WIN32
    SetConsoleCursorPosition(hOut, Position);
#endif // WIN32
}

void GetWindowSize(int& columns, int& rows) {
#ifdef WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hOut, &csbi);
    columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#endif // WIN32
}

void ShowConsoleCursor(bool showFlag)
{
#ifdef WIN32
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hOut, &cursorInfo);
    cursorInfo.bVisible = showFlag;
    SetConsoleCursorInfo(hOut, &cursorInfo);
#endif // WIN32
}

int main(int argc, char* argv[]) {

#if _DEBUG
    VideoCapture cap("BadApple.mp4");
#else
    if (argc < 2) {
        exit(0);
    }
    VideoCapture cap(argv[1]);
#endif

    if (!cap.isOpened()) {
        std::cout << "Error opening video stream or file" << std::endl;
        return -1;
    }

    int columns, rows;
    double fps = cap.get(CAP_PROP_FPS);
    double timeM = 1.0 / fps * 1000000.0;
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    Position.X = 0;
    Position.Y = 0;

    system("cls");
    system("pause");
    system("cls");
    GetWindowSize(columns, rows);
    ShowConsoleCursor(false);

    sf::Music Music;
#if _DEBUG
    auto file = std::filesystem::absolute("BadApple.wav").string();
    bool isOpen = Music.openFromFile(file);
    if(isOpen) Music.play();
#else
    Sleep(1000);
    if (argc >= 3) {
        Music.openFromFile(argv[2]);
        Music.play();
    }
#endif

    N = rows / (processor_count);
    
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    while (true) {
        std::chrono::steady_clock::time_point fstart = std::chrono::steady_clock::now();
        Mat frame;
        frameCount++;
        clearscreen();
        cap >> frame;
        if (frame.empty())
            break;
        //cvtColor(frame, frame, cv::COLOR_RGB2GRAY);
        resize(frame, frame, Size(columns, rows), 0, 0, INTER_CUBIC);
        printf(asciiMT(frame).c_str());
        while (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - fstart).count() < timeM);
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    cap.release();
    ShowConsoleCursor(true);

    system("cls");

    std::cout << "Elapsed Time = " << (double)std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000 << "s" << std::endl;
    std::cout << "Video Time = " << (double)frameCount / fps << "s" << std::endl;

    system("pause");
    return 0;
}