
#ifndef BUTIPTR_H
#define BUTIPTR_H
#include<string>
#include<type_traits>
#include <cassert>
#include"MemoryAllocator.h"

namespace ButiEngine {
namespace GUI {
bool Input(const std::string&, nullptr_t);
}
class IEnable_value_from_this;

template <class T, class = void>
struct _Can_enable_value: std::false_type {}; // detect unambiguous and accessible inheritance from enable_shared_from_this

template <class T>
struct _Can_enable_value<T, std::void_t<typename T::_Evft_type>>
	: std::is_convertible<std::remove_cv_t<T>*, typename T::_Evft_type*>::type {};
#ifndef BUTIPTR_FORWARD_H
#define BUTIPTR_FORWARD_H
namespace SmartPtrDetail {
namespace RefferenceCounter {
template<typename Allocator = ButiMemorySystem::Allocator>
class RefferenceCounter;
template<typename Allocator = ButiMemorySystem::Allocator>
class RefferenceCounter_weak;
}
}
template<typename T, typename RefferenceObject = SmartPtrDetail::RefferenceCounter::RefferenceCounter<ButiMemorySystem::Allocator>>
class Value_ptr;
#endif

/// <summary>
/// Value_ptr<void>を保存するために経由するインターフェース
/// </summary>
class IValuePtrRestoreObject {
public:
	virtual void RestoreValue(ButiEngine::Value_ptr<void>&)const = 0;
	template<class Archive>	void serialize(Archive& archive){}
};


template<typename T>
constexpr bool False(){ return false; }

namespace SmartPtrDetail {
template <typename T,typename Allocator>
T* CloneValue(const T* arg_p_v) {
	if constexpr (std::is_copy_constructible_v<T>) {
		auto output = Allocator::template allocate<T>(*arg_p_v);
		return output;
	}
	else if constexpr(std::is_copy_assignable_v<T>&&std::is_default_constructible_v<T>) {
		auto output = Allocator::template allocate<T>();
		*output = *arg_p_v;
		return output;
	}
	assert(0 && "コピーが不可能な型をコピーしようとしています");
	return nullptr;
}

class MemberPtrNotify {};
static MemberPtrNotify MemberNotify;
namespace TypeStorage {
struct has_ShowGUI_StrArg_impl {
	template <typename T>
	static auto check(T&& x)->decltype(x.ShowGUI("TestLabel"), std::true_type{});

	template <typename T>
	static auto check(...)->std::false_type;
};
struct has_ShowGUI_impl {
	template <typename T>
	static auto check(T&& x)->decltype(x.ShowGUI(), std::true_type{});

