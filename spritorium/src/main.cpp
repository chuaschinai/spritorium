#include <spdlog/spdlog.h>

#include "app/app_state.hpp"
#include "app/application.hpp"

int main(int argc, char** argv)
{
    try {
        Application app;

        while (g.IsRunning) {
            app.Run();
        }
        
    } catch(std::exception& e) {
        SPDLOG_CRITICAL(e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
