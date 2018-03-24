#include "utf8.hpp"

using namespace utf8;

#include <algorithm>
#include <iostream>

//	#include "catch.hpp"
#include "doctest.h"

bool octet_equal(const OctetT &a, const char &b) {
	return a == b;
}

bool cp_equal(const CodepointT &a, const CodepointT &b) {
	return a == b;
}

TEST_SUITE("[ctor] constructor works") {

	TEST_CASE("[ctor0] default constructor works") {
		rope rp;
		auto a = rp.data().get();

		REQUIRE(rp.size() == 0);
		REQUIRE(std::equal(a, a, "", octet_equal));
	}

	TEST_CASE("[ctor1a] constructor (octet version) taking (count, ch) works") {
		rope rp(5, 'c');
		auto a = rp.data().get();

		REQUIRE(rp.size() == 5);
		REQUIRE(rp.at(0) == 'c');
		REQUIRE(rp.at(1) == 'c');
		REQUIRE(rp.at(2) == 'c');
		REQUIRE(rp.at(3) == 'c');
		REQUIRE(rp.at(4) == 'c');
	}

	TEST_CASE("[ctor1b] constructor (codepoint version) taking (count, ch) works") {
		//	門
		rope rp(4, static_cast<CodepointT>(0x9580), rope::mode::cp);

		REQUIRE(rp.size_cp() == 4);
		REQUIRE(rp.at_cp(0) == 0x9580);
		REQUIRE(rp.at_cp(1) == 0x9580);
		REQUIRE(rp.at_cp(2) == 0x9580);
		REQUIRE(rp.at_cp(3) == 0x9580);

		REQUIRE(rp.size() == 12);
		REQUIRE(rp.at(0) == 0xe9);
		REQUIRE(rp.at(1) == 0x96);
		REQUIRE(rp.at(2) == 0x80);
		REQUIRE(rp.at(3) == 0xe9);
		REQUIRE(rp.at(4) == 0x96);
		REQUIRE(rp.at(5) == 0x80);
		REQUIRE(rp.at(6) == 0xe9);
		REQUIRE(rp.at(7) == 0x96);
		REQUIRE(rp.at(8) == 0x80);
		REQUIRE(rp.at(9) == 0xe9);
		REQUIRE(rp.at(10) == 0x96);
		REQUIRE(rp.at(11) == 0x80);
	}

	/*
	TEST_CASE("[ctor2a] constructor (octet version) taking (other, pos, count) works") {
		rope rp("12345"), rq(rp, 1, 3);

		REQUIRE(rp.size() == 3);
		REQUIRE(rp.at(0) == '2');
		REQUIRE(rp.at(1) == '3');
		REQUIRE(rp.at(2) == '4');
	}
	*/

	TEST_CASE("[ctor3a] constructor (octet version) taking (s, count) works") {
		rope rp("dzongkha", 5);

		REQUIRE(rp.size() == 5);
		REQUIRE(rp.at(0) == 'd');
		REQUIRE(rp.at(1) == 'z');
		REQUIRE(rp.at(2) == 'o');
		REQUIRE(rp.at(3) == 'n');
		REQUIRE(rp.at(4) == 'g');
	}

	TEST_CASE("[ctor3b] constructor (codepoint version) taking (s, count) works") {
		//	頓珍漢
		int a[] = { 0x9813, 0x73cd, 0x6f22 };
		rope rp(a, 2, rope::mode::cp);

		REQUIRE(rp.size_cp() == 2);
		REQUIRE(rp.at_cp(0) == 0x9813);
		REQUIRE(rp.at_cp(1) == 0x73cd);

		REQUIRE(rp.size() == 6);
		REQUIRE(rp.at(0) == 0xe9);
		REQUIRE(rp.at(1) == 0xa0);
		REQUIRE(rp.at(2) == 0x93);
		REQUIRE(rp.at(3) == 0xe7);
		REQUIRE(rp.at(4) == 0x8f);
		REQUIRE(rp.at(5) == 0x8d);
	}

	TEST_CASE("[ctor4a] constructor (octet version) taking (s) works") {
		rope rp("dzongkha");

		REQUIRE(rp.size() == 8);
		REQUIRE(rp.at(0) == 'd');
		REQUIRE(rp.at(1) == 'z');
		REQUIRE(rp.at(2) == 'o');
		REQUIRE(rp.at(3) == 'n');
		REQUIRE(rp.at(4) == 'g');
		REQUIRE(rp.at(5) == 'k');
		REQUIRE(rp.at(6) == 'h');
		REQUIRE(rp.at(7) == 'a');
	}

	TEST_CASE("[ctor4b] constructor (codepoint version) taking (s) works") {
		//	頓珍漢
		int a[] = { 0x9813, 0x73cd, 0x6f22, 0 };
		rope rp(a, rope::mode::cp);

		REQUIRE(rp.size_cp() == 3);
		REQUIRE(rp.at_cp(0) == 0x9813);
		REQUIRE(rp.at_cp(1) == 0x73cd);
		REQUIRE(rp.at_cp(2) == 0x6f22);

		REQUIRE(rp.size() == 9);
		REQUIRE(rp.at(0) == 0xe9);
		REQUIRE(rp.at(1) == 0xa0);
		REQUIRE(rp.at(2) == 0x93);
		REQUIRE(rp.at(3) == 0xe7);
		REQUIRE(rp.at(4) == 0x8f);
		REQUIRE(rp.at(5) == 0x8d);
		REQUIRE(rp.at(6) == 0xe6);
		REQUIRE(rp.at(7) == 0xbc);
		REQUIRE(rp.at(8) == 0xa2);
	}

	TEST_CASE("[ctor5a] constructor (octet version) taking (first, last) works") {
		std::string ziering = "ziering";
		rope rp(ziering.crbegin(), ziering.crend());

		REQUIRE(rp.size() == 7);
		REQUIRE(rp.at(0) == 'g');
		REQUIRE(rp.at(1) == 'n');
		REQUIRE(rp.at(2) == 'i');
		REQUIRE(rp.at(3) == 'r');
		REQUIRE(rp.at(4) == 'e');
		REQUIRE(rp.at(5) == 'i');
		REQUIRE(rp.at(6) == 'z');
	}

	TEST_CASE("[ctor5b] constructor (codepoint version) taking (first, last) works") {
		//	甲斐
		CodepointT a[] = { 0x7532, 0x6590 };
		rope rp(a, a + 2, rope::mode::cp);

		REQUIRE(rp.size_cp() == 2);
		REQUIRE(rp.at_cp(0) == 0x7532);
		REQUIRE(rp.at_cp(1) == 0x6590);

		REQUIRE(rp.size() == 6);
		REQUIRE(rp.at(0) == 0xe7);
		REQUIRE(rp.at(1) == 0x94);
		REQUIRE(rp.at(2) == 0xb2);
		REQUIRE(rp.at(3) == 0xe6);
		REQUIRE(rp.at(4) == 0x96);
		REQUIRE(rp.at(5) == 0x90);
	}

	TEST_CASE("[ctor6] copy constructor works") {
		rope rp("copycat"), rq(rp);

		REQUIRE(rq.size() == 7);
		REQUIRE(rq.at(0) == 'c');
		REQUIRE(rq.at(1) == 'o');
		REQUIRE(rq.at(2) == 'p');
		REQUIRE(rq.at(3) == 'y');
		REQUIRE(rq.at(4) == 'c');
		REQUIRE(rq.at(5) == 'a');
		REQUIRE(rq.at(6) == 't');
	}

	/*
	TEST_CASE("[ctor7] move constructor works") {}
	*/

	TEST_CASE("[ctor8a] constructor (octet version) taking (init) works") {
		rope rp({ 'c', 'o', 'n', 'f', 'e', 'r' });

		REQUIRE(rp.size() == 6);
		REQUIRE(rp.at(0) == 'c');
		REQUIRE(rp.at(1) == 'o');
		REQUIRE(rp.at(2) == 'n');
		REQUIRE(rp.at(3) == 'f');
		REQUIRE(rp.at(4) == 'e');
		REQUIRE(rp.at(5) == 'r');
	}

	TEST_CASE("[ctor8b] constructor (codepoint version) taking (init) works") {
		//	超硬合金
		rope rp({ 0x8d85, 0x786c, 0x5408, 0x91d1 }, rope::mode::cp);

		REQUIRE(rp.size_cp() == 4);
		REQUIRE(rp.at_cp(0) == 0x8d85);
		REQUIRE(rp.at_cp(1) == 0x786c);
		REQUIRE(rp.at_cp(2) == 0x5408);
		REQUIRE(rp.at_cp(3) == 0x91d1);

		REQUIRE(rp.size() == 12);
		REQUIRE(rp.at(0) == 0xe8);
		REQUIRE(rp.at(1) == 0xb6);
		REQUIRE(rp.at(2) == 0x85);
		REQUIRE(rp.at(3) == 0xe7);
		REQUIRE(rp.at(4) == 0xa1);
		REQUIRE(rp.at(5) == 0xac);
		REQUIRE(rp.at(6) == 0xe5);
		REQUIRE(rp.at(7) == 0x90);
		REQUIRE(rp.at(8) == 0x88);
		REQUIRE(rp.at(9) == 0xe9);
		REQUIRE(rp.at(10) == 0x87);
		REQUIRE(rp.at(11) == 0x91);
	}

	TEST_CASE("[ctor9a] constructor (octet version) taking (str) works") {
		std::string wallet = "wallet";
		rope rp(wallet);

		REQUIRE(rp.size() == 6);
		REQUIRE(rp.at(0) == 'w');
		REQUIRE(rp.at(1) == 'a');
		REQUIRE(rp.at(2) == 'l');
		REQUIRE(rp.at(3) == 'l');
		REQUIRE(rp.at(4) == 'e');
		REQUIRE(rp.at(5) == 't');
	}

}

