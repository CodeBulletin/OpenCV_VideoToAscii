#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <SFML/Audio.hpp>
#include <windows.h>

#include <iostream>
#include <thread>
#include <future>
#include <chrono>
#include <cstdlib>

using namespace std;
using namespace cv;
const std::string ASCII_LIST = " .',:;c1xokXdO0K";

HANDLE hOut;
COORD Position;

long frameCount = 0;
int N;

const int processor_count = std::thread::hardware_concurrency();

std::string gray2ascii(const Mat& img) {
    std::string ls;
    int n = (255/(ASCII_LIST.size()-1));

    for (int i = 0; i < img.rows; i++) {
        for (int j = 0; j < img.cols; j += 1) {
            ls += ASCII_LIST[int(img.at<uchar>(i, j) / n)];
        }
        if (i != img.size[0] - 1) {
            ls += "\n";
        }
    }
    return ls;
}

std::string gray2asciiMT(const Mat& img) {
    string str;
    vector<future<string>> threads;
    for (int i = 0; i < img.rows; i+=N) {
        Mat tile = img(Range(i, min(i + N, img.rows)), Range(0, img.cols));
        threads.push_back(std::async(std::launch::async, gray2ascii, tile));
    }
    for (auto& th : threads) {
        str += th.get();
    }
    return str;
}

void clearscreen()
{
    SetConsoleCursorPosition(hOut, Position);
}

int main(int argc, char* argv[]) {

#if _DEBUG
    //pass
#else
    if (argc < 2) {
        exit(0);
    }
#endif

#if _DEBUG
    VideoCapture cap("RickRoll.mp4");
#else
    VideoCapture cap(argv[1]);
#endif
    if (!cap.isOpened()) {
        cout << "Error opening video stream or file" << endl;
        return -1;
    }

    double fps = cap.get(CAP_PROP_FPS);
    double timeM = 1.0 / fps * 1000000.0;
    system("cls");
    system("pause");
    system("cls");
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    Position.X = 0;
    Position.Y = 0;
    Sleep(5000);

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int columns, rows;

    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = false;
    SetConsoleCursorInfo(hOut, &info);

    GetConsoleScreenBufferInfo(hOut, &csbi);
    columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    sf::Music sound;
    if (argc >= 3) {
#if _DEBUG
        //pass
#else
        sound.openFromFile(argv[2]);
        sound.play();
#endif
    }

    N = rows / (processor_count);
    
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    while (true) {
        std::chrono::steady_clock::time_point begin1 = std::chrono::steady_clock::now();
        Mat frame;
        frameCount++;
        clearscreen();
        cap >> frame;
        if (frame.empty())
            break;
        cvtColor(frame, frame, cv::COLOR_RGB2GRAY);
        resize(frame, frame, Size(columns, rows), 0, 0, INTER_CUBIC);
        printf(gray2asciiMT(frame).c_str());
        while (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - begin1).count() < timeM);
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    cap.release();
    system("cls");
    std::cout << "Elapsed Time = " << (double)std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000 << "s" << std::endl;
    std::cout << "Video Time = " << (double)frameCount / fps << "s" << std::endl;
    system("pause");
    return 0;
}