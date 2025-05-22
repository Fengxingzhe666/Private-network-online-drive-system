#include "ProgressBar.h"

void showProgressBar(int progress, int total) {
    const int barWidth = 50;

    float ratio = static_cast<float>(progress) / total;
    int barLength = static_cast<int>(ratio * barWidth);

    std::cout << "[";
    for (int i = 0; i < barWidth; ++i) {
        if (i < barLength) {
            std::cout << "=";
        }
        else {
            std::cout << " ";
        }
    }
    std::cout << "] " << static_cast<int>(ratio * 100.0) << "%\r";
    std::cout.flush();
}
//Ñ­»·Õ¹Ê¾progressbar
void circle_disp_progressbar(const int& progress, const int& total,const bool& flag) {
    while (flag) {
        showProgressBar(progress, total);
        Sleep(1000);
    }
}