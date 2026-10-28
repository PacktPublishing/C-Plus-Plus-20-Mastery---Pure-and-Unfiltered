#include <coroutine>
#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>

struct Task {
    struct promise_type {
        std::ifstream m_Input{};
        std::string m_ReadBytes{};
        Task get_return_object() {
            return {std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() {
            return {};
        }
        std::suspend_always final_suspend() noexcept {
            return {};
        }
        void unhandled_exception() {
        }
        void return_void() {}

        /*std::suspend_always await_transform(int bytes) {
                m_ReadBytes.resize(bytes);
                using namespace std::chrono_literals;
                std::this_thread::sleep_for(1s);
                m_Input.read(m_ReadBytes.data(), bytes);
                return {};
        }*/
    };
    std::coroutine_handle<promise_type> m_Handle;

    void open_file(const std::string &file) {
        m_Handle.promise().m_Input.open(file);
        if (!m_Handle.promise().m_Input) {
            throw std::logic_error{"Could not open the file for reading"};
        }
    }
    std::string get_read_bytes() const {
        return m_Handle.promise().m_ReadBytes;
    }
    Task(const std::coroutine_handle<promise_type> &handle)
        : m_Handle{handle} {
    }
    ~Task() {
        m_Handle.destroy();
    }
};


struct FileAwaiter {
    int bytes{};
    bool next_available{};
    bool await_ready() {
        return false;
    }
    void await_suspend(std::coroutine_handle<Task::promise_type> handle) {
        handle.promise().m_ReadBytes.resize(bytes);
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(200ms);
        handle.promise().m_Input.read(handle.promise().m_ReadBytes.data(), bytes);
        if (handle.promise().m_Input.eof()) {
            next_available = false;
            return;
        }
        next_available = true;
    }
    bool await_resume() {
        return next_available;
    }
};

Task GetFileData() {
    std::cout << "\n\t<Coroutine - Acquiring data>\n";
    bool available{true};
    while (available) {
        available = co_await FileAwaiter{40};
    }

    std::cout << "\t<Coroutine - Finished>\n";
    co_return;
}

int main() {
    try {
        std::cout << "main - Starting coroutine\n";
        auto task = GetFileData();
        task.open_file("main.cpp");
        while (!task.m_Handle.done()) {
            task.m_Handle.resume();
            std::cout << task.get_read_bytes() << '\n';
            std::cout << "[ main running ]\n";
        }
        std::cout << "\nmain - Coroutine is done!\n";
    } catch (std::exception &ex) {
        std::cout << "Exception - > " << ex.what() << '\n';
    }
}