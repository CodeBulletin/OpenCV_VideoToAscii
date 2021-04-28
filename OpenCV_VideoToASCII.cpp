#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <windows.h>

using namespace std;
using namespace cv;
const std::string ASCII_LIST = " .',:;c1xokXdO0K";

std::string gray2ascii(Mat img) {
    std::string ls;
    int n = (255/(ASCII_LIST.size()-1));

    for (int i = 0; i < img.size[0]; i++) {
        for (int j = 0; j < img.size[1]; j += 1) {
            ls += ASCII_LIST[int(img.at<uchar>(i, j) / n)];
        }
        if (i != img.size[0] - 1) {
            ls += "\n";
        }
    }
    return ls;
}

void clearscreen()
{
    HANDLE hOut;
    COORD Position;

    hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    Position.X = 0;
    Position.Y = 0;
    SetConsoleCursorPosition(hOut, Position);
}

int main(int argc, char* argv[]) {

    if (argc < 2) {
        exit(0);
    }

    VideoCapture cap(argv[1]);
    if (!cap.isOpened()) {
        cout << "Error opening video stream or file" << endl;
        return -1;
    }
    
    system("cls");

    Sleep(5000);

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int columns, rows;

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    while (true) {
        Mat frame;
        clearscreen();
        cap >> frame;
        if (frame.empty())
            break;
        cvtColor(frame, frame, cv::COLOR_RGB2GRAY);
        resize(frame, frame, Size(columns, rows), 0, 0, INTER_CUBIC);
        std::cout << gray2ascii(frame);
        if (waitKey(30) == 27) {
            break;
        }
    }
    cap.release();
    destroyAllWindows();

    return 0;
}