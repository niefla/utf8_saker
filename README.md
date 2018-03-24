# utf8_saker

[![Build Status](https://travis-ci.org/niefla/utf8_saker.svg?branch=develop)](https://travis-ci.org/niefla/utf8_saker)

UTF-8 things made with C++11.

## how to use

Just put `utf8.hpp` inside your project and `#include` it.

## features

*	`utf8::cvt`
	*	Conversion between UTF-8 octets stored in `std::basic_string<uint8_t>` and codepoints stored in `std::basic_string<uint32_t>`.
*	`utf8::validate`
	*	Validation for UTF-8 octet and codepoint sequences.

## to-do's

*	Rope and iterators
*	Regular expressions
	*	Implement `std::regex_traits<uint32_t>` for codepoints. Can probably ignore locales and provide our own character classes (Unicode-based).
*	Miscellaneous
	*	BOM support
	*	Need character class supports; multiple codepoints can map to one character. Becomes a problem in the test since it reverses strings by codepoints, not by characters.
