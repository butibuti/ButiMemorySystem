#ifndef BUTILIST_H
#define BUTILIST_H
#include"ButiUtil/ButiUtil/TypeDetail.h"
#include"ButiUtil/ButiUtil/Exception.h"
#include<cassert>
#include"MemoryAllocator.h"
namespace ButiEngine {

template <typename T, typename U>inline constexpr T max_(T a, U b) { return  ((a) > (b)) ? (a) : (b); }
struct has_eq_operator_impl {
	template <typename T>	static auto check(T&& x)->decltype(x == x, std::true_type{});

	template <typename T>	static auto check(...)->std::false_type;
}; 
struct has_neq_operator_impl {
	template <typename T>	static auto check(T&& x)->decltype(x != x, std::true_type{});

	template <typename T>	static auto check(...)->std::false_type;
};
template <typename T>class has_eq_operator : public decltype(has_eq_operator_impl::check<T>(std::declval<T>())) {};
template <typename T>class has_neq_operator : public decltype(has_neq_operator_impl::check<T>(std::declval<T>())) {};

template <typename T>constexpr bool has_eq_operator_v = has_eq_operator<T>();
template <typename T>constexpr bool has_neq_operator_v = has_neq_operator<T>();

namespace ButiContainerDetail {

class IteratorHelper
{
public:

	// 一致する最初の要素を削除する
	template <typename TList, typename T>
	static bool Remove(TList& arg_list, const T& arg_item)
	{
		for (auto itr = arg_list.begin(), end = arg_list.end(); itr != end; ++itr)
		{
			if (*itr == arg_item) {
				arg_list.erase(itr);
				return true;
			}
		}
		return false;
	}

	// 等しい要素をすべて削除する
	template <typename TIterator, typename T>
	static TIterator RemoveAll(TIterator arg_begin, TIterator arg_end, const T& arg_value)
	{
		return std::remove(arg_begin, arg_end, arg_value);
	}

	// 該当する要素をすべて削除する
	template <typename TIterator, typename F>
	static TIterator RemoveAll_f(TIterator arg_begin, TIterator arg_end, F arg_function)
	{
		return std::remove_if(arg_begin, arg_end, arg_function);
	}
	template<typename ...Args>
	struct first_enabled {};
	template<typename T, typename ...Args>
	struct first_enabled<std::enable_if<true, T>, Args...> { using type = T; };
	template<typename T, typename ...Args>
	struct first_enabled<std::enable_if<false, T>, Args...> : first_enabled<Args...> {};
	template<typename T, typename ...Args>
	struct first_enabled<T, Args...> { using type = T; };

	template<typename ...Args>
	using first_enabled_t = typename first_enabled<Args...>::type;
};

template<class T>
class ContainerAllocator
{
public:


	using value_type = T;
	using const_value_type = const value_type;
	using pointer_type = value_type*;
	using void_pointer_type = void*;
	using const_pointer_type = const value_type*;
	using const_void_pointer_type = const void*;

	//STL対応
	using pointer = T*;
	using const_pointer = const T*;
	using size_type = std::size_t;
	template<class U>
	struct rebind
	{
		using other = ContainerAllocator<U>;
	};
	//

	ContainerAllocator() { }
	ContainerAllocator(const ContainerAllocator& arg_other) { }

	template<class U>
	ContainerAllocator(const ContainerAllocator<U>& arg_other) { }

	static pointer_type allocate(size_type arg_count, const_pointer_type hint = nullptr)
	{
		return reinterpret_cast<pointer_type>(ButiMemorySystem::Allocator::allocate(arg_count * static_cast<size_type>( sizeof(T))));
	}

	static void deallocate(pointer_type arg_ptr, size_type)
	{
		ButiMemorySystem::Allocator::deallocate(arg_ptr);
	}
	static void deallocate(void* arg_ptr, size_type)
	{
		ButiMemorySystem::Allocator::deallocate(arg_ptr);
	}
};

}
template<typename T>
class Iterator {
public:
	using value_type = T;
	using const_value_type = const value_type;
	using pointer_type = value_type*;
	using void_pointer_type = void*;
	using const_pointer_type = const value_type*;
	using const_void_pointer_type = const void*;
	using reference_type = value_type&;
	using const_reference_type = const value_type&;