TEST_SUITE("[at] at() works") {

	TEST_CASE("[at] at() works") {
		rope rp("12345");

		REQUIRE(rp.at(0) == '1');
		REQUIRE(rp.at(1) == '2');
		REQUIRE(rp.at(2) == '3');
		REQUIRE(rp.at(3) == '4');
		REQUIRE(rp.at(4) == '5');
	}

	TEST_CASE("[at_cp] at_cp() works") {
		rope rp("12345");

		REQUIRE(rp.at_cp(0) == '1');
		REQUIRE(rp.at_cp(1) == '2');
		REQUIRE(rp.at_cp(2) == '3');
		REQUIRE(rp.at_cp(3) == '4');
		REQUIRE(rp.at_cp(4) == '5');
	}

}

TEST_SUITE("[front] front() works") {

	TEST_CASE("[front] front() works") {
		rope rp("12345");

		REQUIRE(rp.front() == '1');
	}

	TEST_CASE("[front_cp] front_cp() works") {
		rope rp(4, static_cast<CodepointT>(0x9580), rope::mode::cp);

		REQUIRE(rp.front_cp() == 0x9580);
	}

}

TEST_SUITE("[back] back() works") {

	TEST_CASE("[back] back() works") {
		rope rp("12345");

		REQUIRE(rp.back() == '5');
	}

	TEST_CASE("[back_cp] back_cp() works") {
		rope rp(4, static_cast<CodepointT>(0x9580), rope::mode::cp);

		REQUIRE(rp.back_cp() == 0x9580);
	}

}

