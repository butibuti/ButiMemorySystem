
#ifndef BUTIPTR_H
#define BUTIPTR_H
#include<string>
#include<type_traits>
#include <cassert>
#include"MemoryAllocator.h"
#include"RefferenceCounter.h"
#ifdef __vm_value_h__
namespace std {
template<typename T>
inline std::string to_string(const ButiScript::Type_hasMember<T>& arg_v);
}
#endif // __vm_value_h__

namespace ButiEngine {



template<typename T, typename RefferenceObject = SmartPtrDetail::RefferenceCounter::RefferenceCounter_weak<ButiMemorySystem::Allocator>>
class Value_weak_ptr {
	using this_type = Value_weak_ptr<T, RefferenceObject>;
	using refCounter_type = RefferenceObject;
	using value_type = T;
	using element_type = std::remove_all_extents_t<T>;
	using pointer = T*;
	using const_pointer = const T*;

	template<typename S, typename R> friend class Value_ptr;
	template<typename S, typename R> friend class Value_weak_ptr;

public:
	Value_weak_ptr()noexcept :p_value(nullptr){}
	Value_weak_ptr(const this_type& arg_other)noexcept {
		p_value = arg_other.p_value;
		refferenceCounter = arg_other.refferenceCounter;
	}
	Value_weak_ptr(const Value_ptr<T>& arg_other)noexcept {
		p_value = arg_other.p_value;
		refferenceCounter = arg_other.refferenceCounter;
	}
	template<typename S>
	this_type& operator=(const Value_ptr<S>& arg_other) noexcept {
		if constexpr (std::is_void_v<S>) {
			p_value = reinterpret_cast<T*>(arg_other.p_value);
		}
		else {
			p_value = arg_other.p_value;
		}
		refferenceCounter = arg_other.refferenceCounter;
		return *this;
	}
	template<typename S>
	this_type& operator=(const Value_weak_ptr<S>& arg_other) noexcept {
		if constexpr (std::is_void_v<S>) {
			p_value = reinterpret_cast<T*>(arg_other.p_value);
		}
		else {
			p_value = arg_other.p_value;
		}
		refferenceCounter = arg_other.refferenceCounter;
		return *this;
	}
	this_type& operator=(std::nullptr_t) noexcept {
		p_value =nullptr;
		refferenceCounter = refCounter_type();
		return *this;
	}
	bool operator==(const this_type& arg_other)const noexcept {
		return p_value == arg_other.p_value;
	}
	operator Value_weak_ptr<void> ()const {
		auto output = Value_weak_ptr<void>();
		output.p_value=p_value;
		output.refferenceCounter = refferenceCounter;
		return output;
	}
	[[nodiscard]] Value_ptr<T> lock() const noexcept{
		if (refferenceCounter.count()) {
			return Value_ptr<T>(*this);
		}
		else {
			return nullptr;
		}
	}
private:
	pointer p_value;
	refCounter_type refferenceCounter;
};
template<typename T, typename RefferenceObject >
class Value_ptr {
public:
	using this_type = Value_ptr<T, RefferenceObject>;
	using refCounter_type = RefferenceObject;
	using value_type = T; 
	using element_type =std::remove_all_extents_t<T>;
	using pointer = T*;
	using const_pointer = const T*;
	using reference = SmartPtrDetail::TypeStorage::type_to_ref<T>;
	using const_reference = SmartPtrDetail::TypeStorage::type_to_constRef<T>;