	template <typename T>
	static auto check(...)->std::false_type;
};
template <typename T>
class has_ShowGUI_StrArg : public decltype(has_ShowGUI_StrArg_impl::check<T>(std::declval<T>())) {};
template <typename T>
class has_ShowGUI : public decltype(has_ShowGUI_impl::check<T>(std::declval<T>())) {};

template <typename T>
constexpr bool has_ShowGUI_StrArg_v = has_ShowGUI_StrArg<T>();
template <typename T>
constexpr bool has_ShowGUI_v = has_ShowGUI<T>();

template<typename T>struct type_to_ref_impl { using reftype = T&; };
template <> struct type_to_ref_impl<void> { using reftype = void; };

template<typename T>struct type_to_constRef_impl { using reftype =const T&; };
template <> struct type_to_constRef_impl <void> { using reftype = void; };

template <typename T> using type_to_ref =typename type_to_ref_impl<T>::reftype;
template <typename T> using type_to_constRef = typename type_to_constRef_impl<T>::reftype;

struct FunctionCheck_std_to_string {
	template <typename T>
	auto operator()(T&& arg_check)->decltype(std::to_string(arg_check));
};

struct FunctionCheck_GUI_Input {
	template <typename T>
	auto operator()(T&& arg_check) -> decltype(GUI::Input("", arg_check));
};
template<typename T>
inline T& Through(T& arg_ref_v) { return arg_ref_v; }

template <typename T, typename Allocator = ButiMemorySystem::Allocator>
class TypeStorage;
template <typename T, typename Allocator = ButiMemorySystem::Allocator>
class MemberTypeStorage;
/// <summary>
/// インスタンスが生成された際の型を記憶し、適切なデストラクタ呼び出し、コピー、String変換、GUI呼び出しを行う
/// </summary>
class ITypeStorage {
public:
	virtual void Destroy() = 0;
	virtual ITypeStorage* Clone() const= 0;
	virtual ITypeStorage* CreateSameTypeStorage(void * arg_src)const = 0;
	virtual std::string ToString() const= 0;
	virtual void* ptr() = 0;
	virtual const void* ptr() const = 0;
	template <typename OwnType,typename CastType,typename Allocator>
	inline ITypeStorage* StaticCast()const;
	template<std::int32_t size>
	inline void Write(const void* arg_src) {
		memcpy_s(ptr(), size, arg_src, size);
	}
	virtual void Write(const void* arg_src) = 0;
	virtual bool ShowGUI(const std::string& arg_label) = 0;
	virtual inline void GetRestoreObject(Value_ptr<IValuePtrRestoreObject>& arg_ref_vlp) const = 0;
protected:
	template<typename T>
	inline bool ShowGUI_(const std::string& arg_label, T& arg_value) {
		if constexpr (std::is_invocable_v<FunctionCheck_GUI_Input, T>)
		{
			return GUI::Input(arg_label, arg_value);
		}
		else if constexpr (has_ShowGUI_v<T>)
		{
			arg_value.ShowGUI();
		}
		else if constexpr (has_ShowGUI_StrArg_v<T>)
		{
			return arg_value.ShowGUI(arg_label);
		}
		else {
			//GUI::Text(arg_label + ":対応していない型です");
		}
		return false;
	}
	template<typename T>
	inline std::string ToString_(const T& arg_value) const{
		if constexpr (std::is_same_v<std::string, std::remove_all_extents_t<T>>) {
			return arg_value;
		}
		else if constexpr (std::is_invocable_v<FunctionCheck_std_to_string, T>) {
			return std::to_string(arg_value);
		}
		else {
			return "このクラスはto_stringに対応していません";
		}
	}
};
template <typename T,typename Allocator>
class TypeStorage : public ITypeStorage {
	friend typename Allocator;
public:
	using this_type = TypeStorage<T,Allocator>;
	TypeStorage(T* arg_p) :p_value(arg_p) {}
	inline void Destroy()override {
		Allocator::deallocate(p_value);
		Allocator::deallocate(this);
	}

	inline ITypeStorage* Clone()const override {
		return Allocator::template allocate<this_type>(CloneValue<T,Allocator>(p_value));
	}
	//同じ型のTypeStorageを作成する
	inline ITypeStorage* CreateSameTypeStorage(void* arg_src)const override { 
		return Allocator::template allocate<this_type>(reinterpret_cast<T*>(arg_src));
	}
	inline std::string ToString()const override {
		return ToString_(*p_value);
	}
	inline  void* ptr()override { return p_value; }
	inline const void* ptr()const override { return p_value; }
	inline void Write(const void* arg_src)override {
		if constexpr (std::is_copy_assignable_v<T>)
		{
			*p_value = *reinterpret_cast<const T*>(arg_src);
		}
	}
	static this_type* get(T* arg_p) {return Allocator::template allocate<this_type>(arg_p);}
	inline bool ShowGUI(const std::string& arg_label) {
		return ShowGUI_(arg_label, *p_value);
	}
	inline void GetRestoreObject(Value_ptr<IValuePtrRestoreObject>& arg_ref_vlp)const override;
private:
	template<typename T,typename Allocator> friend class MemberTypeStorage;
	this_type& operator=(const this_type&) { return *this; }
	TypeStorage(const this_type& arg_rhs) : p_value(arg_rhs.p_value) {}
	TypeStorage() = delete;
	T* p_value;
};
template <typename T,typename Allocator>
class MemberTypeStorage : public ITypeStorage {
	friend typename Allocator;
public:
	using this_type = MemberTypeStorage<T,Allocator>;
	inline void Destroy()override {	Allocator::deallocate(this);}
	//値のクローン作成(今後メモリ確保は変更)
	inline ITypeStorage* Clone()const override {
		return Allocator::template allocate<TypeStorage<T>>(CloneValue<T,Allocator>(p_value));
	}
	//同じ型のTypeStorageを作成する
	inline ITypeStorage* CreateSameTypeStorage(void* arg_src)const override { return Allocator::template allocate<this_type>(reinterpret_cast<T*>(arg_src)); }
	inline  void* ptr()override { return p_value; }
	inline const void* ptr()const override { return p_value; }
	inline std::string ToString()const override {
		return ToString_(*p_value);
	}
	static this_type* get(T* arg_p) { return Allocator::template allocate<this_type>(arg_p); }
	inline void Write(const void* arg_src)override {
		*p_value = *reinterpret_cast<const T*>(arg_src);
	}
	inline bool ShowGUI(const std::string& arg_label) {
		return ShowGUI_(arg_label, *p_value);
	}
	inline void GetRestoreObject(Value_ptr<IValuePtrRestoreObject>& arg_ref_vlp)const override;
private:
	this_type& operator=(const this_type&) { return *this; }
	MemberTypeStorage(const this_type&) = delete;
	MemberTypeStorage() = delete;
	MemberTypeStorage(T* arg_p) :p_value(arg_p) {}
	T* p_value;
};

template<typename OwnType, typename CastType, typename Allocator>
inline ITypeStorage* ITypeStorage::StaticCast() const
{
	return Allocator::template allocate<TypeStorage<CastType>>(Allocator::template allocate<CastType>( static_cast<CastType>(* reinterpret_cast<const OwnType*>(ptr()))));
}

class TypeStorageSpecifier {
public:
	template<typename T> class type : public TypeStorage<T> {
	public:
		using name = TypeStorage<T>;
	};
};
class MemberTypeStorageSpecifier {
public:
	template<typename T> class type : public MemberTypeStorage<T> {
	public:
		using name = MemberTypeStorage<T>;
	};
};
}
namespace RefferenceCounter {
template<typename Allocator>
class RefferenceCounter {
	using this_type = RefferenceCounter<ButiMemorySystem::Allocator>;
public:
	using Alloc = Allocator;
	constexpr RefferenceCounter()noexcept :p_typeStorage(nullptr),use(0),weak(0) {}
	constexpr RefferenceCounter(TypeStorage::ITypeStorage* arg_p_typeStorage) :p_typeStorage(arg_p_typeStorage), use(0), weak(0) {}
	RefferenceCounter(this_type* arg_r) : p_typeStorage(arg_r->p_typeStorage) {}	