TEST_SUITE("[insert] insert() works") {

	TEST_CASE("[insert1a] insert() taking (index = 0, count, ch) on an empty rope works") {
		rope rp("");
		rp.insert(0, 2, 'l');

		REQUIRE(rp.size() == 2);
		REQUIRE(rp.at(0) == 'l');
		REQUIRE(rp.at(1) == 'l');
	}

	TEST_CASE("[insert1b] insert() taking (index = 0, count, ch) on a non-empty rope works") {
		rope rp("rdvark");
		rp.insert(0, 2, 'a');

		REQUIRE(rp.size() == 8);
		REQUIRE(rp.at(0) == 'a');
		REQUIRE(rp.at(1) == 'a');
		REQUIRE(rp.at(2) == 'r');
		REQUIRE(rp.at(3) == 'd');
		REQUIRE(rp.at(4) == 'v');
		REQUIRE(rp.at(5) == 'a');
		REQUIRE(rp.at(6) == 'r');
		REQUIRE(rp.at(7) == 'k');
	}

	TEST_CASE("[insert1c] insert() taking (index = size(), count, ch) on a non-empty rope works") {
		rope rp("b");
		rp.insert(1, 2, 'e');

		REQUIRE(rp.size() == 3);
		REQUIRE(rp.at(0) == 'b');
		REQUIRE(rp.at(1) == 'e');
		REQUIRE(rp.at(2) == 'e');
	}

	TEST_CASE("[insert1d] insert() taking (index > 0, count, ch) on a non-empty rope works") {
		rope rp("heo");
		rp.insert(2, 2, 'l');

		REQUIRE(rp.size() == 5);
		REQUIRE(rp.at(0) == 'h');
		REQUIRE(rp.at(1) == 'e');
		REQUIRE(rp.at(2) == 'l');
		REQUIRE(rp.at(3) == 'l');
		REQUIRE(rp.at(4) == 'o');
	}

	TEST_CASE("[insert2d] insert() taking (index > 0, s) on a non-empty rope works") {
		rope rp("unity");
		rp.insert(3, "versi");

		REQUIRE(rp.size() == 10);
		REQUIRE(rp.at(0) == 'u');
		REQUIRE(rp.at(1) == 'n');
		REQUIRE(rp.at(2) == 'i');
		REQUIRE(rp.at(3) == 'v');
		REQUIRE(rp.at(4) == 'e');
		REQUIRE(rp.at(5) == 'r');
		REQUIRE(rp.at(6) == 's');
		REQUIRE(rp.at(7) == 'i');
		REQUIRE(rp.at(8) == 't');
		REQUIRE(rp.at(9) == 'y');
	}

	TEST_CASE("[insert3d] insert() taking (index > 0, s, count) on a non-empty rope works") {
		rope rp("france");
		rp.insert(2, "agriculture", 3);

		REQUIRE(rp.size() == 9);
		REQUIRE(rp.at(0) == 'f');
		REQUIRE(rp.at(1) == 'r');
		REQUIRE(rp.at(2) == 'a');
		REQUIRE(rp.at(3) == 'g');
		REQUIRE(rp.at(4) == 'r');
		REQUIRE(rp.at(5) == 'a');
		REQUIRE(rp.at(6) == 'n');
		REQUIRE(rp.at(7) == 'c');
		REQUIRE(rp.at(8) == 'e');
	}

	TEST_CASE("[insert4d] insert() taking (index > 0, str) on a non-empty rope works") {
		rope rp("cale");
		rp.insert(2, std::string("rlis"));

		REQUIRE(rp.size() == 8);
		REQUIRE(rp.at(0) == 'c');
		REQUIRE(rp.at(1) == 'a');
		REQUIRE(rp.at(2) == 'r');
		REQUIRE(rp.at(3) == 'l');
		REQUIRE(rp.at(4) == 'i');
		REQUIRE(rp.at(5) == 's');
		REQUIRE(rp.at(6) == 'l');
		REQUIRE(rp.at(7) == 'e');
	}

}

/*
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
}*/