	inline constexpr Value_ptr()noexcept :p_value(nullptr), refferenceCounter() {}
	inline constexpr Value_ptr(std::nullptr_t)noexcept :p_value(nullptr), refferenceCounter() {}
	inline Value_ptr(const this_type& arg_s) : p_value(arg_s.p_value), refferenceCounter(arg_s.refferenceCounter) {}
	template<typename S, std::enable_if_t<std::is_convertible_v<S*, pointer>, std::int32_t> = 0>
	inline Value_ptr(const Value_ptr<S, refCounter_type>& arg_s) : p_value(arg_s.p_value), refferenceCounter(arg_s.refferenceCounter) {}
	template<typename S>
	inline explicit Value_ptr(const S& arg_v) :Value_ptr(refCounter_type::Alloc::allocate(arg_v)){	}
	template <class U>
	Value_ptr(const Value_ptr<U>& arg_other, element_type* arg_p) noexcept {
		refferenceCounter = arg_other.refferenceCounter;
		p_value = arg_p;
	}
	inline Value_ptr(T* arg_p, RefferenceObject arg_refObj) :p_value(arg_p), refferenceCounter(arg_refObj) {}
	template<typename S>
	inline explicit Value_ptr(S* arg_p) :p_value(arg_p), refferenceCounter(arg_p, SmartPtrDetail::TypeStorage::TypeStorageSpecifier()) {
		_SetWeakPtrEnableValue(arg_p);
	}
	template<typename S>
	inline explicit Value_ptr(S* arg_p, [[maybe_unused]] const SmartPtrDetail::MemberPtrNotify) :p_value(arg_p), refferenceCounter(arg_p, SmartPtrDetail::TypeStorage::MemberTypeStorageSpecifier()) {}
	template<typename S, typename D>
	inline explicit Value_ptr(S* arg_p, D arg_d) :p_value(arg_p), refferenceCounter(arg_p, arg_d) {
		_SetWeakPtrEnableValue(arg_p);
	}
	template<typename S>
	inline constexpr Value_ptr(const Value_weak_ptr<S> arg_vwp_other)noexcept {
		ConstructFromWeak(arg_vwp_other);
	}

	~Value_ptr() {}
	inline this_type CreateSameTypeValuePtr(void* arg_p_src) const{
		return this_type(arg_p_src, refferenceCounter.CreateSameType(arg_p_src));
	}

	inline void reset() { this_type().swap(*this); }
	inline void reset(const this_type& arg_s) { this_type(arg_s).swap(*this); }
	template<class S> void reset(S* arg_p) { this_type(arg_p).swap(*this); }
	template<class S, class D>
	inline void reset(S* arg_p, D arg_d) { this_type(arg_p, arg_d).swap(*this); }
	inline void swap(this_type& arg_other) {
		std::swap(p_value, arg_other.p_value);
		refferenceCounter.swap(arg_other.refferenceCounter);
	}

	inline this_type& operator=(const this_type& arg_s) {
		p_value = arg_s.p_value;
		refferenceCounter = arg_s.refferenceCounter;
		return *this;
	}
	template<class S>
	inline this_type& operator=(const Value_ptr<S, refCounter_type>& arg_s) {
		p_value = arg_s.p_value;
		refferenceCounter = arg_s.refferenceCounter;
		return *this;
	}
	template<class S>
	inline bool operator<(const Value_ptr<S, refCounter_type>& arg_s)const {
		return p_value < arg_s.p_value;
	}
	template<class S>
	inline bool operator<=(const Value_ptr<S, refCounter_type>& arg_s)const {
		return p_value <= arg_s.p_value;
	}
	template<class S>
	inline bool operator>(const Value_ptr<S, refCounter_type>& arg_s)const {
		return p_value > arg_s.p_value;
	}
	template<class S>
	inline bool operator>=(const Value_ptr<S, refCounter_type>& arg_s)const {
		return p_value >= arg_s.p_value;
	}

	inline std::string ToString() const{
		return refferenceCounter.GetTypeStorage()->ToString();
	}
	Value_ptr<T> Clone()const {
		auto typeStorage = refferenceCounter.GetTypeStorage()->Clone();
		return Value_ptr<T>(reinterpret_cast<pointer>(typeStorage->ptr()), typeStorage);
	}
	/// <summary>
	/// データの直接書き込み
	/// </summary>
	/// <param name="arg_src">書き込み元のポインタ、型が同一と信頼</param>
	inline void Write(const void* arg_src) {
		refferenceCounter.Write(arg_src);
	}
	template<typename CastType>
	inline Value_ptr<CastType> StaticCast()const {
		auto typeStorage = refferenceCounter.GetTypeStorage()->StaticCast<T, CastType, refCounter_type::Alloc>();
		return Value_ptr<CastType>(reinterpret_cast<CastType*>(typeStorage->ptr()), typeStorage);
	}
	template<typename OwnType, typename CastType>
	inline Value_ptr<CastType> StaticCast()const {
		auto typeStorage = refferenceCounter.GetTypeStorage()->StaticCast<OwnType, CastType, refCounter_type::Alloc>();
		return Value_ptr<CastType>(reinterpret_cast<CastType*>(typeStorage->ptr()), typeStorage);
	}