	template<typename S, typename D>
	explicit RefferenceCounter(S* arg_p, D arg_d):use(1), weak(0), p_typeStorage(D::template type<S>::name::get(arg_p)) {}
	template<typename S, typename D>
	explicit RefferenceCounter(S* arg_p, D* arg_p_d):use(1), weak(0), p_typeStorage(arg_p_d) {}
	~RefferenceCounter() {  }
	inline bool release() {
		if (!_Decrease_notZero()&& p_typeStorage) {
			auto storage = p_typeStorage;
			p_typeStorage = nullptr;
			if (!weak) {
				Alloc::deallocate(this);
			}
			storage->Destroy();			
			return true;
		}
		return false;
	}
	inline void release_weak() {
		if (!_Decrease_weak_notZero()&&!use) {
			Alloc::deallocate(this);
		}
	}
	inline void Write(const void* arg_src) {
		p_typeStorage->Write(arg_src);
	}
	inline void swap(this_type& r) { std::swap(p_typeStorage, r.p_typeStorage); }
	inline TypeStorage::ITypeStorage* GetTypeStorage() { return p_typeStorage; }
	inline const TypeStorage::ITypeStorage* GetTypeStorage()const { return p_typeStorage; }
	inline bool unique()const { return use_count() == 1; }
	inline std::uint64_t use_count()const { return p_typeStorage ? use : 0; }
	inline std::uint64_t weak_count()const { return p_typeStorage ? weak: 0; }
	inline bool ShowGUI(const std::string& arg_label) {
		return p_typeStorage ?p_typeStorage->ShowGUI(arg_label):false;
	}
	inline std::int64_t _Increase() {
		return _InterlockedIncrement64(reinterpret_cast<volatile std::int64_t*>(&use));
	}
	inline std::int64_t _Decrease() {
		return _InterlockedDecrement64(reinterpret_cast<volatile std::int64_t*>(&use));
	}
	inline std::int64_t _Increase_weak() {
		return _InterlockedIncrement64(reinterpret_cast<volatile std::int64_t*>(&weak));
	}
	inline std::int64_t _Decrease_weak() {
		return _InterlockedDecrement64(reinterpret_cast<volatile std::int64_t*>(&weak));
	}
	inline bool _Increase_notZero() {
		return	use ? _Increase() : false;
	}
	inline bool _Decrease_notZero() {
		return	use ? _Decrease() : false;
	}
	inline bool _Increase_weak_notZero() {
		return	weak ? _Increase_weak() : false;
	}
	inline bool _Decrease_weak_notZero() {
		return	weak ? _Decrease_weak() : false;
	}
	inline void GetRestoreObject(Value_ptr<IValuePtrRestoreObject>& arg_ref_vlp) const;

private:
	
