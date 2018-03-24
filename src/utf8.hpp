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
#include <memory>
#include <algorithm>
#include <initializer_list>
#include <type_traits>
#include <utility>

//	printf debugging best debugging
#include <iostream>
#include <bitset>

/**
 *	\namespace utf8
 *	One namespace to bring them all.
 */
namespace utf8 {

	using OctetT = uint8_t;
	using CodepointT = uint32_t;

	using octet_string = std::basic_string<OctetT>;
	using cp_string = std::basic_string<CodepointT>;

	/**
	 *	\fn std::ostream &utf8::operator<<(std::ostream &lhs, const cp_string &rhs)
 	 *	Writes the codepoints in `rhs` to the output stream `lhs`.
	 */
	std::ostream &operator<<(std::ostream &lhs, const cp_string &rhs) {
		for (const uint32_t &ch : rhs) {
			lhs << ch;
		}
		return lhs;
	}

	/**
	 *	\fn std::ostream &utf8::operator<<(std::ostream &lhs, const octet_string &rhs)
 	 *	Writes the octets in `rhs` to the output stream `lhs`.
	 */
	std::ostream &operator<<(std::ostream &lhs, const octet_string &rhs) {
		for (const uint8_t &ch : rhs) {
			lhs << ch;
		}
		return lhs;
	}

	/**
	 *	\namespace utf8::core
	 *	The heart of this implementation. Single functions that are really miscellaneous but too important to be put in the namespace misc.
	 */
	namespace core {

		/**
		 *	\fn std::tuple<uint32_t, size_t> utf8::core::get_cp_and_size_from_utf8(const It &octet_iter)
	 	 *	Reads as many octets as required from `octet_iter` and returns the codepoint encoded by these octet(s) and the number of octets read, in a tuple.
	 	 *	\param		octet_iter		Iterator to an octet sequence.
	 	 *	\return		A tuple whose first element is the codepoint encoded by the octet(s) beginning from `octet_iter` and whose second element is the number of octet(s) that encode the codepoint.
	 	 *	\exception	runtime_error	Thrown if the octet(s) do not satisfy UTF-8 or the encoded codepoint is not a valid Unicode codepoint.
		 */
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
				throw std::runtime_error("invalid utf-8");
			}