	inline reference operator*() { if constexpr (std::is_same_v<element_type, void>) {} else { return *get(); } }
	inline const_reference operator*()const { if constexpr (std::is_same_v<element_type, void>) {} else { return *get(); } }
	//inline pointer operator->() { return get(); }
	inline pointer operator->()const { return get(); }

	inline pointer get()const { return p_value; }
	template<typename RetType>
	inline RetType* get() { return reinterpret_cast<RetType*>(p_value); }
	template<typename RetType>
	inline const RetType* get()const { return reinterpret_cast<RetType*>(p_value); }

	inline bool unique()const { return refferenceCounter.unique(); }
	inline std::uint64_t use_count()const { return refferenceCounter.use_count(); }

	inline operator bool()const { return !!p_value; }
	inline bool operator==(std::nullptr_t)const {
		return !p_value;
	}
	inline bool operator!=(std::nullptr_t)const {
		return p_value;
	}
	inline operator std::int32_t() const = delete;
	inline bool operator!()const { return !p_value; }
	inline bool ShowGUI(const std::string& arg_label) {
		return refferenceCounter.ShowGUI(arg_label);
	}

	inline Value_ptr<IValuePtrRestoreObject, refCounter_type> GetRestoreObject() const {
		Value_ptr<IValuePtrRestoreObject, refCounter_type> output;
		refferenceCounter.GetRestoreObject(output);
		return output;
	}

private:
	template<typename S>
	inline bool ConstructFromWeak(const Value_weak_ptr<S>& arg_vwp_other) noexcept {
		if (arg_vwp_other.p_value) {
			p_value = arg_vwp_other.p_value;
			refferenceCounter = arg_vwp_other.refferenceCounter;
			refferenceCounter._Increase_notZero();
			return true;
		}

		return false;
	}
	template <class U>
	inline constexpr void _SetWeakPtrEnableValue(U* arg_p_this) noexcept {
		if constexpr (std::conjunction_v<std::negation<std::is_array<T>>, std::negation<std::is_volatile<U>>, _Can_enable_value<U>>) {
			arg_p_this->weak_ptr = *this;
		}
	}
	template<typename S, typename R> friend class Value_ptr;
	template<typename S, typename R> friend class Value_weak_ptr;
	pointer p_value;
	refCounter_type refferenceCounter;
};


template<class T, class U, class R>
inline bool operator==(const Value_ptr<T, R>& arg_lhs, const Value_ptr<U, R>& arg_rhs) {
	return arg_lhs.get() == arg_rhs.get();
}
template<class T, class U, class R>
inline bool operator!=(const Value_ptr<T, R>& arg_lhs, const Value_ptr<U, R>& arg_rhs) {
	return arg_lhs.get() != arg_rhs.get();
}
template <typename T>
class enable_value_from_this {
public:

	using _Evft_type = enable_value_from_this;
	template<typename S, typename R> friend class Value_ptr;

	[[nodiscard]] Value_ptr<T> value_from_this() {
		return Value_ptr<T>(weak_ptr);
	}

	[[nodiscard]] Value_ptr<const T> value_from_this() const {
		return Value_ptr<const T>(weak_ptr);
	}
protected:
	constexpr enable_value_from_this() noexcept :weak_ptr(){}

	enable_value_from_this(const enable_value_from_this&) noexcept :weak_ptr(){
	}

	enable_value_from_this& operator=(const enable_value_from_this&) noexcept {
		return *this;
	}
	/// <summary>
	/// Value_ptrをポインタのように扱わなければならないときの最終兵器
	/// </summary>
	/// <returns>weak_ptrへのアドレス</returns>
	const Value_weak_ptr<T>* weakAddress()const noexcept { return &weak_ptr; }
	Value_weak_ptr<T>* weakAddress()noexcept{ return &weak_ptr; }
	~enable_value_from_this() = default;