	using this_type = Iterator;

	//std関数に対応するためのusing
	using iterator_category = std::random_access_iterator_tag;
	using difference_type = std::int32_t;
	using pointer = const pointer_type;
	using reference = const value_type&;
	//
	Iterator(pointer_type arg_ptr)noexcept :p_v(arg_ptr) {}
	Iterator(nullptr_t)noexcept :p_v(nullptr) {}
	Iterator(void_pointer_type arg_ptr)noexcept :p_v(reinterpret_cast<pointer_type>(arg_ptr)) {}
	Iterator(const this_type& arg_other) noexcept :p_v(arg_other.p_v) {}
	Iterator(this_type&& arg_other)noexcept :p_v(arg_other.p_v) {
		arg_other.p_v = nullptr;
	}

	this_type& operator=(const this_type& arg_other) noexcept {
		p_v = arg_other.p_v;
		return *this;
	}
	this_type& operator=(this_type&& arg_other) noexcept {
		p_v = arg_other.p_v;
		arg_other.p_v = nullptr;
		return *this;
	}

	reference_type operator*() const noexcept {
		return *p_v;
	}

	pointer_type operator->() const noexcept {
		return p_v;
	}
	const_reference_type operator[](const std::int32_t arg_index) const noexcept {
		return p_v[arg_index];
	}
	reference_type operator[](const std::int32_t arg_index) noexcept {
		return p_v[arg_index];
	}
	this_type& operator++() noexcept {
		p_v++;
		return *this;
	}
	this_type operator++(std::int32_t) noexcept {
		auto output = *this;
		p_v++;
		return output;
	}
	this_type& operator--() noexcept {
		p_v--;
		return *this;
	}
	this_type operator--(std::int32_t) noexcept {
		auto output = *this;
		p_v--;
		return output;
	}
	inline this_type& operator+=(size_t arg_step) noexcept {
		p_v += arg_step;
		return *this;
	}
	inline this_type operator+(size_t arg_step) const noexcept {
		auto output = *this;
		output.p_v += arg_step;
		return output;
	}
	inline this_type& operator-=(size_t arg_step) noexcept {
		p_v -= arg_step;
	}
	inline this_type operator-(size_t arg_step)const noexcept {
		auto output = *this;
		output.p_v -= arg_step;
		return output;
	}
	inline bool operator==(const this_type& arg_other) const noexcept {
		return p_v == arg_other.p_v;
	}
	inline bool operator!=(const this_type& arg_other) const noexcept {
		return !(*this == arg_other);
	}
	inline bool operator>(const this_type& arg_other) const noexcept {
		return p_v > arg_other.p_v;
	}
	inline bool operator<(const this_type& arg_other) const noexcept {
		return p_v < arg_other.p_v;
	}
	inline bool operator>=(const this_type& arg_other) const noexcept {
		return !(*this < arg_other);
	}
	inline bool operator<=(const this_type& arg_other) const noexcept {
		return !(*this > arg_other);
	}
	inline difference_type operator-(const this_type& arg_other)const noexcept {
		return static_cast<difference_type> (p_v - arg_other.p_v);
	}
	inline pointer_type Ptr()const noexcept { return p_v; }
	inline void _Seek_to(const_pointer_type arg_p_v) noexcept {
		p_v = (const_cast<pointer_type>(arg_p_v));
	}
	inline pointer_type _Unwrapped() const noexcept {
		return p_v;
	}
private:
	pointer_type p_v;
};


template<typename T>
class ReverseIterator {
public:
	using value_type = T;
	using const_value_type = const value_type;
	using pointer_type = value_type*;
	using void_pointer_type = void*;
	using const_pointer_type = const value_type*;
	using const_void_pointer_type = const void*;
	using reference_type = value_type&;
	using const_reference_type = const value_type&;


	using this_type = ReverseIterator;

