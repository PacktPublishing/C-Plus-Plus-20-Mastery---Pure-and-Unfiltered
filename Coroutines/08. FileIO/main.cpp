#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
std::string GetFileData(std::ifstream &input, int bytes) {
	std::string read_bytes{} ;
	read_bytes.resize(bytes) ;
	using namespace std::chrono_literals ;
	std::this_thread::sleep_for(1s) ;
	input.read(read_bytes.data(), bytes) ;
	return read_bytes ;
}

int main() {
	std::ifstream input{"main.cpp"} ;
	if(!input) {
		throw std::logic_error{"Cannot open file for reading"} ;
	}
	std::cout << GetFileData(input, 8) << '\n' ;
	std::cout << GetFileData(input, 15) << '\n' ;
	std::cout << GetFileData(input, 20) << '\n' ;
	std::cout << GetFileData(input, 6) << '\n' ;

	std::cout << "[ main running ]\n" ;
}
