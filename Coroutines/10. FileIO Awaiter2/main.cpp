#include <coroutine>
#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>
#include <filesystem>
struct Task {
    struct promise_type {
        std::string m_ReadBytes{};
        Task get_return_object() {
            return {std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_never initial_suspend() {
            return {};
        }
        std::suspend_always final_suspend() noexcept {
            return {};
        }
        void unhandled_exception() {
        }
        void return_value(std::string fileData) {
	        m_ReadBytes = std::move(fileData) ;
        }

        /*std::suspend_always await_transform(int bytes) {
                m_ReadBytes.resize(bytes);
                using namespace std::chrono_literals;
                std::this_thread::sleep_for(1s);
                m_Input.read(m_ReadBytes.data(), bytes);
                return {};
        }*/
    };
    std::coroutine_handle<promise_type> m_Handle;

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
    std::filesystem::path filePath{} ;
    std::string fileData{} ;
    std::thread worker{};
    bool await_ready() {
        return false;
    }
    void await_suspend(std::coroutine_handle<Task::promise_type> handle) {
        std::cout << "[FileAwaiter] Reading file..." ;
		worker = std::thread{ [this, handle]() {
			std::ifstream input_stream{filePath};
			if (!input_stream) {
				throw std::logic_error{"Could not open the file for reading"};
			}
			auto fileSize = file_size(filePath);
			fileData.resize(fileSize);
			input_stream.read(fileData.data(), fileSize);
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(5s);
			std::cout << "done\n";
            handle.resume() ;
			}
		};
        worker.detach() ;
    }
    std::string await_resume() {
        return std::move(fileData);
    }
};

Task GetFileData(std::filesystem::path filePath) {
    std::cout << "\n\t<Coroutine - Acquiring data>\n";
    auto fileData = co_await FileAwaiter{filePath};

    std::cout << "\t<Coroutine - Finished>\n";
    co_return fileData;
}

int main() {
    try {
        std::cout << "main - Starting coroutine\n";
        auto task = GetFileData("main.cpp");
        while (!task.m_Handle.done()) {
            std::cout << "[ main running ]\n";
            std::this_thread::sleep_for(std::chrono::milliseconds{500}) ;
        }
        std::cout << "[main] File data acquired. Press enter to display file content" ;
        std::cin.get() ;
    	std::cout << task.get_read_bytes() << '\n';
        std::cout << "\nmain - Coroutine is done!\n";
    } catch (std::exception &ex) {
        std::cout << "Exception - > " << ex.what() << '\n';
    }
}