	//std関数に対応するためのusing
	using iterator_category = std::random_access_iterator_tag;
	using difference_type = std::int32_t;
	using pointer = const pointer_type;
	using reference = const value_type&;
	//
	ReverseIterator(pointer_type arg_ptr)noexcept :p_v(arg_ptr-1) {}
	ReverseIterator(void_pointer_type arg_ptr)noexcept :p_v(reinterpret_cast<pointer_type>(arg_ptr)-1) {}
	ReverseIterator(const this_type& arg_other) noexcept :p_v(arg_other.p_v) {}
	ReverseIterator(this_type&& arg_other)noexcept :p_v(arg_other.p_v) {
		arg_other.p_v = nullptr;
	}

	this_type& operator=(const this_type& arg_other) noexcept {
		p_v = arg_other.p_v;
		return *this;
	}
	this_type& operator=(this_type&& arg_other) noexcept {
		p_v = arg_other.p_v;
		arg_other.p_v = nullptr;
		return *this;
	}

	reference_type operator*() const noexcept {
		return *p_v;
	}

	pointer_type operator->() const noexcept {
		return p_v;
	}
	const_reference_type operator[](const std::int32_t arg_index) const noexcept {
		return p_v[arg_index];
	}
	reference_type operator[](const std::int32_t arg_index) noexcept {
		return p_v[arg_index];
	}
	this_type& operator++() noexcept {
		p_v--;
		return *this;
	}
	this_type operator++(std::int32_t) noexcept {
		auto output = *this;
		p_v--;
		return output;
	}
	this_type& operator--() noexcept {
		p_v++;
		return *this;
	}
	this_type operator--(std::int32_t) noexcept {
		auto output = *this;
		p_v++;
		return output;
	}
	inline this_type& operator+=(size_t arg_step) noexcept {
		p_v += arg_step;
		return *this;
	}
	inline this_type operator+(size_t arg_step) const noexcept {
		auto output = *this;
		output.p_v -= arg_step;
		return output;
	}
	inline this_type& operator-=(size_t arg_step) noexcept {
		p_v += arg_step;
	}
	inline this_type operator-(size_t arg_step)const noexcept {
		auto output = *this;
		output.p_v += arg_step;
		return output;
	}
	inline bool operator==(const this_type& arg_other) const noexcept {
		return p_v == arg_other.p_v;
	}
	inline bool operator!=(const this_type& arg_other) const noexcept {
		return !(*this == arg_other);
	}
	inline bool operator>(const this_type& arg_other) const noexcept {
		return p_v < arg_other.p_v;
	}
	inline bool operator<(const this_type& arg_other) const noexcept {
		return p_v > arg_other.p_v;
	}
	inline bool operator>=(const this_type& arg_other) const noexcept {
		return !(*this < arg_other);
	}
	inline bool operator<=(const this_type& arg_other) const noexcept {
		return !(*this > arg_other);
	}
	inline difference_type operator-(const this_type& arg_other)const noexcept {
		return (arg_other.p_v-p_v);
	}
	inline pointer_type Ptr()const noexcept { return p_v; }
	inline void _Seek_to(const_pointer_type arg_p_v) noexcept {
		p_v = (const_cast<pointer_type>(arg_p_v));
	}
	inline pointer_type _Unwrapped() const noexcept {
		return p_v;
	}
private:
	pointer_type p_v;
};


template<typename T>
class List
{
public:
	using value_type = T;
	//using difference_type = std::vector<value_type, ButiContainerDetail::ContainerAllocator<value_type>>::difference_type;
	using const_value_type = const value_type;
	using pointer_type = value_type*;
	using void_pointer_type = void*;
	using const_pointer_type = const value_type*;
	using const_void_pointer_type = const void*;
	using reference_type = value_type&;
	using const_reference_type = const value_type&;
	using this_type = List;

	using iterator_type = Iterator<value_type>;
	using const_iterator_type = const Iterator<value_type>;
	using reverse_iterator_type = Iterator<value_type>;
	using const_reverse_iterator_type = const Iterator<value_type>;


