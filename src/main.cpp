#include "thread/ThreadPool.h"
#include <webui.hpp>
#undef min
#undef max
#include <chrono>
#include <date/date.h>
#include <format>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>

template<typename T1, typename T2, typename ... Args>
void concatProcess(std::string& result, T1 && firstString, T2 && secondString, Args && ... args);

template<typename T1, typename T2, typename T3, typename T4, typename ... Args>
void concatMore(std::string& result, T1&& firstString, T2&& secondString, T3&& str1, T4&& str2, Args&&... args) {
    result.append(str1).append(firstString).append(str2).append(secondString);
    concatProcess(result, std::forward<T1>(firstString), std::forward<T2>(secondString), std::forward<Args>(args)...);
}

template<typename T1, typename T2, typename T3, typename T4>
void concatMore(std::string& result, T1&& firstString, T2&& secondString, T3&& str1, T4&& str2) {
    result.append(str1).append(firstString).append(str2).append(secondString);
}

template<typename T1, typename T2>
void concat(std::string& result, T1&& firstString, T2&& str1) {
    result.append(str1).append(firstString);
}

template<typename T1, typename T2, typename ... Args>
void concatProcess(std::string& result, T1&& firstString, T2&& secondString, Args&&... args) {
    constexpr size_t count = sizeof...(Args);

    if constexpr (count >= 2) {
        concatMore(result, firstString, secondString, std::forward<Args>(args)...);
    } else if constexpr (count == 1) {
        concat(result, firstString, std::forward<Args>(args)...);
    }
}

template<typename T1, typename T2, typename ... Args>
std::string concatString(T1&& firstString, T2&& secondString, Args&&... args) {
    std::string result = "";
    concatProcess(result, std::forward<T1>(firstString), std::forward<T2>(secondString), std::forward<Args>(args)...);
    return result;
}

// 生成自定义事件执行脚本
template<typename ... Args>
std::string generateCustomEvent(std::string className, std::string ev, Args... args) {
    std::string str = "for(e of document.getElementsByClassName('" + className + "')){";
    str += "e.dispatchEvent(new CustomEvent('" + ev + "', { detail: { ";

    str += concatString(": '", "', ", args ...);

    str += " }, bubbles: false, cancelable: true }));";
    str += "}";
    return str;
}

static std::string generateCustomEventForJson(const std::string& className, const std::string& eventName, const std::string& json) {
    return "for(e of document.getElementsByClassName('" + className + "')){e.dispatchEvent(new CustomEvent('" + eventName + "', { detail: " + json + ", bubbles: false, cancelable: true }));}";
}

int64_t clickCount = 0;
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
static void startServer(const std::string& url, int port = -1) {
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
                auto str = e->get_string_view();
                std::cout << "Starting navigation to: " << str << std::endl;
                //myWindow.navigate(str);
            }
        });
    myWindow.bind("clickCount", [](webui::window::event *e) {
            clickCount++;
            std::cout << e->get_string_view(0) << std::endl;
            e->return_string(std::format("{}", clickCount));
        });
    if (port != -1) {
        myWindow.set_size(port);
    }
    myWindow.show(url);
    webui::wait();
}

int main(int argc, char **argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    if (argc < 2) {
        startServer("index.html");
    } else {
        startServer(argv[1], 9000);
    }
    return 0;
}

#ifdef _WIN32
// Release build
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd) {
    (void)hInstance;
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nShowCmd;
    startServer("index.html");
    return 0;
}

#endif
