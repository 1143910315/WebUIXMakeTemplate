#include "thread/ThreadPool.h"
#include <webui.hpp>
#undef min
#undef max
#include <chrono>
#include <date/date.h>
#include <format>
#include <iostream>
#include <sstream>
#include <string>

template<typename ... Args>
std::string concatEven(std::string firstString, std::string secondString, std::string str, Args... args);

template<typename ... Args>
std::string concatOdd(std::string firstString, std::string secondString, std::string str, Args... args) {
    return str + firstString + concatEven(firstString, secondString, args ...);
}

template<>
std::string concatOdd(std::string firstString, std::string secondString, std::string str) {
    return str + firstString;
}

template<typename ... Args>
std::string concatEven(std::string firstString, std::string secondString, std::string str, Args... args) {
    return str + secondString + concatOdd(firstString, secondString, args ...);
}

template<>
std::string concatEven(std::string firstString, std::string secondString, std::string str) {
    return str + secondString;
}

// 生成自定义事件执行脚本
template<typename ... Args>
std::string generateCustomEvent(std::string className, std::string ev, Args... args) {
    std::string str = "for(e of document.getElementsByClassName('" + className + "')){";
    str += "e.dispatchEvent(new CustomEvent('" + ev + "', { detail: { ";

    str += concatOdd(": '", "', ", args ...);

    str += " }, bubbles: false, cancelable: true }));";
    str += "}";
    return str;
}

int64_t count = 0;
thread::ThreadPool pool;
bool running = true;
webui::window myWindow;
std::function<void()> timeSend = []() {
        if (running) {
            // 获取当前时间
            auto now = std::chrono::system_clock::now() + std::chrono::hours(8);
            // 转换为系统时间
            auto currentTime = date::floor<std::chrono::seconds>(now);
            // 设置自定义格式
            std::ostringstream oss;
            oss << date::format("%Y-%m-%d %H:%M:%S", currentTime);
            myWindow.run(generateCustomEvent("receiveCppData", "receive-data", "message", "当前时间是：" + oss.str()));
            pool.addDelayTask(std::chrono::seconds(1), timeSend);
        }
    };
int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    webui::set_default_root_folder("dist");

    myWindow.bind("", [](webui::window::event *e) {
        if (e->event_type == WEBUI_EVENT_CONNECTED) {
            pool.addDelayTask(std::chrono::seconds(1), timeSend);
            std::cout << "Connected." << std::endl;
        } else if (e->event_type == WEBUI_EVENT_DISCONNECTED) {
            running = false;
            std::cout << "Disconnected." << std::endl;
        } else if (e->event_type == WEBUI_EVENT_MOUSE_CLICK) {
            std::cout << "Click." << std::endl;
        } else if (e->event_type == WEBUI_EVENT_NAVIGATION) {
            std::cout << "Starting navigation to: " << e->get_string_view() << std::endl;
        }
    });
    myWindow.bind("clickCount", [](webui::window::event *e) {
        count++;
        std::cout << e->get_string_view(0) << std::endl;
        e->return_string(std::format("{}", count));
    });
    myWindow.show("index.html");
    webui::wait();
    return 0;
}

#ifdef _WIN32
// Release build
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd) {
    (void)hInstance;
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nShowCmd;
    return main();
}

#endif