	inline constexpr List()noexcept {}
	inline List(const List& arg_other) :List(arg_other.begin(), arg_other.end()) {}
	inline List(List&& arg_other) {
		p_data = arg_other.p_data;
		currentDataSize = arg_other.currentDataSize;
		currentCapacity = arg_other.currentCapacity;
		arg_other.p_data = nullptr;
		arg_other.currentDataSize = 0;
		arg_other.currentCapacity = 0;
	}
	inline List(std::initializer_list<value_type> arg_initializer) :List(arg_initializer.begin(), arg_initializer.end()) {}

	template <typename Iterator_T>
	inline List(Iterator_T arg_begin, Iterator_T arg_end) {
		if constexpr (ButiTypeDetail::has_subtract_v<Iterator_T>) {
			Reserve((arg_end - arg_begin) + currentCapacity);
		}
		for (auto itr = arg_begin; itr != arg_end; itr++) {
			Add(*itr);
		}
	}

	~List() {
		if constexpr (!std::is_trivially_destructible_v<value_type>) {
			for (auto itr = begin(), endItr = end(); itr < endItr; itr++) {
				if constexpr (!std::is_trivially_destructible_v<T>) {
					itr->~value_type();
				}
			}
		}
		dealloc();
	}
	inline bool operator==(const List<T>& arg_other)const noexcept {
		if constexpr (has_neq_operator_v<T>) {
			if (this->currentDataSize != arg_other.currentDataSize) {
				return false;
			}
			for (std::uint32_t index=0; index < this->currentDataSize; index++) {
				if ((reinterpret_cast<const T*>(this->p_data))[index] != (reinterpret_cast<const T*>(arg_other.p_data))[index]) {
					return false;
				}
			}
			return true;
		}
		return false;
	}
	inline bool IsEmpty() const noexcept
	{
		return !currentDataSize;
	}

	std::uint32_t GetSize()const noexcept
	{
		return currentDataSize;
	}

	std::int32_t GetCapacity()const noexcept
	{
		return currentCapacity;
	}
	inline void push_back(const_reference_type arg_item) {
		Add(arg_item);
	}

	void Add(const_reference_type arg_item)
	{
		Reserve(currentDataSize + 1);
		copy_construct(&reinterpret_cast<pointer_type>(p_data)[currentDataSize], arg_item);
		currentDataSize++;
	}


	void Add(const this_type& arg_items)
	{
		Reserve(currentDataSize + arg_items.GetSize());
		for (auto itr = arg_items.begin(), endItr = arg_items.end(); itr < endItr; itr++, currentDataSize++) {
			copy_construct(&reinterpret_cast<pointer_type>(p_data)[currentDataSize], *itr);
		}
	}

	void Add_noDuplicate(const this_type& arg_items)
	{
		Reserve(currentDataSize + arg_items.GetSize());
		for (auto itr = arg_items.begin(), endItr = arg_items.end(); itr < endItr; itr++) {
			if (Contains(*itr)) { continue; }
			copy_construct(&reinterpret_cast<pointer_type>(p_data)[currentDataSize], *itr);
			currentDataSize++;
		}
	}
	/// <summary>
	/// Tを引数にとる関数を渡して走査的に実行する
	/// 配列への破壊的な変更は禁止
	/// </summary>
	/// <typeparam name="F"></typeparam>
	/// <param name="arg_function">Tを引数にとる関数(ラムダ式、std::function、()オーバーロードしたクラスの実体など)</param>
	template<typename F>
	void ForEach(F arg_function) {
		for (auto itr = begin(), endItr = end(); itr < endItr; itr++) {
			arg_function(*itr);
		}
	}
	/// <summary>
	/// Tを引数にとる関数を渡して走査的に実行する
	/// 配列への破壊的な変更は禁止
	/// </summary>
	/// <typeparam name="F"></typeparam>
	/// <param name="arg_function">Tを引数にとる関数(ラムダ式、std::function、()オーバーロードしたクラスの実体など)</param>
	template<typename F>
	void ForEach(F arg_function)const {
		for (auto itr = begin(), endItr = end(); itr < endItr; itr++) {
			arg_function(*itr);
		}
	}

