#include <coroutine>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
struct Task {
	struct promise_type {
		std::ifstream m_Input{};
		std::string m_ReadBytes{};
		Task get_return_object() {
			return { std::coroutine_handle<promise_type>::from_promise(*this) };
		}
		std::suspend_always initial_suspend() {
			return {};
		}
		std::suspend_always final_suspend()noexcept {
			return {};
		}
		void unhandled_exception() {}
		void return_void() {}

		std::suspend_always await_transform(int bytes) {
			m_ReadBytes.resize(bytes);
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(1s);
			m_Input.read(m_ReadBytes.data(), bytes);
			return {};
		}
	};
	std::coroutine_handle<promise_type> m_Handle;

	void open_file(const std::string& file) {
		m_Handle.promise().m_Input.open(file);
		if (!m_Handle.promise().m_Input) {
			throw std::logic_error{ "Could not open the file for reading" };
		}
	}
	std::string get_read_bytes()const {
		return m_Handle.promise().m_ReadBytes;
	}
	Task(const std::coroutine_handle<promise_type>& handle)
		: m_Handle{ handle } {
	}
	~Task() {
		m_Handle.destroy();
	}
};

Task GetFileData() {
	std::cout << "\n\t<Coroutine - Acquiring data>\n";
	co_await 8;
	co_await 20;
	co_await 15;
	co_await 4;
	co_await 24;
	std::cout << "\t<Coroutine - Finished>\n";
	co_return;
}

int main() {
	std::cout << "main - Starting coroutine\n";
	auto task = GetFileData();
	task.open_file("main.cpp");
	while (!task.m_Handle.done()) {
		task.m_Handle.resume();
		std::cout << task.get_read_bytes() << '\n';
		std::cout << "[ main running ]\n";
	}
	std::cout << "\nmain - Coroutine is done!\n";
}