	RefferenceCounter* CreateSameTypeStorage(void* arg_p_src)const {
		return Alloc::template allocate< RefferenceCounter>(p_typeStorage->CreateSameTypeStorage(arg_p_src));
	}
	std::uint64_t use=0, weak=0;
	TypeStorage::ITypeStorage* p_typeStorage=nullptr;
};

}
}

template<typename T, typename RefferenceObject = SmartPtrDetail::RefferenceCounter::RefferenceCounter<ButiMemorySystem::Allocator>>
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
		p_refferenceCounter = arg_other.p_refferenceCounter;
		Increase();
	}
	Value_weak_ptr(const Value_ptr<T>& arg_other)noexcept {
		p_value = arg_other.p_value; 
		p_refferenceCounter = arg_other.p_refferenceCounter;
		Increase();
	}

	template<typename S, std::enable_if_t<std::is_convertible_v<S*, pointer>, std::int32_t> = 0>
	inline Value_weak_ptr(const Value_ptr<S, refCounter_type>& arg_s) : p_value(arg_s.p_value) {
		p_refferenceCounter = arg_s.p_refferenceCounter;
		Increase();
	}
	~Value_weak_ptr() {
		Release();
	}
	this_type& operator=(const this_type& arg_other) noexcept {
		p_value = arg_other.p_value;
		Release(); p_refferenceCounter = arg_other.p_refferenceCounter;
		Increase();
		return *this;
	}
	this_type& operator=(const Value_ptr<T>& arg_other) noexcept {
		p_value = arg_other.p_value;
		Release(); p_refferenceCounter = arg_other.p_refferenceCounter;
		Increase();
		return *this;
	}
	template<typename S>
	this_type& operator=(const Value_ptr<S>& arg_other) noexcept {
		if constexpr (std::is_void_v<S>) {
			p_value = reinterpret_cast<T*>(arg_other.p_value);
		}
		else {
			p_value = arg_other.p_value;
		}
		Release(); p_refferenceCounter = arg_other.p_refferenceCounter;
		Increase();
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
		Release(); 
		p_refferenceCounter = arg_other.p_refferenceCounter;
		Increase();
		return *this;
	}
	this_type& operator=(std::nullptr_t) noexcept {
		p_value =nullptr;
		Release();
		p_refferenceCounter = nullptr;
		return *this;
	}
	bool operator==(const this_type& arg_other)const noexcept {
		return p_value == arg_other.p_value;
	}
	operator Value_weak_ptr<void> ()const {
		auto output = Value_weak_ptr<void>();
		output.p_value=p_value;
		output.p_refferenceCounter = p_refferenceCounter; 
		output.Increase();
		return output;
	}
	[[nodiscard]] Value_ptr<T> lock() const noexcept{
		if (p_value&& p_refferenceCounter->use_count()) {
			return Value_ptr<T>(*this);
		}
		else {
			return nullptr;
		}
	}