	inline void Insert(const std::int32_t arg_index, const_reference_type arg_item)
	{
		Insert(begin() + arg_index, arg_item);
	}
	inline void Insert(iterator_type arg_pos, const_reference_type arg_item)
	{
		if (currentDataSize + 1 > currentCapacity) {
			auto keepSizeAlignment = max_(currentCapacity, 1) * 2;
			for (; keepSizeAlignment < currentDataSize + 1; keepSizeAlignment *= 2);

			auto p_newData = ButiContainerDetail::ContainerAllocator<value_type>::allocate(keepSizeAlignment);
			auto newDataItr = p_newData;
			for (iterator_type itr = begin(), endItr = arg_pos; itr != endItr; itr++, newDataItr++) {
				copy_construct(newDataItr, *itr);
				if constexpr (!std::is_trivially_destructible_v<T>) {
					itr->~value_type();
				}
			}
			copy_construct(newDataItr, arg_item);
			newDataItr++;
			for (iterator_type itr = arg_pos, endItr = end(); itr != endItr; itr++, newDataItr++) {
				copy_construct(newDataItr, *itr); 
				if constexpr (!std::is_trivially_destructible_v<T>) {
					itr->~value_type();
				}
			}
			dealloc();
			p_data = p_newData;
			currentCapacity = keepSizeAlignment;
			currentDataSize += 1;
			return;
		}
		for (iterator_type itr = end() , endItr =arg_pos ; itr > endItr; itr--) {
			copy_construct(itr.Ptr(), *(itr - 1));
		}
		*arg_pos = arg_item;
		currentDataSize += 1;
	}

	inline void Insert(const_iterator_type arg_pos, const_iterator_type arg_begin, const_iterator_type arg_end) {
		std::int32_t insertLength = arg_end - arg_begin;
		if (currentDataSize + insertLength > currentCapacity) {
			auto keepSizeAlignment = max_(currentCapacity, 1) * 2;
			for (; keepSizeAlignment < currentDataSize + insertLength; keepSizeAlignment *= 2);

			auto p_newData = ButiContainerDetail::ContainerAllocator<value_type>::allocate(keepSizeAlignment);
			auto newDataItr = p_newData;
			for (iterator_type itr = begin(), endItr = arg_pos; itr != endItr; itr++, newDataItr++) {
				copy_construct(newDataItr, *itr); 
				if constexpr (!std::is_trivially_destructible_v<T>) {
					itr->~value_type();
				}
			}
			for (iterator_type itr = arg_begin; itr < arg_end; itr++, newDataItr++) {
				copy_construct(newDataItr, *itr);
			}
			for (iterator_type itr = arg_pos, endItr = end(); itr != endItr; itr++, newDataItr++) {
				copy_construct(newDataItr, *itr); 
				if constexpr (!std::is_trivially_destructible_v<T>) {
					itr->~value_type();
				}
			}
			dealloc();
			p_data = p_newData;
			currentCapacity = keepSizeAlignment;
			currentDataSize += insertLength;
			return;
		}

		for (iterator_type itr = arg_pos + insertLength, endItr = end() + insertLength; itr < endItr; itr++) {
			assign(itr.Ptr(), *(itr - insertLength));
		}
		for (iterator_type itr = arg_pos, endItr = arg_pos + insertLength, assignItr = arg_begin; itr < endItr; itr++, assignItr++) {
			assign(itr.Ptr(), *(assignItr));
		}
		currentDataSize += insertLength;
	}
	template<typename OtherIteratorType>
	inline void Insert(const_iterator_type arg_pos, OtherIteratorType arg_begin, OtherIteratorType arg_end) {
		std::int32_t insertLength = arg_end - arg_begin;
		if (currentDataSize + insertLength > currentCapacity) {
			auto keepSizeAlignment = max_(currentCapacity, 1) * 2;
			for (; keepSizeAlignment < currentDataSize + insertLength; keepSizeAlignment *= 2);

			auto p_newData = ButiContainerDetail::ContainerAllocator<value_type>::allocate(keepSizeAlignment);
			auto newDataItr = p_newData;
			for (auto itr = begin(), endItr = arg_pos; itr != endItr; itr++, newDataItr++) {
				copy_construct(newDataItr, *itr); if constexpr (!std::is_trivially_destructible_v<T>) {
					itr->~value_type();
				}
			}
			for (auto itr = arg_begin; itr != arg_end; itr++, newDataItr++) {
				copy_construct(newDataItr, *itr);
			}
			for (auto itr = arg_pos, endItr = end(); itr != endItr; itr++, newDataItr++) {
				copy_construct(newDataItr, *itr); if constexpr (!std::is_trivially_destructible_v<T>) {
					itr->~value_type();
				}
			}
			dealloc();
			p_data = p_newData;
			currentCapacity = keepSizeAlignment;
			currentDataSize += insertLength;
			return;
		}

		for (iterator_type itr = arg_pos + insertLength, endItr = end() + insertLength; itr != endItr; itr++) {
			assign(itr.Ptr(), *(itr - insertLength));
		}
		auto assignItr = arg_begin;
		for (iterator_type itr = arg_pos, endItr = arg_pos + insertLength; itr != endItr; itr++, assignItr++) {
			assign(itr.Ptr(), *(assignItr));
		}
		currentDataSize += insertLength;
	}


