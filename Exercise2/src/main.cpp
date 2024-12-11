#include <stdexcept>
#include <chrono>       // Use instead of <ctime> and <sys/time.h>
#include <windows.h>    // For Windows-specific timing functions
#include "TSPSolver.h"

// Error status and message buffer
int status;
char errmsg[255];

int main(int argc, char const* argv[]) {
    try {
        // Time measurement using std::chrono
        auto start = std::chrono::high_resolution_clock::now();

        // Your implementation here

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        std::cout << "Execution time: " << duration.count() << " ms" << std::endl;

        return 0;
    }
    catch (std::exception& e) {
        std::cout << ">>>EXCEPTION: " << e.what() << std::endl;
        return 1;
    }
}