private:

	inline void Release() {
		if (p_refferenceCounter) {
			p_refferenceCounter->release_weak();
		}
	}
	inline void Increase() {
		if (p_refferenceCounter) {
			p_refferenceCounter->_Increase_weak();
		}
	}
	pointer p_value=nullptr;
	refCounter_type* p_refferenceCounter=nullptr;
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

	inline constexpr Value_ptr()noexcept :p_value(nullptr), p_refferenceCounter(nullptr) {}
	inline constexpr Value_ptr(std::nullptr_t)noexcept :p_value(nullptr), p_refferenceCounter(nullptr) {}
	inline Value_ptr(const this_type& arg_s) : p_value(arg_s.p_value){
		p_refferenceCounter = arg_s.p_refferenceCounter;
		Increase();
	}
	template<typename S, std::enable_if_t<std::is_convertible_v<S*, pointer>, std::int32_t> = 0>
	inline Value_ptr(const Value_ptr<S, refCounter_type>& arg_s) : p_value(arg_s.p_value) {
		p_refferenceCounter = arg_s.p_refferenceCounter;
		Increase();
	}
	template<typename S>
	inline explicit Value_ptr(const S& arg_v) :Value_ptr(refCounter_type::Alloc::allocate(arg_v)){	}
	template <class U>
	Value_ptr(const Value_ptr<U>& arg_other, element_type* arg_p) noexcept {
		p_refferenceCounter = arg_other.p_refferenceCounter;
		Increase();
		p_value = arg_p;
	}
	inline Value_ptr(T* arg_p, RefferenceObject* arg_refObj) :p_value(arg_p), p_refferenceCounter(arg_refObj) { Increase(); }
	template<typename S>
	inline explicit Value_ptr(S* arg_p) :p_value(arg_p) {
		p_refferenceCounter = ButiMemorySystem::Allocator::allocate<RefferenceObject>(arg_p, SmartPtrDetail::TypeStorage::TypeStorageSpecifier());
		_SetWeakPtrEnableValue(arg_p);
	}
	template<typename S>
	inline explicit Value_ptr(S* arg_p, [[maybe_unused]] const SmartPtrDetail::MemberPtrNotify) :p_value(arg_p) {
		p_refferenceCounter = ButiMemorySystem::Allocator::allocate<RefferenceObject>(arg_p, SmartPtrDetail::TypeStorage::MemberTypeStorageSpecifier());
	}
	template<typename S, typename D>
	inline explicit Value_ptr(S* arg_p, D arg_d) :p_value(arg_p){
		p_refferenceCounter = ButiMemorySystem::Allocator::allocate<RefferenceObject>(arg_p, arg_d);
		_SetWeakPtrEnableValue(arg_p);
	}

	inline constexpr Value_ptr(const Value_weak_ptr<T>& arg_vwp_other)noexcept {
		ConstructFromWeak(arg_vwp_other);
	}
	template<typename S>
	inline constexpr Value_ptr(const Value_weak_ptr<S>& arg_vwp_other)noexcept {
		ConstructFromWeak(arg_vwp_other);
	}

	~Value_ptr() {
		Release();
	}
	inline this_type CreateSameTypeValuePtr(void* arg_p_src) const{
		return this_type(arg_p_src, p_refferenceCounter->CreateSameType(arg_p_src));
	}

	inline void reset() { this_type().swap(*this); }
	inline void reset(const this_type& arg_s) { this_type(arg_s).swap(*this); }
	template<class S> void reset(S* arg_p) { this_type(arg_p).swap(*this); }
	template<class S, class D>
	inline void reset(S* arg_p, D arg_d) { this_type(arg_p, arg_d).swap(*this); }
	inline void swap(this_type& arg_other) {
		if (!arg_other&&!(*this)) {
			return;
		}
		std::swap(p_value, arg_other.p_value);
		if (p_refferenceCounter&& arg_other.p_refferenceCounter) {
			p_refferenceCounter->swap(*arg_other.p_refferenceCounter);
		}
	}

	inline this_type& operator=(const this_type& arg_s) {
		p_value = arg_s.p_value;
		Release();
		p_refferenceCounter = arg_s.p_refferenceCounter;
		Increase();
		return *this;
	}
	inline this_type& operator=(const std::nullptr_t) {
		p_value = nullptr;
		Release();
		p_refferenceCounter = nullptr;
		return *this;
	}
	template<class S, std::enable_if_t<std::is_convertible_v<S*, pointer>, std::int32_t> = 0>
	inline this_type& operator=(const Value_ptr<S, refCounter_type>& arg_s) {
		p_value = arg_s.p_value;
		Release();
		p_refferenceCounter = arg_s.p_refferenceCounter;
		Increase();
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
		return p_refferenceCounter->GetTypeStorage()->ToString();
	}
	Value_ptr<T> Clone()const {
		auto typeStorage = p_refferenceCounter->GetTypeStorage()->Clone();
		return Value_ptr<T>(reinterpret_cast<pointer>(typeStorage->ptr()), typeStorage);
	}
	/// <summary>
	/// データの直接書き込み
	/// </summary>
	/// <param name="arg_src">書き込み元のポインタ、型が同一と信頼</param>
	inline void Write(const void* arg_src) {
		p_refferenceCounter->Write(arg_src);
	}
	template<typename CastType>
	inline Value_ptr<CastType> StaticCast()const {
		auto typeStorage = p_refferenceCounter->GetTypeStorage()->StaticCast<T, CastType, refCounter_type::Alloc>();
		return Value_ptr<CastType>(reinterpret_cast<CastType*>(typeStorage->ptr()), typeStorage);
	}
	template<typename OwnType, typename CastType>
	inline Value_ptr<CastType> StaticCast()const {
		auto typeStorage = p_refferenceCounter->GetTypeStorage()->StaticCast<OwnType, CastType, refCounter_type::Alloc>();
		return Value_ptr<CastType>(reinterpret_cast<CastType*>(typeStorage->ptr()), typeStorage);
	}

	inline reference operator*() { if constexpr (std::is_same_v<element_type, void>) {} else { return *get(); } }
	inline const_reference operator*()const { if constexpr (std::is_same_v<element_type, void>) {} else { return *get(); } }
	//inline pointer operator->() { return get(); }
	inline pointer operator->()const {
#ifdef _EDITORBUILD
		if (!p_value) {
			throw std::exception();
		}
#endif // _EDITORBUILD
		return get(); }

	inline pointer get()const { return 	p_value; }
	template<typename RetType>
	inline RetType* get() { return reinterpret_cast<RetType*>(p_value); }
	template<typename RetType>
	inline const RetType* get()const { return reinterpret_cast<RetType*>(p_value); }

	inline bool unique()const { return p_refferenceCounter->unique(); }
	inline std::uint64_t use_count()const { return p_refferenceCounter->use_count(); }

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
		return p_refferenceCounter->ShowGUI(arg_label);
	}

	inline Value_ptr<IValuePtrRestoreObject, refCounter_type> GetRestoreObject() const {
		Value_ptr<IValuePtrRestoreObject, refCounter_type> output;
		p_refferenceCounter->GetRestoreObject(output);
		return output;
	}