	inline void Clear()
	{
		for (auto itr = begin(), endItr = end(); itr < endItr; itr++) {
			if constexpr (!std::is_trivially_destructible_v<T>) {
				itr->~value_type();
			}
		}
		currentDataSize = 0;
	}

	inline bool Remove(const value_type& arg_item)
	{
		auto result = ButiContainerDetail::IteratorHelper::Remove(*this, arg_item);
		return result;
	}


	template<typename F>
	bool Remove(F arg_function)
	{
		for (auto itr = begin(), endItr = end(); itr != endItr; ++itr)
		{
			if (arg_function((*itr))) {
				erase(itr);
				return true;
			}
		}
		return false;
	}


	void RemoveAll(const value_type& arg_item)
	{
		auto endItr = end();
		auto removedEnd = ButiContainerDetail::IteratorHelper::RemoveAll(begin(), endItr, arg_item);
		auto diff = endItr - removedEnd;
		erase(removedEnd, endItr);
	}

	template<typename F>
	void RemoveAll(F arg_function)
	{
		auto endItr = end();
		auto removedEnd = ButiContainerDetail::IteratorHelper::RemoveAll_f(begin(), endItr, arg_function);
		auto diff = endItr - removedEnd;
		erase(removedEnd, endItr);
	}

	inline void RemoveAt(const std::int32_t arg_index)
	{
		if (IsOutOfRange(arg_index)) {
			return;
		}
		erase(begin() + arg_index);
	}
	void RemoveLast()
	{
		RemoveAt(GetSize() - 1);
	}

	inline void Resize(const std::int32_t arg_size)
	{
		if constexpr (std::is_default_constructible_v<T>) {
			if (arg_size <= 0 || arg_size == currentDataSize) { return; }
			if (arg_size > currentDataSize) {
				Reserve(arg_size);
				for (auto itr = end().Ptr(); currentDataSize < arg_size; currentDataSize++, itr++) {
					default_construct(itr);
				}
			}
			else {
				for (auto itr = (begin() + arg_size), endItr = end(); itr < endItr; itr++) {
					if constexpr (!std::is_trivially_destructible_v<T>) {
						itr->~value_type();
					}
				}
				currentDataSize = arg_size;
			}

		}
		else {
			assert(0 && "デフォルトコンストラクタの存在しない型のResizeは不可能です");
		}
	}
	inline void Resize(const std::int32_t arg_size, const_reference_type arg_item)
	{
		if constexpr (std::is_copy_constructible_v<T>) {
			if (arg_size <= 0 || arg_size == currentDataSize) { return; }
			if (arg_size > currentDataSize) {
				Reserve(arg_size);
				for (auto itr = end().Ptr(); currentDataSize < arg_size; currentDataSize++, itr++) {
					copy_construct(itr, arg_item);
				}
			}
			else {
				for (auto itr = (begin() + arg_size), endItr = end(); itr < endItr; itr++) {
					if constexpr (!std::is_trivially_destructible_v<T>) {
						itr->~value_type();
					}
				}
				currentDataSize = arg_size;
			}

		}
		else {
			assert(0 && "コピーコンストラクタの存在しない型のResizeは不可能です");
		}
	}

