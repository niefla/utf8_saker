#include "utf8.hpp"

#include <iostream>
#include <fstream>
#include <regex>
#include <vector>
#include <sstream>

using namespace utf8;

void test_reverse() {
	std::ifstream reader("dummy");
	if (!reader.is_open()) {
		return;
	}

	std::ofstream writer("dummy.reverse", std::ios::trunc);
	if (!writer.is_open()) {
		return;
	}

	std::string line;
	while (std::getline(reader, line)) {
		cp_string cp = cvt::cp_from_utf8(line.cbegin(), line.cend());
		utf8_string reversed = cvt::utf8_from_cp(cp.rbegin(), cp.rend());

		writer << reversed << std::endl;
	}
}

void test_regex() {
	std::ifstream reader("regex.pattern");
	if (!reader.is_open()) {
		return;
	}
	std::string line;

	std::vector<std::basic_regex<uint32_t>> patterns;
	while (std::getline(reader, line)) {
		std::basic_string<uint32_t> cps = cvt::cp_from_utf8(line.cbegin(), line.cend());
		try {
			patterns.push_back(std::basic_regex<uint32_t>(cps));
		}
		catch (const std::bad_cast&) {
			std::cout << "sad\n";
		}
	}

	reader.close();
	reader.open("regex.in");
	if (!reader.is_open()) {
		return;
	}

	std::ofstream writer("regex.out", std::ios::trunc);
	if (!writer.is_open()) {
		return;
	}

	while (std::getline(reader, line)) {
		std::basic_string<uint32_t> cp = cvt::cp_from_utf8(line.cbegin(), line.cend());

		for (size_t i = 0; i < patterns.size(); ++i) {
			cp = std::regex_replace(
				cp, patterns[i], std::basic_string<uint32_t>()
			);
		}

		std::basic_string<uint8_t> utf8_out = cvt::utf8_from_cp(cp.cbegin(), cp.cend());
		for (const auto &ch : utf8_out) {
			writer << ch;
		}

		writer << std::endl;
	}
}

int main() {
	test_reverse();
	//	test_regex();

	return 0;
}