private:
	inline bool ConstructFromWeak(const Value_weak_ptr<T>& arg_vwp_other) noexcept {
		if (arg_vwp_other.p_value && arg_vwp_other.p_refferenceCounter->use_count()) {
			p_value = arg_vwp_other.p_value;
			Release();
			p_refferenceCounter = arg_vwp_other.p_refferenceCounter;
			p_refferenceCounter->_Increase();
			return true;
		}

		return false;
	}
	template<typename S>
	inline bool ConstructFromWeak(const Value_weak_ptr<S>& arg_vwp_other) noexcept {
		if (arg_vwp_other.p_value && arg_vwp_other.p_refferenceCounter->use_count()) {
			p_value = arg_vwp_other.p_value;
			Release();
			p_refferenceCounter = arg_vwp_other.p_refferenceCounter;
			p_refferenceCounter->_Increase();
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

	inline void Release() {
		if (p_refferenceCounter) {
			if (p_refferenceCounter->release()) {
				p_refferenceCounter = nullptr;
			}
		}
	}
	inline void Increase() {
		if (p_refferenceCounter) {
			p_refferenceCounter -> _Increase();
		}
	}
	template<typename S, typename R> friend class Value_ptr;
	template<typename S, typename R> friend class Value_weak_ptr;
	pointer p_value=nullptr;
	refCounter_type* p_refferenceCounter=nullptr;
};


template<class T, class U, class R>
inline bool operator==(const Value_ptr<T, R>& arg_lhs, const Value_ptr<U, R>& arg_rhs) {
	return arg_lhs.get() == arg_rhs.get();
}
template<class T, class U, class R>
inline bool operator!=(const Value_ptr<T, R>& arg_lhs, const Value_ptr<U, R>& arg_rhs) {
	return arg_lhs.get() != arg_rhs.get();
}
class IEnable_value_from_this{};

template <typename T>
class enable_value_from_this:public IEnable_value_from_this {
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

	virtual ~enable_value_from_this() = default;

	template<typename S, typename R> friend class Value_ptr;
private:
	Value_weak_ptr<T> weak_ptr = Value_weak_ptr<T>();
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
		ARCHIVE_BUTI(data);
	}
private:
	T data;
};
template <typename Allocator>
inline void SmartPtrDetail::RefferenceCounter::RefferenceCounter<Allocator>::GetRestoreObject(Value_ptr<IValuePtrRestoreObject>& arg_ref_vlp)const {
	p_typeStorage->GetRestoreObject(arg_ref_vlp);
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