	bool Contains(const value_type& arg_item) const
	{
		return std::find(begin(), end(), arg_item) != end();
	}

	template<typename F>
	bool Contains(F arg_function) const
	{
		return std::find_if(begin(), end(), arg_function) != end();
	}

	std::int32_t IndexOf(const value_type& arg_item, std::int32_t arg_startIndex = 0) const
	{
		if (IsEmpty()) { return -1; }
		CheckOutOfRange(arg_startIndex);
		auto itr = std::find(begin() + arg_startIndex, end(), arg_item);
		return itr != end() ? itr - begin() : -1;
	}

	template<typename F>
	std::int32_t IndexOf(F arg_function, std::int32_t arg_startIndex = 0) const
	{
		if (IsEmpty()) { return -1; }
		CheckOutOfRange(arg_startIndex);
		auto itr = std::find_if(begin() + arg_startIndex, end(), arg_function);
		return itr != end() ? itr - begin() : -1;
	}

	iterator_type Find(const value_type& arg_item) const
	{
		auto itr = std::find(begin(), end(), arg_item);
		if (itr != end()) {
			return itr;
		}
		return itr != end() ? itr : nullptr;
	}

	template<typename F>
	iterator_type Find(F arg_function) const
	{
		auto itr = std::find_if(begin(), end(), arg_function);
		return itr != end() ? itr : nullptr;
	}


	reference_type GetFront()
	{
		CheckEmpty();
		return *begin();
	}

	const_reference_type GetFront() const
	{
		CheckEmpty();
		return *begin();
	}

	reference_type GetLast()
	{
		CheckEmpty();
		return *(end() - 1);
	}

	const_reference_type GetLast() const
	{
		CheckEmpty();
		return *(end() - 1);
	}
	reference_type At(const std::uint32_t arg_index)
	{
		CheckOutOfRange(arg_index);
		return reinterpret_cast<pointer_type>(p_data)[arg_index];
	}

	const_reference_type At(const std::uint32_t arg_index) const
	{
		CheckOutOfRange(arg_index);
		return reinterpret_cast<pointer_type>(p_data)[arg_index];
	}
	reference_type at(const std::uint32_t arg_index)
	{
		CheckOutOfRange(arg_index);
		return reinterpret_cast<pointer_type>(p_data)[arg_index];
	}

	const_reference_type at(const std::uint32_t arg_index) const
	{
		CheckOutOfRange(arg_index);
		return reinterpret_cast<pointer_type>(p_data)[arg_index];
	}

	inline bool IsOutOfRange(const std::uint32_t arg_index) const noexcept
	{
		return (arg_index < 0 || GetSize() <= arg_index);
	}

	List& operator=(const List& arg_other) {
		Clear();
		dealloc();
		currentCapacity = 0;
		currentDataSize = 0;
		Add(arg_other);
		return *this;
	}

	reference_type operator[] (const std::int32_t arg_index) {
		CheckOutOfRange(arg_index);
		return reinterpret_cast<pointer_type>(p_data)[arg_index];
	}

	reference_type operator[] (const std::int32_t arg_index) const {
		CheckOutOfRange(arg_index);
		return reinterpret_cast<pointer_type>(p_data)[arg_index];
	}

	inline std::uint32_t Reserve(const std::uint32_t arg_size) {
		if (arg_size <= currentCapacity) {
			return currentCapacity;
		}

		auto keepSizeAlignment = max_(currentCapacity, 1) * 2;
		for (; keepSizeAlignment < arg_size; keepSizeAlignment *= 2);
		auto p_newData = ButiContainerDetail::ContainerAllocator<value_type>::allocate(keepSizeAlignment);
		for (auto itr = begin().Ptr(), endItr = end().Ptr(), newDataItr = p_newData; itr != endItr; itr++, newDataItr++) {
			copy_construct(newDataItr, *itr);
			if constexpr (!std::is_trivially_destructible_v<T>) {
				itr->~value_type();
			}
		}
		dealloc();
		p_data = p_newData;
		currentCapacity = keepSizeAlignment;
		return keepSizeAlignment;
	}