	template<typename S, typename R> friend class Value_ptr;
private:
	Value_weak_ptr<T> weak_ptr;
};

template<typename T, typename... Args>
inline constexpr Value_ptr<T> make_value(Args&&... args) {
	return Value_ptr<T>(ButiMemorySystem::Allocator::template allocate<T>(args...));
}
template <typename T>
inline Value_ptr<T> to_value(T* v) {
	return Value_ptr<T>(v, SmartPtrDetail::MemberNotify);
}

template<typename T>
class ValuePtrRestoreObject :public IValuePtrRestoreObject {
public:
	template<typename T, std::enable_if_t<std::is_copy_constructible_v<T>, std::int32_t> = 0>
	ValuePtrRestoreObject(const T& arg_value) :data(arg_value) {
	}
	ValuePtrRestoreObject() {
	}
	void RestoreValue(ButiEngine::Value_ptr<void>& arg_ref_value)const override;
	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(data);
	}
private:
	T data;
};
template <typename Allocator>
inline void SmartPtrDetail::RefferenceCounter::RefferenceCounter<Allocator>::GetRestoreObject(Value_ptr<IValuePtrRestoreObject>& arg_ref_vlp)const {
	p_impl->p_typeStorage->GetRestoreObject(arg_ref_vlp);
}
template<typename T,typename Allocator>
inline void SmartPtrDetail::TypeStorage::TypeStorage<T,Allocator>::GetRestoreObject(Value_ptr<IValuePtrRestoreObject>& arg_ref_vlp)const {
	if constexpr (!std::is_base_of_v<IValuePtrRestoreObject, T> && std::is_copy_constructible_v<T>) {
		arg_ref_vlp = make_value<ValuePtrRestoreObject<T>>(*p_value);
	}
}
template<typename T,typename Allocator>
inline void SmartPtrDetail::TypeStorage::MemberTypeStorage<T,Allocator>::GetRestoreObject(Value_ptr<IValuePtrRestoreObject>& arg_ref_vlp)const {
	if constexpr (!std::is_base_of_v<IValuePtrRestoreObject, T>&& std::is_copy_constructible_v<T>) {
		arg_ref_vlp = make_value<ValuePtrRestoreObject<T>>(*p_value);
	}
}
template<typename T>
void ValuePtrRestoreObject<T>::RestoreValue(ButiEngine::Value_ptr<void>& arg_ref_value)const
{
	arg_ref_value = make_value<T>(data);
}
template<typename AfterType, typename BeforeType>
static inline constexpr Value_ptr<AfterType> dynamic_value_ptr_cast(const Value_ptr<BeforeType>& arg_before) {
	const auto temp = dynamic_cast<Value_ptr<AfterType>::element_type*>(arg_before.get());
	if (temp) {
		return Value_ptr<AfterType>(arg_before, temp);
	}
	return nullptr;
}
template<typename AfterType, typename BeforeType>
static inline constexpr Value_ptr<AfterType> dynamic_value_ptr_cast(Value_ptr<BeforeType>&& arg_before) {
	const auto temp = dynamic_cast<Value_ptr<AfterType>::element_type*>(arg_before.get());
	if (temp) {
		return Value_ptr<AfterType>(std::move(arg_before), temp);
	}
	return nullptr;
}

template<typename AfterType, typename BeforeType>
static inline constexpr Value_ptr<AfterType> static_value_ptr_cast(const Value_ptr<BeforeType>& arg_before) {
	const auto temp = static_cast<Value_ptr<AfterType>::element_type*>(arg_before.get());
	return Value_ptr<AfterType>(arg_before, temp);
}
template<typename AfterType, typename BeforeType>
static inline constexpr Value_ptr<AfterType> static_value_ptr_cast(Value_ptr<BeforeType>&& arg_before) {
	const auto temp = static_cast<Value_ptr<AfterType>::element_type*>(arg_before.get());
	return Value_ptr<AfterType>(std::move( arg_before), temp);
}
template <typename T>
static inline Value_ptr<T> CreateSameTypeValuePtr(Value_ptr<T> arg_baseValuePtr, void* arg_p_src) {
	return arg_baseValuePtr->CreateSameTypeValuePtr(arg_p_src);
}
}

//boostへの対応
namespace boost {
template<typename T> T* get_pointer(const ButiEngine::Value_ptr<T>& arg_vlp)
{
	return arg_vlp.get();
}

}


//mapへの対応
namespace std {

template<typename T>
struct hash<ButiEngine::Value_ptr<T>> {
public:
	size_t operator()(const ButiEngine::Value_ptr<T>& data)const {
		return reinterpret_cast<std::uint64_t>(data.get());
	}
};
}
#endif // !BUTIPTR_H
