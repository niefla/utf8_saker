# impl

Notes for implementation

## `utf8::rope`

Probably cannot satisfy `SequenceContainer`

### `SequenceContainer`

*	`Container`
	*	Given container type `C`, element type `T`, objects `a`, `b` of type `C`, the following types are defined:

### `AllocatorAwareContainer`

## `utf8::rope::iterator`

### `RandomAccessIterator`

`utf8_string` should have the member types `iterator` and `const_iterator` both of which must be `RandomAccessIterator`s. They are defined as classes.

The requirements listed below are taken from [cppreference](https://en.cppreference.com/w/cpp/concept/RandomAccessIterator).

The current implementation in mind does not satisfy `OutputIterator`.

*	`MoveConstructible`
	*	`It` has move constructor or copy constructor that takes `const T&` ("move-from" value is unspecified)
*	`CopyConstructible`
	*	`It` is `MoveConstructible`
	*	`It` has copy constructor that does not change the "copy-from" value
*	`MoveAssignable`
	*	`It` has move assignment operator or copy assignment operator that takes `const T&` ("move-from" value is unspecified)
*	`CopyAssignable`
	*	`It` is `MoveAssignable`
	*	`It` has copy assignment operator that does not change the "copy-from" value
*	`Destructible`
	*	`It` has a destructor that makes all resources reclaimed and throws no exceptions
*	`Swappable`
	*	`It` has a `swap` function (non-member that optionally calls member `swap` / friend)
*	`Iterator`
	*	`It` is `CopyConstructible`
	*	`It` is `CopyAssignable`
	*	`It` is `Destructible`
	*	lvalue `It&` is `Swappable`
	*	`std::iterator_traits<It>` has member typedefs `value_type`, `difference_type`, `reference`, `pointer` and `iterator_category`
	*	Given `r` of type `It`
		*	`*r` is supported given `r` is dereferenceable
		*	`++r` is supported given `r` is incrementable, returns `It&`
	*	an `Iterator` is **not** dereferenceable if it is past-the-end or before-begin, not associated with any sequence, or invalidated by sequence operations
*	`EqualityComparable`
	*	has `operator==` and is an equivalence relation
*	`InputIterator`
	*	`It` is `Iterator`
	*	`It` is `EqualityComparable`
	*	Given `i`, `j` of type `It` or `const It`, the type `reference` denoted by `std::iterator_traits<It>::reference`, and the type `value_type` denoted by `std::iterator_traits<It>::value_type`,
	*	`i != j` is equivalent to `!(i == j)`
	*	`*i` has type `reference`, convertible to `value_type`
	*	`i->m` is equivalent to `(*i).m`
	*	`++i` returns `It&`
	*	`(void)i++` is equivalent to `(void)++i`
	*	`*i++` is convertible to `value_type`
*	`DefaultConstructible`
	*	a public default constructor must be defined
*	`ForwardIterator`
	*	`It` is `InputIterator`
	*	`It` is `DefaultConstructible`
	*	multipass guarantee: given `a` and `b`, dereferenceable `It`s
		*	if `a == b` then either they are both non-dereferenceable or `*a` and `*b` are references bound to the same object
		*	assignment through a mutable `ForwardIterator` cannot invalidate the iterator
		*	incrementing a copy of `a` does not change the value read from `a`
		*	if `a == b` then `++a == ++b`
	*	given `T` the type denoted by `std::iterator_traits<It>::value_type`, `std::iterator_traits<It>:reference` must be exactly
		*	`T&` if `It` is `OutputIterator` (`It` is mutable)
		*	`const T&` otherwise (`It` is constant)
	*	(in)equality comparison is defined over all iterators for the same underlying sequence
	*	given `i` a referenceable iterator of type `It` and the type `reference`, denoted by `std::iterator_traits<It>::reference`,
		*	⚠ `i++` has type `It` and is equivalent to `It ip=i; ++i; return ip;`
		*	`*i++` has type `reference`
*	`BidirectionalIterator`
	*	`It` is `ForwardIterator`
	*	given `a` and `b` both of type `It` and `reference` denoted by `std::iterator_traits<It>::reference`
		*	`--a` returns `It&` if `a` is decrementale (there is `b` such that `a == ++b`). postconditions are: `a` is dereferenceable, `--(++a) == a`, if `--a == --b` then `a == b`, `&a == &--a` (address of iterator does not change after decrement)
		*	⚠ `a--` returns a type convertible to `const T&`, equivalent to `It temp = a; --a; return temp;`
		*	`*a--` has type `reference`
	*	the begin iterator is not decrementable
*	`RandomAccessIterator`
	*	`It` is `BidirectionalIterator`
	*	given `value_type`, `reference_type`, `reference` denoted in `std::iterator_traits<It>::...`, `i`, `a`, `b` of type `It` or `const It`, `r` of type `It&`, `n` of type `difference_type`
		*	`r += n` has type `It&` and is equivalent to:
			```
			difference_type m = n;
			if (m >= 0) while (m--) ++r; else while (m++) --r;
			return r;
			```
			done in ⚠ **constant complexity**
		*	`a + n` and `n + a` have type `It` and is equivalent to
			```
			It temp = a;
			return temp += n;
			```
		*	`r -= n` has type `It&` and is equivalent to `return r += -n;`
		*	`i - n` has type `It` and is equivalent to
			```
			It temp = i;
			return temp -= n;
			```
		*	`b - a` has type `difference_type` and is equivalent to `return n;` given there exists an `n` such that `a + n == b`
		*	`i[n]` has type convertible to `reference` and is equivalent to `*(i + n)`
		*	`a < b` has type convertible to `bool` and is equivalent to `b - a > 0` and satisfies the total ordering relation:
			*	`!(a < a)`
			*	if `a < b` then `!(b < a)`
			*	if `a < b` and `b < c` then `a < c`
			*	`a < b` or `b < a` or `a == b`
		*	similarly, `a > b` is equivalent to `b < a`, `a >= b` to `!(a < b)`, `a <= b` to `!(a > b)`
		*	this implies `LessThanComparable`