	iterator_type  begin() { return iterator_type(reinterpret_cast<pointer_type>(p_data)); }
	const_iterator_type	begin() const { return iterator_type(reinterpret_cast<pointer_type>(p_data)); }
	iterator_type end() { return iterator_type(reinterpret_cast<pointer_type>(p_data) + currentDataSize); }
	const_iterator_type end() const { return iterator_type(reinterpret_cast<pointer_type>(p_data) + currentDataSize); }

	reverse_iterator_type  rbegin() { return reverse_iterator_type(reinterpret_cast<pointer_type>(p_data)); }
	const_reverse_iterator_type	rbegin() const { return reverse_iterator_type(reinterpret_cast<pointer_type>(p_data)); }
	reverse_iterator_type rend() { return reverse_iterator_type(reinterpret_cast<pointer_type>(p_data) + currentDataSize); }
	const_reverse_iterator_type rend() const { return reverse_iterator_type(reinterpret_cast<pointer_type>(p_data) + currentDataSize); }

	const_iterator_type cbegin() { return iterator_type(reinterpret_cast<pointer_type>(p_data)); }
	const_iterator_type cend() const { return iterator_type(reinterpret_cast<pointer_type>(p_data) + currentDataSize); }
	iterator_type erase(iterator_type arg_pos) {
		auto currentPos = arg_pos;
		for (auto endItr = end() - 1; currentPos != endItr; currentPos++) {
			assign(currentPos.Ptr(), *(currentPos + 1));
		}
		currentDataSize--;
		if constexpr (!std::is_trivially_destructible_v<T>) {
			end()->~value_type();
		}
		return arg_pos;
	}
	iterator_type erase(iterator_type arg_begin, iterator_type arg_end) {
		for (auto itr = arg_begin, endItr = end() - 1; itr != endItr; itr++) {
			assign(itr.Ptr(), *(itr + 1));
		}
		currentDataSize -= arg_end - arg_begin;
		for (auto itr = end(), endItr = end() + (arg_end - arg_begin); itr != endItr; itr++) {
			if constexpr (!std::is_trivially_destructible_v<T>) {
				itr->~value_type();
			}
		}
		return arg_begin;
	}
	inline const T* data()const {
		return p_data;
	}
	inline T* data(){
		return p_data;
	}

private:

	inline static void  copy_construct(pointer_type arg_pos, const_reference_type arg_item) {
		new (arg_pos) value_type(std::forward<const_value_type>(arg_item));
	}
	inline static void  default_construct(pointer_type arg_pos) {
		new (arg_pos) value_type();
	}
	inline static void  assign(pointer_type arg_pos, const_reference_type arg_item) {
		*arg_pos = arg_item;
	}
	inline void dealloc() {
		if (p_data) {
			ButiContainerDetail::ContainerAllocator<value_type>::deallocate(reinterpret_cast<void*>(p_data), currentCapacity);
			p_data = nullptr;
		}
	}


	inline void CheckOutOfRange(const std::int32_t arg_index) const
	{
		if (IsOutOfRange(arg_index)) {
#ifdef BUTIEXCEPTION_DEFINED
			throw ButiEngine::ButiException(L"Listの範囲外へのアクセスです");
#else
			throw std::exception();
#endif // BUTIEXCEPTION_DEFINED

		}
	}

	inline void CheckEmpty()const {
		if (IsEmpty()) {
#ifdef BUTIEXCEPTION_DEFINED
			throw ButiEngine::ButiException(L"Listが空です");
#else
			throw std::exception();
#endif // BUTIEXCEPTION_DEFINED
		}
	}

	T* p_data = nullptr;
	std::uint32_t currentDataSize = 0;
	std::uint32_t currentCapacity = 0;

};


}

#endif//BUTILIST_H