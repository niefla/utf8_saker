# rope design

This document specifies what functions implemented in `utf8::rope` corresponds to the functions implemented for `std::basic_string`, as `utf8::rope` is designed to mirror `std::basic_string` as much as possible.

One persistent design issue is how to let the user specify octet / codepoint behavior. These are the rule of thumbs:

*	For functions with names, the functions with the `_cp` suffix perform codepoint behavior, and those without perform octet behavior just like their `std::basic_string` counterparts.
*	For constructors, octet / codepoint behavior is specified via a optional parameter `m` of type `const mode&` whose default value is `mode::octet`.
*	For operators, codepoint behavior is used if template substitution for codepoint types succeed. Otherwise, SFINAE and the function falls back to the default behavior which is octet behavior.

## [constructor](http://en.cppreference.com/w/cpp/string/basic_string/basic_string)

| `std::basic_string` | `utf8::rope` | Description |
| :---: | :---: | :---: |
| (1) `explicit basic_string(const Allocator &alloc = Allocator());` | `explicit rope(const Allocator &alloc = Allocator());` | Initializes `this->alloc` to `alloc`, `root` to `std::make_shared<rope_node>()` |
| (2) `basic_string(size_type count, CharT ch, const Allocator& alloc = Allocator());` | `template <typename GeneralT> rope(size_type count, GeneralT ch, const mode &m = mode::octet, const Allocator &alloc = Allocator());` | Template catches all `GeneralT`; specify octet / codepoint behavior (copy `ch` as octet / codepoint) via `m` |
| (3) `basic_string(const basic_string& other, size_type pos, size_type count = npos, const Allocator& alloc = Allocator());` | `rope(const rope &other, size_type pos, size_type count = npos, const mode &m = mode::octet, const Allocator &alloc = Allocator());` | Specify octet / codepoint behavior (`count`-many octets / codepoints) via `m` |
| (4) `basic_string(const CharT* s, size_type count, const Allocator& alloc = Allocator());` | `template <typename GeneralT> rope(const GeneralT *s, size_type count, const mode &m = mode::octet, const Allocator &alloc = Allocator());` | Template catches all `const GeneralT*`; specify octet / codepoint behavior (`count`-many octets / codepoints) via `m` |
| (5) `basic_string(const CharT* s, const Allocator& alloc = Allocator());` | `template <typename GeneralT> rope(const GeneralT *s, const mode &m = mode::octet, const Allocator &alloc = Allocator());` | Template catches all `const CharT*`, specify octet / codepoint behavior (characters in `s` are copied as octets / codepoints) via `m` |
| (6) `template <class InputIt> basic_string(InputIt first, InputIt last, const Allocator& alloc = Allocator());` | `template <class InputIt> rope(InputIt first, InputIt last, const mode &m = mode::octet, const Allocator &alloc = Allocator());` | Template catches all `InputIt`; specify octet / codepoint behavior (characters between `first` and `last` are copied as octets / codepoints) |
| (7) `basic_string(const basic_string& other, const Allocator& alloc);` | `rope(const rope &other, const Allocator &alloc = Allocator());` | Copy constructor; performs deep copy |
| (8) `basic_string(basic_string&& other, const Allocator& alloc);` | `rope(rope &&other, const Allocator &alloc = Allocator())` | Move constructor; performs shallow copy |
| (9) `basic_string(std::initializer_list<CharT> init, const Allocator& alloc = Allocator());` | `template <typename GeneralT> rope(std::initializer_list<GeneralT> init, const mode &m = mode::octet, const Allocator &alloc = Allocator())` | Template catches all `std::initializer_list<GeneralT>`; specify octet / codepoint behavior (characters in `init` are copied as octets / codepoints) via `m` |

## [`operator=`](http://en.cppreference.com/w/cpp/string/basic_string/operator%3D)

| `std::basic_string` | `utf8::rope` | Description |
| :---: | :---: | :---: |
| (1) `basic_string& operator=(const basic_string& str);` | `rope& operator=(rope other);` | Copy assignment operator |
| (2) `basic_string& operator=(basic_string&& str);` | `rope& operator=(rope &&other);` | Move assignment operator |
| (3) `basic_string& operator=(const CharT* s);` | `template <typename GeneralT> rope& operator=(const GeneralT *s);` | Template catches all `const GeneralT*`; codepoint specialization and octet fallback |
| (4) `basic_string& operator=(CharT ch);` | `template <typename GeneralT> rope& operator=(GeneralT ch);` | Template catches all `GeneralT`; codepoint specialization and octet fallback |
| (5) `basic_string& operator=(std::initializer_list<CharT> ilist);` | `template <typename GeneralT> rope& operator=(std::initializer_list<GeneralT> ilist);` | Template catches all `std::initializer_list<GeneralT>`; codepoint specialization and octet fallback |

## [`assign`](http://en.cppreference.com/w/cpp/string/basic_string/assign)

| `std::basic_string` | `utf8::rope` | Description |
| :---: | :---: | :---: |
