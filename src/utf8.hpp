/*
MIT License

Copyright (c) 2018 niefla

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef UTF8_HPP_KNACKNING
#define UTF8_HPP_KNACKNING

#include <string>
#include <memory>
#include <iterator>
#include <tuple>
#include <stdexcept>

//	printf debugging best debugging
/*#include <iostream>
#include <bitset>*/

namespace utf8 {

	using cp_string = std::basic_string<uint32_t>;
	using utf8_string = std::basic_string<uint8_t>;

	std::ostream &operator<<(std::ostream &lhs, const cp_string &rhs) {
		for (const uint32_t &ch : rhs) {
			lhs << ch;
		}
		return lhs;
	}

	std::ostream &operator<<(std::ostream &lhs, const utf8_string &rhs) {
		for (const uint8_t &ch : rhs) {
			lhs << ch;
		}
		return lhs;
	}

	/*	the heart of this implementation */
	namespace core {

		template <typename It>
		std::tuple<uint32_t, size_t> get_cp_and_size_from_utf8(const It &octet_iter) {
			uint32_t cp = 0;
			size_t size = 0;

			uint8_t ch = static_cast<uint8_t>(*octet_iter);

			if ((ch & 0b10000000) == 0b00000000) {
				cp |= ch & 0b01111111;
				size = 1;
			}
			else if ((ch & 0b11100000) == 0b11000000) {
				cp |= ch & 0b00011111;
				cp <<= 6;
				cp |= static_cast<uint8_t>(*(octet_iter + 1)) & 0b00111111;

				if (cp < 0x80) {
					/*	invalid utf-8 */
					throw std::runtime_error("invalid codepoint taking 2 octets");
				}

				size = 2;
			}
			else if ((ch & 0b11110000) == 0b11100000) {
				cp |= ch & 0b00001111;
				cp <<= 6;
				cp |= static_cast<uint8_t>(*(octet_iter + 1)) & 0b00111111;
				cp <<= 6;
				cp |= static_cast<uint8_t>(*(octet_iter + 2)) & 0b00111111;

				if (cp < 0x800) {
					/*	invalid utf-8 */
					throw std::runtime_error("invalid codepoint taking 3 octets");
				}

				size = 3;
			}
			else if ((ch & 0b11111000) == 0b11110000) {
				cp |= ch & 0b00000111;
				cp <<= 6;
				cp |= static_cast<uint8_t>(*(octet_iter + 1)) & 0b00111111;
				cp <<= 6;
				cp |= static_cast<uint8_t>(*(octet_iter + 2)) & 0b00111111;
				cp <<= 6;
				cp |= static_cast<uint8_t>(*(octet_iter + 3)) & 0b00111111;

				if (cp < 0x10000) {
					/*	invalid utf-8 */
					throw std::runtime_error("invalid codepoint taking 4 octets");
				}

				size = 4;
			}
			else {
				/*	invalid utf-8 */
				throw std::runtime_error("invalid utf8");
			}

			return std::make_tuple(cp, size);
		}

		template <typename It>
		std::tuple<uint8_t, uint8_t, uint8_t, uint8_t, size_t> get_utf8_and_size_from_cp(const It &cp_iter) {
			uint8_t octet0 = 0, octet1 = 0, octet2 = 0, octet3 = 0;
			size_t size = 0;

			uint32_t cp = static_cast<uint32_t>(*cp_iter);

			if (cp < 0x80) {
				octet0 = cp;
				size = 1;
			}
			else if (cp < 0x800) {
				octet0 = 0b11000000 | (cp >> 6);
				octet1 = 0b10000000 | (cp & 0b111111);
				size = 2;
			}
			else if (cp < 0x10000) {
				octet0 = 0b11100000 | (cp >> 12);
				octet1 = 0b10000000 | ((cp >> 6) & 0b111111);
				octet2 = 0b10000000 | (cp & 0b111111);
				size = 3;
			}
			else if (cp < 0x110000) {
				octet0 = 0b11100000 | (cp >> 18);
				octet1 = 0b10000000 | ((cp >> 12) & 0b111111);
				octet2 = 0b10000000 | ((cp >> 6) & 0b111111);
				octet3 = 0b10000000 | (cp & 0b111111);
				size = 4;
			}
			else {
				/*	invalid codepoint */
				throw std::runtime_error("invalid codepoint");
			}

			return std::make_tuple(octet0, octet1, octet2, octet3, size);
		}

	}

