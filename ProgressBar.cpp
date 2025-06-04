#include "ProgressBar.h"

void showProgressBar(int64_t progress, int64_t total) {
    const int barWidth = 50;

    float ratio = static_cast<float>(progress) / total;
    int64_t barLength = static_cast<int64_t>(ratio * barWidth);

    std::cout << "[";
    for (int i = 0; i < barWidth; ++i) {
        if (i < barLength) {
            std::cout << "=";
        }
        else {
            std::cout << " ";
        }
    }
    std::cout << "] " << static_cast<int64_t>(ratio * 100.0) << "%\r";
    std::cout.flush();
}