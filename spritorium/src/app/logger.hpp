#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

inline std::shared_ptr<spdlog::logger> OnlyConsoleStdout;

inline void LoggerInit() {
    OnlyConsoleStdout = spdlog::stdout_color_st("OnlyConsoleStdout");
    OnlyConsoleStdout->set_pattern("[%Ss:%ems] [%^%l%$]: %v");

    auto console_sink = std::make_shared<spdlog::sinks::wincolor_stdout_sink_st>();
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_st>("logs/log.txt");

    auto logger = std::make_shared<spdlog::logger>("multi_sink", spdlog::sinks_init_list{console_sink, file_sink});
    logger->set_pattern("[%Ss:%ems] [%^%l%$]: %v");
    spdlog::set_default_logger(logger);

    logger->flush_on(spdlog::level::warn);
}

namespace spto
{
    template <typename... Args>
    void Info(spdlog::format_string_t<Args...> fmt, Args &&...args) {
        spdlog::info(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void Warn(spdlog::format_string_t<Args...> fmt, Args &&...args) {
        spdlog::warn(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void Error(spdlog::format_string_t<Args...> fmt, Args &&...args) {
        spdlog::error(fmt, std::forward<Args>(args)...);
    }
}

#include <imgui.h>

template<>
struct fmt::formatter<ImVec2> : fmt::formatter<std::string> {
    auto format(ImVec2 v, format_context& ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "[{}, {}]", v.x, v.y);
    }
};