	namespace cvt {

		template <typename It>
		cp_string cp_from_utf8(It utf8_first, It utf8_last) {
			cp_string res;

			while (utf8_first != utf8_last) {
				uint32_t cp;
				size_t size;

				std::tie(cp, size) = core::get_cp_and_size_from_utf8(utf8_first);

				res.push_back(cp);
				utf8_first += size;
			}

			return res;
		}

		template <typename It>
		utf8_string utf8_from_cp(It cp_first, It cp_last) {
			utf8_string res;

			while (cp_first != cp_last) {
				uint8_t octet0, octet1, octet2, octet3;
				size_t size;

				std::tie(octet0, octet1, octet2, octet3, size) = core::get_utf8_and_size_from_cp(cp_first);

				res.push_back(octet0);
				if (size > 1) {
					res.push_back(octet1);
				}
				if (size > 2) {
					res.push_back(octet2);
				}
				if (size > 3) {
					res.push_back(octet3);
				}
				++cp_first;
			}

			return res;
		}

	}

	namespace validate {

		template <typename It>
		bool is_utf8_seq(It utf8_first, It utf8_last) {
			while (utf8_first != utf8_last) {
				uint32_t cp;
				size_t size;

				try {
					std::tie(cp, size) = core::get_cp_and_size_from_utf8(utf8_first);
				}
				catch (const std::runtime_error&) {
					return false;
				}

				utf8_first += size;
			}

			return true;
		}

		template <typename It>
		bool is_cp_seq(It cp_first, It cp_last) {
			for (; cp_first != cp_last; ++cp_first) {
				if (*cp_first > 0x10ffff) {
					return false;
				}
			}

			return true;
		}

	}

	class rope {

		using value_type = uint8_t;
		using reference = value_type&;
		using const_reference = const value_type&;
		class iterator {};
		class const_iterator ();
		using difference_type = std::iterator_traits<const_iterator>::difference_type;
		using size_type = size_t;

	};

	/* does not make sense yet, should write own rope-like container
	class iterator {};

	class const_iterator {

	private:

		uint8_t *ptr;

	public:

		using difference_type = std::ptrdiff_t;
		using value_type = const uint32_t;
		using pointer = const uint32_t*;
		using reference = const uint32_t&;

		//	default constructor
		iterator() : ptr(nullptr) {}

		//	copy constructor
		iterator(const iterator &copy_from) = default;

		//	move constructor
		iterator(iterator &&other) noexcept : iterator() {
			swap(*this, other);
		}

		//	destructor
		~iterator() noexcept = default;

		//	copy assignment operator
		iterator& operator=(iterator other) {
			swap(*this, other);
			return *this;
		}

		//	move assignment operator
		iterator& operator=(iterator &&other) noexcept {
			swap(*this, other);
			return *this;
		}

		//	swap
		friend void swap(iterator &i, iterator &j) {
			using std::swap;
			swap(i.ptr, j.ptr);
		}

		reference operator*() const {
			return;
		}
	};*/

}

/*
	specialization for std::iterator_traits<utf8::iterator>
*/
/*
namespace std {

	template <>
	struct iterator_traits<utf8::iterator> {
		using difference_type = typename utf8::iterator::difference_type;
		using value_type = typename utf8::iterator::value_type;
		using pointer = typename utf8::iterator::pointer;
		using reference = typename utf8::iterator::reference;
		using iterator_category = std::random_access_iterator_tag;
	};

}*/

#endif