			return std::make_tuple(cp, size);
		}

		/**
		 *	\fn std::tuple<uint8_t, uint8_t, uint8_t, uint8_t, size_t> utf8::core::get_utf8_and_size_from_cp(const uint32_t &cp)
	 	 *	Returns the octet(s) that encode the codepoint `cp` and the number of octets used in the encoding, in a tuple. The octets not used in the encoding will be set to 0, and those used should be considered in the order of increasing index in the tuple. That is, if `cp` has a 2-octet encoding, the first and the second octets will encode the codepoint, whereas the third and the fourth octets will be set to 0.
	 	 *	\param		cp				Codepoint to extract the octets from.
	 	 *	\return		A tuple whose first element is the codepoint encoded by the octet(s) beginning from `octet_iter` and whose second element is the number of octet(s) that encode the codepoint.
	 	 *	\exception	runtime_error	Thrown if the codepoint is not a valid Unicode codepoint.
		 */
		std::tuple<uint8_t, uint8_t, uint8_t, uint8_t, size_t> get_utf8_and_size_from_cp(const uint32_t &cp) {
			uint8_t octet0 = 0, octet1 = 0, octet2 = 0, octet3 = 0;
			size_t size = 0;

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

	/**
	 *	\namespace utf8::cvt
	 *	Conversion functions between octet sequences and codepoint sequences.
	 */
	namespace cvt {

		/**
		 *	\fn cp_string utf8::cvt::cp_from_utf8(It utf8_first, It utf8_last)
	 	 *	Converts the octets in the range specified by the iterators into a codepoint sequence.
	 	 *	\param		utf8_first		Iterator pointing to the first octet to be considered.
	 	 *	\param		utf8_last		Iterator pointing to one past the last octet to be considered.
	 	 *	\return		A string containing the codepoints encoded by the read octets.
		 */
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

		/**
		 *	\fn utf8_string utf8::cvt::utf8_from_cp(It cp_first, It cp_last)
	 	 *	Converts the codepoints in the range specified by the iterators into an octet sequence.
	 	 *	\param		cp_first		Iterator pointing to the first codepoint to be considered.
	 	 *	\param		cp_last			Iterator pointing to one past the last codepoint to be considered.
	 	 *	\return		A string containing the octets encoded by the read codepoints.
		 */
		template <typename It>
		octet_string utf8_from_cp(It cp_first, It cp_last) {
			octet_string res;

			for (; cp_first != cp_last; ++cp_first) {
				uint8_t octet0, octet1, octet2, octet3;
				size_t size;

				std::tie(octet0, octet1, octet2, octet3, size) = core::get_utf8_and_size_from_cp(*cp_first);

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
			}

			return res;
		}

	}

	/**
	 *	\namespace utf8::validate
	 *	Validation functions for octet sequences and codepoint sequences.
	 */
	namespace validate {

		/**
		 *	\fn bool utf8::validate::is_utf8_seq(It utf8_first, It utf8_last)
		 *	Checks if the octets in the range specified by the iterators are valid UTF-8 octets.
		 *	\param		utf8_first		Iterator pointing to the first octet to be considered.
		 *	\param		utf8_last		Iterator pointing to one past the last octet to be considered.
		 *	\return		`true` iff the octets in the range are valid UTF-8 octets.
		 */
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

		/**
		 *	\fn bool utf8::validate::is_cp_seq(It cp_first, It cp_last)
		 *	Checks if the codepoints in the range specified by the iterators are valid Unicode codepoints.
		 *	\param		cp_first		Iterator pointing to the first codepoint to be considered.
		 *	\param		cp_last			Iterator pointing to one past the last codepoint to be considered.
		 *	\return		`true` iff the codepoints in the range are valid Unicode codepoints.
		 */
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

	/**
	 *	\class utf8::rope
	 *	A rope implementation that stores octets and provides a `std::basic_string`-like interface for both octets and codepoints.
	 */
	class rope {

	public:

		//	typedef's like templates
		using Traits = std::char_traits<OctetT>;
		using Allocator = std::allocator<OctetT>;

		using traits_type = Traits;
		using allocator_type = Allocator;

		using octet_value_type = OctetT;
		using octet_reference = octet_value_type&;
		using const_octet_reference = const octet_value_type&;
		class octet_iterator {};
		class const_octet_iterator {};

		using cp_value_type = CodepointT;
		using cp_reference = cp_value_type&;
		using const_cp_reference = const cp_value_type&;
		class cp_iterator {};
		class const_cp_iterator {};

		using difference_type = std::allocator_traits<Allocator>::difference_type;
		using size_type = std::allocator_traits<Allocator>::size_type;

	private:

		//	not a class, just an aggregate
		struct rope_node {

			//	len and weight store the number of bytes. should we store the number of characters too? is there a guarantee that a character does not cross rope_node's?
			//	unless the user invalidates the rope (e.g. by inserting bytes), a codepoint does not span multiple rope_node's.
			//	functions that can potentially invalidate the rope should have some sort of warning functionality.

			//	len_octet if leaf, sum of weight_octet's in the left subtree if nonleaf
			size_type weight_octet = 0;

			//	len_cp if leaf, sum of weight_cp's in the left subtree if nonleaf
			size_type weight_cp = 0;

			//	number of bytes allocated to ptr. should be zero if nonleaf
			size_type len_octet = 0;

			//	number of codepoints stored in ptr. should be zero if nonleaf
			size_type len_cp = 0;

			//	address at which bytes are allocated. should be nullptr if nonleaf
			//	(bool)node->ptr is true iff leaf
			uint8_t *ptr = nullptr;

			//	left and right subtree. should be nullptr if leaf
			std::shared_ptr<rope_node> left = std::shared_ptr<rope_node>();
			std::shared_ptr<rope_node> right = std::shared_ptr<rope_node>();

			//	need access to allocator
			Allocator *allocator = nullptr;

			~rope_node() {
				if (ptr) {
					allocator->deallocate(ptr, len_octet);
				}

				left.reset();
				right.reset();
			}

		};

		//	node-building functions
		std::shared_ptr<rope_node> make_node() {
			std::shared_ptr<rope_node> node = std::make_shared<rope_node>();
			node->allocator = &allocator;
			return node;
		}

		void set_leaf(
			const std::shared_ptr<rope_node> &leaf,
			OctetT *seq,
			size_type len_octet,
			size_type len_cp
		) {
			leaf->ptr = seq;
			leaf->len_octet = len_octet;
			leaf->len_cp = len_cp;
			leaf->weight_octet = len_octet;
			leaf->weight_cp = len_cp;
			leaf->left.reset();
			leaf->right.reset();
		}

		void set_leaf(
			const std::shared_ptr<rope_node> &leaf,
			OctetT *seq,
			size_type len_octet
		) {
			set_leaf(leaf, seq, len_octet, calc_len_cp(seq, len_octet));
		}

		std::pair<size_type, size_type> set_branch(
			const std::shared_ptr<rope_node> &branch,
			const std::shared_ptr<rope_node> &left = {},
			const std::shared_ptr<rope_node> &right = {}
		) {
			branch->ptr = nullptr;
			branch->len_octet = 0;
			branch->len_cp = 0;
			branch->left = left;
			branch->right = right;
			return calc_weight(branch);
		}

		std::shared_ptr<rope_node> make_leaf(
			OctetT *seq,
			size_type len_octet,
			size_type len_cp
		) {
			std::shared_ptr<rope_node> leaf = make_node();
			set_leaf(leaf, seq, len_octet, len_cp);
			return leaf;
		}

		std::shared_ptr<rope_node> make_leaf(
			OctetT *seq,
			size_type len_octet
		) {
			return make_leaf(seq, len_octet, calc_len_cp(seq, len_octet));
		}

		std::tuple<std::shared_ptr<rope_node>, size_type, size_type> make_branch(
			const std::shared_ptr<rope_node> &left = {},
			const std::shared_ptr<rope_node> &right = {}
		) {
			std::shared_ptr<rope_node> branch = make_node();
			size_type weight_octet, weight_cp;
			std::tie(weight_octet, weight_cp) = set_branch(branch, left, right);
			return std::make_tuple(branch, weight_octet, weight_cp);
		}

	public:

		enum struct mode {
			octet = 0, cp = 1
		};

		static const size_type npos = -1;

		/**
		 *	\name Constructor
		 *	@{
		 *	\fn utf8::rope::rope(const Allocator &alloc = Allocator())
		 *	Default constructor. Corresponds to (1) of [`std::basic_string::basic_string()`](http://en.cppreference.com/w/cpp/string/basic_string/basic_string).
		 *	\param		alloc			Allocator to use for all memory allocations of this rope.
		 */
		explicit rope(const Allocator &alloc = Allocator()) : rope(alloc, make_node(), 0, 0) {}

		/**
		 *	\fn utf8::rope::rope(size_type count, GeneralT ch, const mode &m = mode::octet, const Allocator &alloc = Allocator())
		 *	Constructs the rope with `count` copies of the character `ch`. If `m == mode::octet`, `ch` will be copied as an octet; the number of octets copied will be exactly `count`. If `m == mode::cp`, `ch` will be copied as a codepoint; the number of octets copied will be `k * count`, where `k` is the number of octets used to encode `ch` as a codepoint. Corresponds to (2) of [`std::basic_string::basic_string()`](http://en.cppreference.com/w/cpp/string/basic_string/basic_string).
		 *	\warning	The behavior is undefined if `count >= npos`.
		 *	\param		count			Number of times `ch` will be copied.
		 *	\param		ch				Character to be copied. This may represent an octet or a codepoint; see above.
		 *	\param		m				Mode in which the copy will take place.
		 *	\param		alloc			Allocator to use for all memory allocations of this rope.
		 */
		template <typename GeneralT>
		rope(
			size_type count,
			GeneralT ch,
			const mode &m = mode::octet,
			const Allocator &alloc = Allocator()
		) : rope(alloc) {
			OctetT *ptr = nullptr;
			if (m == mode::octet) {
				OctetT octet = static_cast<OctetT>(ch);
				ptr = allocator.allocate(count);
				for (size_t it = 0; it < count; ++it) {
					*(ptr + it) = octet;
				}

				set_leaf(root, ptr, count);
				size_ = count;
				size_cp_ = root->len_cp;
			}
			else {
				CodepointT cp = static_cast<CodepointT>(ch);
				uint8_t octet0, octet1, octet2, octet3;
				size_t cp_size;

				std::tie(octet0, octet1, octet2, octet3, cp_size) = core::get_utf8_and_size_from_cp(cp);

				ptr = allocator.allocate(cp_size * count);
				for (size_t it = 0; it < count; ++it) {
					OctetT *offset_ptr = ptr + it * cp_size;
					*offset_ptr = octet0;
					if (cp_size > 1) {
						*(offset_ptr + 1) = octet1;
					}
					if (cp_size > 2) {
						*(offset_ptr + 2) = octet2;
					}
					if (cp_size > 3) {
						*(offset_ptr + 3) = octet3;
					}
				}

				set_leaf(root, ptr, cp_size * count, count);
				size_ = cp_size * count;
				size_cp_ = count;
			}
		}

		/**
		 *	\fn utf8::rope::rope(const rope &other, size_type pos, size_type count = npos, const mode &m = mode::octet, const Allocator &alloc = Allocator())
	 	 *	Constructs the rope with a subrope `[pos, pos+count[` of `other`. If `count == npos` or `count` is not specified or the requested subrope lasts past the end of the rope, then the resulting subrope is `[pos, size()[`. If `m == mode::octet`, the subrope will consist of `count`-many octets. If `m == mode::cp`, the subrope will consist of `count`-many codepoints. Corresponds to (3) of [`std::basic_string::basic_string()`](http://en.cppreference.com/w/cpp/string/basic_string/basic_string).
	 	 *	\param		other			Rope from which the subrope will be taken.
	 	 *	\param		pos				Index to the first character from `other` to be copied.
	 	 *	\param		count			Number of characters from `other` to be copied.
	 	 *	\param		m				Mode in which the copy will take place.
	 	 *	\param		alloc			Allocator to use for all memory allocations of this rope.
		 *	\todo		To be implemented.
		 */
		rope(
			const rope &other,
			size_type pos,
			size_type count = npos,
			const mode &m = mode::octet,
			const Allocator &alloc = Allocator()
		) : rope(alloc) {
			//	throw std::out_of_range("rope::rope");
		}

		/**
		 *	\fn utf8::rope::rope(const GeneralT *s, size_type count, const mode &m = mode::octet, const Allocator &alloc = Allocator())
	 	 *	Constructs the rope with the first `count` characters of the string pointed to by `s`. `s` can contain zeros. If `m == mode:octet`, `s` is treated as a string of octets. If `m == mode::cp`, `s` is treated as a string of codepoints. Corresponds to (4) of [`std::basic_string::basic_string()`](http://en.cppreference.com/w/cpp/string/basic_string/basic_string).
	 	 *	\warning	The behavior is undefined if `s` does not point at an array of at least `count` elements of `GeneralT`, including the case when `s` is a null pointer.
	 	 *	\param		s				Pointer to an array of characters to use as source to initialize the rope with.
	 	 *	\param		count			Number of characters from `s` to be copied.
	 	 *	\param		m				Mode in which the copy will take place.
	 	 *	\param		alloc			Allocator to use for all memory allocations of this rope.
		 */
		template <typename GeneralT>
		rope(
			const GeneralT *s,
			size_type count,
			const mode &m = mode::octet,
			const Allocator &alloc = Allocator()
		) : rope(alloc) {
			if (m == mode::octet) {
				OctetT *ptr = allocator.allocate(count);
				for (size_t it = 0; it < count; ++it) {
					*(ptr + it) = *(s + it);
				}

				set_leaf(root, ptr, count);
				size_ = count;
				size_cp_ = root->len_cp;
			}
			else {
				CodepointT cp;
				uint8_t octet0, octet1, octet2, octet3;
				size_t cp_size;
				for (size_t it = 0; it < count; ++it) {
					cp = static_cast<CodepointT>(*(s + it));
					std::tie(octet0, octet1, octet2, octet3, cp_size) = core::get_utf8_and_size_from_cp(cp);

					OctetT *ptr = allocator.allocate(cp_size);
					ptr[0] = octet0;
					if (cp_size > 1) {
						ptr[1] = octet1;
					}
					if (cp_size > 2) {
						ptr[2] = octet2;
					}
					if (cp_size > 3) {
						ptr[3] = octet3;
					}

					if (it == 0) {
						root = make_leaf(ptr, cp_size, 1);;
					}
					else {
						std::tie(root, std::ignore, std::ignore) = insert_after_node(root, make_leaf(ptr, cp_size, 1));
					}

					size_ += cp_size;
					++size_cp_;
				}
			}
		}

		/**
		 *	\fn utf8::rope::rope(const GeneralT *s, const mode &m = mode::octet, const Allocator &alloc = Allocator())
		 *	Constructs the rope with the contents initialized with a copy of the null-terminated character string pointed to by `s`. The length of the rope is determined by the first null character. If `m == mode:octet`, `s` is treated as a string of octets. If `m == mode::cp`, `s` is treated as a string of codepoints. Corresponds to (5) of [`std::basic_string::basic_string()`](http://en.cppreference.com/w/cpp/string/basic_string/basic_string).
		 *	\warning	The behavior is undefined if `s` does not point at an array of at least `Traits::length(s)+1` elements of `GeneralT`, including the case when `s` is a null pointer.
	 	 *	\param		s				Pointer to an array of characters to use as source to initialize the rope with.
	 	 *	\param		m				Mode in which the copy will take place.
	 	 *	\param		alloc			Allocator to use for all memory allocations of this rope.
		 */
		template <typename GeneralT>
		rope(
			const GeneralT *s,
			const mode &m = mode::octet,
			const Allocator &alloc = Allocator()
		) : rope(s, std::char_traits<GeneralT>::length(s), m, alloc) {}

		/**
		 *	\fn utf8::rope::rope(InputIt first, InputIt last, const mode &m = mode::octet, const Allocator &alloc = Allocator())
		 *	Constructs the rope with the contents of the range [`first`, `last`). If `InputIt` is an integral type, equivalent to `basic_string(static_cast<size_type>(first), static_cast<value_type>(last), a)`. If `m == mode::octet`, the range is treated as a range of octets. If `m == mode::cp`, the range is treated as a range of codepoints. Corresponds to (6) of [`std::basic_string::basic_string()`](http://en.cppreference.com/w/cpp/string/basic_string/basic_string).
		 *	\param		first, last		Range to copy the characters from.
	 	 *	\param		m				Mode in which the copy will take place.
	 	 *	\param		alloc			Allocator to use for all memory allocations of this rope.
		 */
		template <class InputIt>
		rope(
			InputIt first,
			InputIt last,
			const mode &m = mode::octet,
			const Allocator &alloc = Allocator()
		) : rope(alloc) {
			size_t len = last - first;

			if (m == mode::octet) {
				OctetT *ptr = allocator.allocate(len);
				for (size_t i = 0; i < len; ++i) {
					*(ptr + i) = *(first + i);
				}

				set_leaf(root, ptr, len);
				size_ = len;
				size_cp_ = root->len_cp;
			}
			else {
				CodepointT cp;
				uint8_t octet0, octet1, octet2, octet3;
				size_t cp_size;
				for (size_t i = 0; i < len; ++i) {
					cp = static_cast<CodepointT>(*(first + i));

					std::tie(octet0, octet1, octet2, octet3, cp_size) = core::get_utf8_and_size_from_cp(cp);

					OctetT *ptr = allocator.allocate(cp_size);
					ptr[0] = octet0;
					if (cp_size > 1) {
						ptr[1] = octet1;
					}
					if (cp_size > 2) {
						ptr[2] = octet2;
					}
					if (cp_size > 3) {
						ptr[3] = octet3;
					}

					if (i == 0) {
						root = make_leaf(ptr, cp_size, 1);;
					}
					else {
						std::tie(root, std::ignore, std::ignore) = insert_after_node(root, make_leaf(ptr, cp_size, 1));
					}

					size_ += cp_size;
					++size_cp_;
				}
			}
		}

		/**
		 *	\fn utf8::rope::rope(const rope &other)
		 *	Copy constructor. Constructs the rope with the copy of the contents of `other`. Corresponds to (7) of [`std::basic_string::basic_string()`](http://en.cppreference.com/w/cpp/string/basic_string/basic_string).
		 *	\param		other			Rope to copy contents from.
		 */
		rope(
			const rope &other
		) : rope(other.allocator, copy_node(other.root), other.size_, other.size_cp_) {}

		/**
		 *	\fn utf8::rope::rope(rope &&other, const Allocator &alloc = Allocator())
		 *	Move constructor. Constructs the rope with the contents of `other` using move semantics. `other` is left in valid, but unspecified state. Corresponds to (8) of [`std::basic_string::basic_string()`](http://en.cppreference.com/w/cpp/string/basic_string/basic_string).
		 *	\param		other			Rope to move from.
		 *	\param		alloc			Allocator to use for all memory allocations of this rope.
		 */
		rope(
			rope &&other,
			const Allocator &alloc = Allocator()
		) : rope(alloc) {
			swap(*this, other);
		}

		/**
		 *	\fn utf8::rope::rope(std::initializer_list<GeneralT> init, const mode &m = mode::octet, const Allocator &alloc = Allocator())
		 *	Constructs the rope with the contents of the initializer list `init`. If `m == mode:octet`, `init` is treated as an initializer list of octets. If `m == mode::cp`, `init` is treated as an initializer list of codepoints. Corresponds to (9) of [`std::basic_string::basic_string()`](http://en.cppreference.com/w/cpp/string/basic_string/basic_string).
		 *	\param		init			Initializer list to initialize the rope with.
	 	 *	\param		m				Mode in which the copy will take place.
	 	 *	\param		alloc			Allocator to use for all memory allocations of this rope.
		 */
		template <typename GeneralT>
		rope(
			std::initializer_list<GeneralT> init,
			const mode &m = mode::octet,
			const Allocator &alloc = Allocator()
		) : rope(init.begin(), init.end(), m, alloc) {}

		/**
		 *	\fn utf8::rope::rope(const std::basic_string<GeneralT> &str, const mode &m = mode::octet, const Allocator &alloc = Allocator())
		 *	Constructs the rope with the contents of the string `str`. If `m == mode:octet`, `str` is treated as a string of octets. If `m == mode::cp`, `s` is treated as a string of codepoints.
		 *	\todo		Codepoint behavior to be implemented.
		 *	@}
		 */
		template <typename GeneralT>
		rope(
			const std::basic_string<GeneralT> &str,
			const mode &m = mode::octet,
			const Allocator &alloc = Allocator()
		) : rope(str.cbegin(), str.cend(), m, alloc) {}

		/**
		 *	\name Destructor
		 *	@{
		 *	\fn utf8::rope::~rope()
		 *	Destroys the rope and deallocates all internal storage.
	 	 *	@}
		 */
		~rope() {
			root.reset();
		}

		/*
		//	construct from string literal
		rope(
			const char *s,
			const Allocator &alloc = Allocator()
		) : rope(s, s + std::char_traits<char>::length(s), mode::octet, alloc) {}
		*/
		//	operator=
		//	http://en.cppreference.com/w/cpp/string/basic_string/operator%3D
		//	using templates because operator= takes only one argument

		//	(1) copy assignment
		rope& operator=(rope other) {
			swap(*this, other);
			return *this;
		}

		//	(2) move assignment
		rope& operator=(rope &&other) {
			swap(*this, other);
			return *this;
		}

		//	(3) characters pointed by supplied pointer
		//	CodepointT specialization outside
		template <typename GeneralT>
		rope& operator=(const GeneralT *s) {
			rope tmp(s);
			swap(*this, tmp);
			return *this;
		}

		//	(4) just a single characters
		//	CodepointT specialization outside
		template <typename GeneralT>
		rope& operator=(GeneralT ch) {
			rope tmp(1, ch);
			swap(*this, tmp);
			return *this;
		}

		//	(5) from a initializer list of characters
		//	CodepointT specialization outside
		template <typename GeneralT>
		rope& operator=(std::initializer_list<GeneralT> ilist) {
			rope tmp(ilist.begin(), ilist.end());
			swap(*this, tmp);
			return *this;
		}

		template <typename GeneralT>
		rope& operator=(const std::basic_string<GeneralT> &str) {
			rope tmp(str);
			swap(*this, tmp);
			return *this;
		}

		//	assign
		//	http://en.cppreference.com/w/cpp/string/basic_string/assign

		//	(1)
		rope& assign(
			size_type count,
			OctetT ch
		) {
			rope tmp(count, ch, mode::octet, allocator);
			swap(*this, tmp);
			return *this;
		}

		//	(2)
		rope& assign(
			const rope &rp
		) {
			rope tmp(rp);
			swap(*this, tmp);
			return *this;
		}

		//	(3)
		rope& assign(
			const rope &rp,
			size_type pos,
			size_type count = npos
		) {
			rope tmp(rp, pos, count, mode::octet, allocator);
			swap(*this, tmp);
			return *this;
		}

		//	(4)
		rope& assign(
			rope &&rp
		) {
			swap(*this, rp);
			return *this;
		}

		//	(5)
		rope& assign(
			const OctetT *s,
			size_type count
		) {
			rope tmp(s, count, mode::octet, allocator);
			swap(*this, tmp);
			return *this;
		}

		//	(6)
		rope& assign(
			const OctetT *s
		) {
			rope tmp(s, allocator);
			swap(*this, tmp);
			return *this;
		}

		//	(7)
		template <class InputIt>
		rope& assign(
			InputIt first,
			InputIt last
		) {
			rope tmp(first, last, mode::octet, allocator);
			swap(*this, tmp);
			return *this;
		}

		//	(8)
		template <typename GeneralT>
		rope& assign(
			std::initializer_list<GeneralT> ilist
		) {
			rope tmp(ilist, mode::octet, allocator);
			swap(*this, tmp);
			return *this;
		}

		template <typename GeneralT>
		rope& assign(
			const std::basic_string<GeneralT> &str
		) {
			rope tmp(str, mode::octet, allocator);
			swap(*this, tmp);
			return *this;
		}

		//	finish the rest of assign_cp

		//	(1)
		rope& assign_cp(
			size_type count,
			CodepointT ch
		) {
			rope tmp(count, ch, mode::cp, allocator);
			swap(*this, tmp);
			return *this;
		}

		//	(2)
		rope& assign_cp(
			const rope &rp
		) {
			rope tmp(rp);
			swap(*this, tmp);
			return *this;
		}

		//	(3)
		rope& assign_cp(
			const rope &rp,
			size_type pos,
			size_type count = npos
		) {
			rope tmp(rp, pos, count, mode::cp, allocator);
			swap(*this, tmp);
			return *this;
		}

		//	(4)
		rope& assign_cp(
			rope &&rp
		) {
			swap(*this, rp);
			return *this;
		}

		//	(5)
		rope& assign_cp(
			const CodepointT *s,
			size_type count
		) {
			rope tmp(s, count, mode::cp, allocator);
			swap(*this, tmp);
			return *this;
		}

		//	(6)
		rope& assign_cp(
			const CodepointT *s
		) {
			rope tmp(s, mode::cp, allocator);
			swap(*this, tmp);
			return *this;
		}

		//	(7)
		template <class InputIt>
		rope& assign_cp(
			InputIt first,
			InputIt last
		) {
			rope tmp(first, last, mode::cp, allocator);
			swap(*this, tmp);
			return *this;
		}

		//	(8)
		template <typename GeneralT>
		rope& assign_cp(
			std::initializer_list<GeneralT> ilist
		) {
			rope tmp(ilist, mode::cp, allocator);
			swap(*this, tmp);
			return *this;
		}

		template <typename GeneralT>
		rope& assign_cp(
			const std::basic_string<GeneralT> &str
		) {
			rope tmp(str, mode::cp, allocator);
			swap(*this, tmp);
			return *this;
		}

		/**
		 *	\name get_allocator
		 *	@{
		 *	\fn allocator_type utf8::rope::get_allocator() const
		 *	Returns the allocator associated with the rope. Corresponds to [`std::basic_string::get_allocator`](http://en.cppreference.com/w/cpp/string/basic_string/get_allocator).
		 *	@}
		 */
		allocator_type get_allocator() const {
			return allocator;
		}

		/**
		 *	\name at
		 *	@{
		 *	\fn octet_reference utf8::rope::at(const size_type &pos)
		 *	Returns a reference to the octet at specified location `pos`. Bounds checking is performed, exception of type `std::out_of_range` will be thrown on invalid access. Corresponds to [`std::basic_string::at`](http://en.cppreference.com/w/cpp/string/basic_string/at).
		 *	\param		pos		Position of the octet to return.
		 *	\return		A reference to the requested octet.
	 	 *	\exception	out_of_range	Thrown if `pos >= size()`.
		 *	\warning	Writing directly to the reference returned may invalidate the rope.
		 */
		octet_reference at(const size_type &pos) {
			if (pos >= size_) {
				throw std::out_of_range("rope::at");
			}
			return index_octet(root, pos);
		}

		/**
		 *	\fn const_octet_reference utf8::rope::at(const size_type &pos) const
		 *	Returns a `const` reference to the octet at specified location `pos`. Bounds checking is performed, exception of type `std::out_of_range` will be thrown on invalid access. Corresponds to [`std::basic_string::at`](http://en.cppreference.com/w/cpp/string/basic_string/at).
		 *	\param		pos		Position of the octet to return.
		 *	\return		A `const` reference to the requested octet.
	 	 *	\exception	out_of_range	Thrown if `pos >= size()`.
		 */
		const_octet_reference at(const size_type &pos) const {
			if (pos >= size_) {
				throw std::out_of_range("rope::at");
			}
			return index_octet(root, pos);
		}

		/**
		 *	\fn CodepointT utf8::rope::at_cp(const size_type &pos) const
		 *	Returns the codepoint at specified location `pos`. Bounds checking is performed, exception of type `std::out_of_range` will be thrown on invalid access. Corresponds to [`std::basic_string::at`](http://en.cppreference.com/w/cpp/string/basic_string/at).
		 *	\param		pos		Position of the codepoint to return.
		 *	\return		A `const` reference to the requested codepoint.
	 	 *	\exception	out_of_range	Thrown if `pos >= size()`.
		 *	@}
		 */
		CodepointT at_cp(const size_type &pos) const {
			if (pos >= size_cp_) {
				throw std::out_of_range("rope::at_cp");
			}
			return index_cp(root, pos);
		}

		//	also need cp_at, cp_at_cp?

		//	operator[]
		//	http://en.cppreference.com/w/cpp/string/basic_string/operator_at

		//	WARNING: writing to the return directly may invalidate the rope
		octet_reference operator[](const size_type &pos) {
			return index_octet(root, pos);
		}

		const_octet_reference operator[](const size_type &pos) const {
			return index_octet(root, pos);
		}

		//	front
		//	http://en.cppreference.com/w/cpp/string/basic_string/front

		//	WARNING: writing to the return directly may invalidate the rope
		OctetT& front() {
			return operator[](0);
		}

		const OctetT& front() const {
			return operator[](0);
		}

		CodepointT front_cp() const {
			return index_cp(root, 0);
		}

		//	back
		//	http://en.cppreference.com/w/cpp/string/basic_string/back

		//	WARNING: writing to the return directly may invalidate the rope
		OctetT& back() {
			return operator[](size_ - 1);
		}

		const OctetT& back() const {
			return operator[](size_- 1);
		}

		CodepointT back_cp() const {
			return index_cp(root, size_cp_ - 1);
		}

		//	data
		//	http://en.cppreference.com/w/cpp/string/basic_string/data

		std::shared_ptr<OctetT> data() const noexcept {
			return c_str();
		}

		std::shared_ptr<CodepointT> data_cp() const noexcept {
			return c_str_cp();
		}

		//	c_str
		//	http://en.cppreference.com/w/cpp/string/basic_string/c_str

		std::shared_ptr<OctetT> c_str() const noexcept {
			return string_repr(root);
		}

		std::shared_ptr<CodepointT> c_str_cp() const noexcept {
			return string_repr_cp(root);
		}

		//	iterator hodgepodge
		//	begin
		//	cbegin
		//	end
		//	cend
		//	rbegin
		//	crbegin
		//	rend
		//	crend
		//	begin_cp
		//	cbegin_cp
		//	end_cp
		//	cend_cp
		//	rbegin_cp
		//	crbegin_cp
		//	rend_cp
		//	crend_cp

		//	empty
		//	http://en.cppreference.com/w/cpp/string/basic_string/empty
		bool empty() const noexcept {
			return size_ == 0;
		}

		//	size
		//	http://en.cppreference.com/w/cpp/string/basic_string/size
		size_type size() const noexcept {
			return size_;
		}

		size_type size_cp() const noexcept {
			return size_cp_;
		}

		//	length
		//	http://en.cppreference.com/w/cpp/string/basic_string/length
		size_type length() const noexcept {
			return size();
		}

		size_type length_cp() const noexcept {
			return size_cp();
		}

		//	max_size
		//	http://en.cppreference.com/w/cpp/string/basic_string/max_size
		//	i don't know how to implement this

		//	reserve
		//	http://en.cppreference.com/w/cpp/string/basic_string/reserve
		//	do not care at this point, make rope work first

		//	capacity
		//	http://en.cppreference.com/w/cpp/string/basic_string/capacity
		//	= size for now
		size_type capacity() const noexcept {
			return size();
		}

		//	shrink_to_fit
		//	http://en.cppreference.com/w/cpp/string/basic_string/shrink_to_fit
		//	do not care at this point, make rope work first

		//	clear
		//	http://en.cppreference.com/w/cpp/string/basic_string/clear
		void clear() noexcept {
			root.reset();
			root = make_node();
			size_ = 0;
			size_cp_ = 0;
		}

		//	insert
		//	http://en.cppreference.com/w/cpp/string/basic_string/insert

		//	(1) count copies of octet ch at octet position index
		rope& insert(
			size_type index,
			size_type count,
			OctetT ch
		) {
			if (count == 0) {
				return *this;
			}

			OctetT *ptr = allocator.allocate(count);
			for (size_t it = 0; it < count; ++it) {
				*(ptr + it) = ch;
			}
			std::shared_ptr<rope_node> inserted = make_leaf(ptr, count);

			if (index > size()) {
				throw std::out_of_range("rope::insert");
			}

			insert_node_octet(index, inserted);

			return *this;
		}

		//	(2) null-terminated octet string s at octet position index
		template<typename GeneralT>
		rope& insert(
			size_type index,
			GeneralT *s
		) {
			return insert(index, s, std::char_traits<GeneralT>::length(s));
		}

		//	(3) first count octets from the octet string s at octet position index
		template<typename GeneralT>
		rope& insert(
			size_type index,
			GeneralT *s,
			size_type count
		) {
			if (count == 0) {
				return *this;
			}

			OctetT *ptr = allocator.allocate(count);
			for (size_t it = 0; it < count; ++it) {
				*(ptr + it) = *(s + it);
			}
			std::shared_ptr<rope_node> inserted = make_leaf(ptr, count);

			if (index > size()) {
				throw std::out_of_range("rope::insert");
			}

			insert_node_octet(index, inserted);

			return *this;
		}

		//	(4) string str at octet position index
		template<typename GeneralT>
		rope& insert(
			size_type index,
			const std::basic_string<GeneralT> &str
		) {
			size_t count = str.size();

			if (count == 0) {
				return *this;
			}

			typename std::add_const<GeneralT>::type *src = str.data();
			OctetT *ptr = allocator.allocate(count);
			for (size_t it = 0; it < count; ++it) {
				*(ptr + it) = static_cast<OctetT>(*(src + it));
			}
			std::shared_ptr<rope_node> inserted = make_leaf(ptr, count);

			if (index > size()) {
				throw std::out_of_range("rope::insert");
			}

			insert_node_octet(index, inserted);

			return *this;
		}

		//	(4) rope rp at octet position index

	private:

		Allocator allocator;
		std::shared_ptr<rope_node> root;
		size_type size_, size_cp_;

		explicit rope(
			const Allocator &alloc,
			const std::shared_ptr<rope_node> &p_root,
			const size_type &p_size,
			const size_type &p_size_cp
		) : allocator(alloc), root(p_root), size_(p_size), size_cp_(p_size_cp) {}

		friend void swap(rope &i, rope &j) {
			using std::swap;
			swap(i.root, j.root);
			swap(i.allocator, j.allocator);
		}

		size_type calc_len_cp(
			OctetT *seq,
			size_type len_octet
		) {
			size_type len_cp = 0;
			size_t i = 0;
			while (i < len_octet) {
				size_t size;
				std::tie(std::ignore, size) = core::get_cp_and_size_from_utf8(seq + i);

				i += size;
				++len_cp;
			}

			return len_cp;
		}

		std::pair<size_type, size_type> calc_weight(
			std::shared_ptr<rope_node> node
		) {
			if (node->ptr) {
				node->weight_octet = node->len_octet;
				node->weight_cp = node->len_cp;
				return std::make_pair(node->weight_octet, node->weight_cp);
			}
			else {
				std::tie(node->weight_octet, node->weight_cp) = calc_weight(node->left);

				size_type right_weight_octet, right_weight_cp;
				std::tie(right_weight_octet, right_weight_cp) = calc_weight(node->right);

				return std::make_pair(node->weight_octet + right_weight_octet, node->weight_cp + right_weight_cp);
			}
		};

		std::shared_ptr<rope_node> copy_node(
			std::shared_ptr<rope_node> node
		) {
			if (!node) {
				return nullptr;
			}

			std::shared_ptr<rope_node> res = make_node();

			res->len_octet = node->len_octet;
			res->len_cp = node->len_cp;
			res->weight_octet = node->weight_octet;
			res->weight_cp = node->weight_cp;

			if (node->ptr) {
				res->ptr = allocator.allocate(res->len_octet);
				std::copy(node->ptr, node->ptr + node->len_octet, res->ptr);
			}
			else {
				res->left = copy_node(node->left);
				res->right = copy_node(node->right);
			}

			return res;
		}

		std::pair<std::shared_ptr<rope_node>, size_type> index_node_octet(
			const std::shared_ptr<rope_node> &node,
			size_type pos
		) const {
			if (node->weight_octet <= pos) {
				return index_node_octet(node->right, pos - node->weight_octet);
			}

			if (node->left) {
				return index_node_octet(node->left, pos);
			}

			return std::make_pair(node, pos);
		}

		octet_reference index_octet(
			std::shared_ptr<rope_node> node,
			size_type pos
		) {
			std::shared_ptr<rope_node> target_node;
			size_type offset;
			std::tie(target_node, offset) = index_node_octet(node, pos);

			return *(target_node->ptr + offset);
		}

		const_octet_reference index_octet(
			std::shared_ptr<rope_node> node,
			size_type pos
		) const {
			std::shared_ptr<rope_node> target_node;
			size_type offset;
			std::tie(target_node, offset) = index_node_octet(node, pos);

			return *(target_node->ptr + offset);
		}

		std::pair<std::shared_ptr<rope_node>, size_type> index_node_cp(
			const std::shared_ptr<rope_node> &node,
			size_type pos
		) const {
			if (node->weight_cp <= pos) {
				return index_node_cp(node->right, pos - node->weight_cp);
			}

			if (node->left) {
				return index_node_cp(node->left, pos);
			}

			size_t octet_pos = 0, cp_size = 0;
			for (size_t cp_i = 0; cp_i < pos; ++cp_i) {
				std::tie(std::ignore, cp_size) = core::get_cp_and_size_from_utf8(node->ptr + octet_pos);
				octet_pos += cp_size;
			}

			return std::make_pair(node, octet_pos);
		}

		CodepointT index_cp(
			std::shared_ptr<rope_node> node,
			size_type pos
		) const {
			std::shared_ptr<rope_node> target_node;
			size_type offset;
			std::tie(target_node, offset) = index_node_cp(node, pos);

			CodepointT cp;
			std::tie(cp, std::ignore) = core::get_cp_and_size_from_utf8(target_node->ptr + offset);

			return cp;
		}

		void write_string_repr(
			const std::shared_ptr<rope_node> &node,
			const std::shared_ptr<OctetT> &octets_repr,
			size_type pos
		) const {
			if (node->ptr) {
				std::copy(node->ptr, node->ptr + node->len_octet, octets_repr.get() + pos);
				return;
			}

			if (node->left) {
				write_string_repr(node->left, octets_repr, pos);
			}

			if (node->right) {
				write_string_repr(node->right, octets_repr, pos + (node->left ? node->left->len_octet : 0));
			}
		}

		void write_string_repr_cp(
			const std::shared_ptr<rope_node> &node,
			const std::shared_ptr<CodepointT> &cps_repr,
			size_type pos
		) const {
			if (node->ptr) {
				OctetT *ptr = node->ptr;
				while (ptr < node->ptr + node->len_octet) {
					CodepointT cp;
					size_t size;

					std::tie(cp, size) = core::get_cp_and_size_from_utf8(ptr);
					cps_repr.get()[pos] = cp;
					ptr += size;
				}
				return;
			}

			if (node->left) {
				write_string_repr_cp(node->left, cps_repr, pos);
			}

			if (node->right) {
				write_string_repr_cp(node->right, cps_repr, pos + (node->left ? node->left->len_cp : 0));
			}
		}

		std::shared_ptr<OctetT> string_repr(
			const std::shared_ptr<rope_node> &node
		) const {
			//	https://stackoverflow.com/a/44950534
			std::shared_ptr<OctetT> str(new OctetT[node->len_octet], std::default_delete<OctetT[]>());
			write_string_repr(node, str, 0);
			return str;
		}

		std::shared_ptr<CodepointT> string_repr_cp(
			const std::shared_ptr<rope_node> &node
		) const {
			//	https://stackoverflow.com/a/44950534
			std::shared_ptr<CodepointT> str(new CodepointT[node->len_cp], std::default_delete<CodepointT[]>());
			write_string_repr_cp(node, str, 0);
			return str;
		}

		std::shared_ptr<rope_node> concat_node(
			const std::shared_ptr<rope_node> &new_left,
			const std::shared_ptr<rope_node> &new_right
		) {
			std::shared_ptr<rope_node> res = std::make_shared<rope_node>();

			res->left = new_left;
			res->right = new_right;
			calc_weight(res);

			return res;
		}

		std::pair<std::shared_ptr<rope_node>, std::shared_ptr<rope_node>> split_node_octet(
			const std::shared_ptr<rope_node> &node,
			size_type pos
		) {
			if (node->weight_octet <= pos) {
				return split_node_octet(node->right, pos - node->weight_octet);
			}

			if (node->left) {
				std::shared_ptr<rope_node> new_right;
				std::tie(std::ignore, new_right) = split_node_octet(node->left, pos);

				std::shared_ptr<rope_node> new_tree = concat_node(new_right, node->right);
				node->right = nullptr;

				return std::make_pair(node, new_tree);
			}

			if (pos == 0) {
				return std::make_pair(nullptr, node);
			}
			else {
				OctetT *new_left_ptr = allocator.allocate(pos);
				std::copy(node->ptr, node->ptr + pos, new_left_ptr);

				size_t new_right_size = node->len_octet - pos;
				OctetT *new_right_ptr = allocator.allocate(new_right_size);
				std::copy(node->ptr + pos, node->ptr + node->len_octet, new_right_ptr);

				return std::make_pair(
					make_leaf(new_left_ptr, pos),
					make_leaf(new_right_ptr, new_right_size)
				);
			}
		}

		std::pair<std::shared_ptr<rope_node>, std::shared_ptr<rope_node>> split_node_cp(
			const std::shared_ptr<rope_node> &node,
			size_type pos
		) {
			if (node->weight_cp <= pos) {
				std::shared_ptr<rope_node> new_left, new_right;
				std::tie(new_left, new_right) = split_node_cp(node->right, pos - node->weight_cp);
				node->left = new_left;
				return std::make_pair(node, new_right);
			}

			if (node->left) {
				std::shared_ptr<rope_node> new_left, new_right;
				std::tie(new_left, new_right) = split_node_cp(node->left, pos);
				node->left = new_left;

				std::shared_ptr<rope_node> new_tree = concat_node(new_right, node->right);
				node->right = nullptr;

				return std::make_pair(node, new_tree);
			}

			if (pos == 0) {
				return std::make_pair(nullptr, node);
			}
			else {
				size_t octet_pos = 0, tmp;
				for (size_t i = 0; i < pos; ++i) {
					std::tie(std::ignore, tmp) = core::get_cp_and_size_from_utf8(node->ptr);
					octet_pos += tmp;
				}

				OctetT *new_left_ptr = allocator.allocate(octet_pos);
				std::copy(node->ptr, node->ptr + octet_pos, new_left_ptr);

				size_t new_right_size = node->len_octet - octet_pos;
				OctetT *new_right_ptr = allocator.allocate(new_right_size);
				std::copy(node->ptr + octet_pos, node->ptr + node->len_octet, new_right_ptr);

				return std::make_pair(
					make_leaf(new_left_ptr, octet_pos),
					make_leaf(new_right_ptr, new_right_size)
				);
			}
		}

		//	balance tree?

		//	make sure that node is not nullptr
		std::tuple<std::shared_ptr<rope_node>, size_type, size_type> insert_before_node(
			const std::shared_ptr<rope_node> &node,
			const std::shared_ptr<rope_node> &what
		) {
			return make_branch(what, node);
		}

		//	make sure that node is not nullptr
		std::tuple<std::shared_ptr<rope_node>, size_type, size_type> insert_after_node(
			const std::shared_ptr<rope_node> &node,
			const std::shared_ptr<rope_node> &what
		) {
			return make_branch(node, what);
		}

		//	recalculates weights as well
		void insert_node_octet(
			size_type index,
			const std::shared_ptr<rope_node> &what
		) {
			if (index == 0) {
				if (size_ == 0) {
					root = what;
				}
				else {
					std::tie(root, size_, size_cp_) = insert_before_node(root, what);
				}
			}
			else if (index == size()) {
				std::tie(root, size_, size_cp_) = insert_after_node(root, what);
			}
			else {
				std::shared_ptr<rope_node> left, right, tmp;
				std::tie(left, right) = split_node_octet(root, index);
				calc_weight(left);
				calc_weight(right);

				std::tie(tmp, size_, size_cp_) = insert_after_node(left, what);
				calc_weight(tmp);

				std::tie(root, size_, size_cp_) = insert_after_node(tmp, right);
			}

			std::tie(size_, size_cp_) = calc_weight(root);
		}

	};

	//	template specializations for CodepointT

	template <>
	rope& rope::operator=(const CodepointT *s) {
		rope tmp(s, mode::cp);
		swap(*this, tmp);
		return *this;
	}

	template <>
	rope& rope::operator=(CodepointT ch) {
		rope tmp(1, ch, mode::cp);
		swap(*this, tmp);
		return *this;
	}

	template <>
	rope& rope::operator=(std::initializer_list<CodepointT> ilist) {
		rope tmp(ilist.begin(), ilist.end(), mode::cp);
		swap(*this, tmp);
		return *this;
	}

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
