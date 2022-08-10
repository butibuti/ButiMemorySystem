#ifndef BUTI_RC_H
#define BUTI_RC_H
#include<string>
#include<type_traits>
#include <cassert>
#include"MemoryAllocator.h"
namespace ButiEngine {
namespace SmartPtrDetail {
namespace RefferenceCounter {
template<typename Allocator = ButiMemorySystem::Allocator>
class RefferenceCounter;
template<typename Allocator = ButiMemorySystem::Allocator>
class RefferenceCounter_weak;
}
}
#ifndef BUTIPTR_FORWARD_H
#define BUTIPTR_FORWARD_H
template<typename T, typename RefferenceObject = SmartPtrDetail::RefferenceCounter::RefferenceCounter<ButiMemorySystem::Allocator>>
class Value_ptr;
#endif
/// <summary>
/// Value_ptr<void>を保存するために経由するインターフェース
/// </summary>
class IValuePtrRestoreObject {
public:
	virtual void RestoreValue(Value_ptr<void>&)const = 0;
	template<class Archive>	void serialize(Archive& archive) {}
};


template<typename T>
constexpr bool False() { return false; }
template <class T, class = void>
struct _Can_enable_value : std::false_type {}; // detect unambiguous and accessible inheritance from enable_shared_from_this

template <class T>
struct _Can_enable_value<T, std::void_t<typename T::_Evft_type>>
	: std::is_convertible<std::remove_cv_t<T>*, typename T::_Evft_type*>::type {};
namespace SmartPtrDetail {

template <typename T, typename Allocator>
T* CloneValue(const T* arg_p_v) {
	if constexpr (std::is_copy_constructible_v<T>) {
		auto output = Allocator::template allocate<T>(*arg_p_v);
		return output;
	}
	else if constexpr (std::is_copy_assignable_v<T> && std::is_default_constructible_v<T>) {
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

template<typename T>struct type_to_constRef_impl { using reftype = const T&; };
template <> struct type_to_constRef_impl <void> { using reftype = void; };

template <typename T> using type_to_ref = typename type_to_ref_impl<T>::reftype;
template <typename T> using type_to_constRef = typename type_to_constRef_impl<T>::reftype;

struct FunctionCheck_std_to_string {
	template <typename T>
	auto operator()(T&& arg_check)->decltype(std::to_string(arg_check));
};

#ifdef BUTIGUI_H
struct FunctionCheck_GUI_Input {
	template <typename T>
	auto operator()(T&& arg_check) -> decltype(GUI::Input("", arg_check));
};
#endif
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
	virtual void Dispose() = 0;
	virtual ITypeStorage* Clone() const = 0;
	virtual ITypeStorage* CreateSameTypeStorage(void* arg_src)const = 0;
	virtual std::string ToString() const = 0;
	virtual void* ptr() = 0;
	virtual const void* ptr() const = 0;
	template <typename OwnType, typename CastType, typename Allocator>
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
#ifdef BUTIGUI_H
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
			GUI::Text(arg_label + u8":対応していない型です");
		}
#endif // BUTIGUI_H
		return false;
	}
	template<typename T>
	inline std::string ToString_(const T& arg_value) const {
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
template <typename T, typename Allocator>
class TypeStorage : public ITypeStorage {
	friend typename Allocator;
public:
	using this_type = TypeStorage<T, Allocator>;
	TypeStorage(T* arg_p) :p_value(arg_p) {}
	inline void Dispose()override { Allocator::deallocate(p_value); }
	inline void Destroy()override { Allocator::deallocate(this); }

	inline ITypeStorage* Clone()const override {
		return Allocator::template allocate<this_type>(CloneValue<T, Allocator>(p_value));
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
	static this_type* get(T* arg_p) { return Allocator::template allocate<this_type>(arg_p); }
	inline bool ShowGUI(const std::string& arg_label) {
		return ShowGUI_(arg_label, *p_value);
	}
	inline void GetRestoreObject(Value_ptr<IValuePtrRestoreObject>& arg_ref_vlp)const override;
private:
	template<typename T, typename Allocator> friend class MemberTypeStorage;
	this_type& operator=(const this_type&) { return *this; }
	TypeStorage(const this_type& arg_rhs) : p_value(arg_rhs.p_value) {}
	TypeStorage() = delete;
	T* p_value;
};
template <typename T, typename Allocator>
class MemberTypeStorage : public ITypeStorage {
	friend typename Allocator;
public:
	using this_type = MemberTypeStorage<T, Allocator>;
	inline void Dispose()override {  }
	inline void Destroy()override { Allocator::deallocate(this); }
	//値のクローン作成(今後メモリ確保は変更)
	inline ITypeStorage* Clone()const override {
		return Allocator::template allocate<TypeStorage<T>>(CloneValue<T, Allocator>(p_value));
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
	return Allocator::template allocate<TypeStorage<CastType>>(Allocator::template allocate<CastType>(static_cast<CastType>(*reinterpret_cast<const OwnType*>(ptr()))));
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
	struct impl;
	friend class RefferenceCounter_weak<Allocator>;
	using this_type = RefferenceCounter<Allocator>;
public:
	using Alloc = Allocator;
	constexpr RefferenceCounter()noexcept :p_impl(nullptr) {}
	constexpr RefferenceCounter(impl* arg_p_impl) : p_impl(arg_p_impl) {}
	RefferenceCounter(this_type* arg_r) : p_impl(arg_r->p_impl) {}
	RefferenceCounter(const this_type& arg_r) :p_impl(arg_r.p_impl) { if (p_impl) p_impl->inc(); }
	template<typename S, typename D>
	explicit RefferenceCounter(S* arg_p, D arg_d) :p_impl(Allocator::template allocate<impl>(arg_p, arg_d)) {}
	template<typename S, typename D>
	explicit RefferenceCounter(S* arg_p, D* arg_p_d) :p_impl(Allocator::template allocate<impl>(arg_p, arg_p_d)) {}
	~RefferenceCounter() { release(); }
	inline void release() {
		if (p_impl && !p_impl->dec()) {
			p_impl->p_typeStorage->Dispose();
			Allocator::deallocate(p_impl);
		}
	}
	inline void Write(const void* arg_src) {
		p_impl->p_typeStorage->Write(arg_src);
	}
	inline void swap(this_type& r) { std::swap(p_impl, r.p_impl); }
	inline TypeStorage::ITypeStorage* GetTypeStorage() { return p_impl->p_typeStorage; }
	inline const TypeStorage::ITypeStorage* GetTypeStorage()const { return p_impl->p_typeStorage; }
	inline bool unique()const { return use_count() == 1; }
	inline std::uint64_t use_count()const { return p_impl ? p_impl->count() : 0; }
	inline this_type& operator=(const this_type& arg_r) {
		impl* tmp = arg_r.p_impl;
		if (tmp != p_impl) {
			if (tmp) tmp->inc();
			release();
			p_impl = tmp;
		}
		return *this;
	}
	inline this_type CreateSameType(void* arg_p_src)const {
		return this_type(p_impl->CreateSameTypeStorage(arg_p_src));
	}
	inline bool ShowGUI(const std::string& arg_label) {
		return p_impl->p_typeStorage->ShowGUI(arg_label);
	}
	inline void _Increase() {
		p_impl->inc();
	}
	inline void _Decrease() {
		p_impl->dec();
	}
	inline bool _Increase_notZero() {
		return p_impl->inc_nz();
	}
	inline bool _Decrease_notZero() {
		return p_impl->dec_nz();
	}
	inline void GetRestoreObject(Value_ptr<IValuePtrRestoreObject>& arg_ref_vlp) const;
private:
	struct impl {
		template<typename S, typename D> explicit impl(S* arg_p, D)
			:use(1), p_typeStorage(D::template type<S>::name::get(arg_p)) {}
		template<typename S, typename D> explicit impl(S* arg_p, D* arg_p_typeStorage)
			:use(1), p_typeStorage(arg_p_typeStorage) {}
		impl(TypeStorage::ITypeStorage* arg_p_typeStorage) :use(1), p_typeStorage(arg_p_typeStorage) {}
		impl(const impl& arg_c) :use(arg_c.use), p_typeStorage(arg_c.p_typeStorage) {}
		inline std::uint64_t inc() {
			return	_InterlockedIncrement64(reinterpret_cast<volatile std::int64_t*>(&use));
		}
		inline std::uint64_t dec() {
			return	_InterlockedDecrement64(reinterpret_cast<volatile std::int64_t*>(&use));
		}
		inline std::uint64_t inc_nz() {
			return	use ? inc() : false;
		}
		inline std::uint64_t dec_nz() {
			return	use ? dec() : false;
		}

		inline std::uint64_t count()const { return use; }
		~impl() { p_typeStorage->Destroy(); }
		impl* CreateSameTypeStorage(void* arg_p_src)const {
			return Allocator::template allocate< impl>(p_typeStorage->CreateSameTypeStorage(arg_p_src));
		}
		std::uint64_t use;
		TypeStorage::ITypeStorage* p_typeStorage;
	};
	impl* p_impl;
};

template<typename Allocator>
class RefferenceCounter_weak {
public:
	using BaseRefferenceCounter = RefferenceCounter<Allocator>;
	constexpr RefferenceCounter_weak(const RefferenceCounter<Allocator>& arg_refferenceCounter) noexcept :p_impl(arg_refferenceCounter.p_impl) {}
	constexpr RefferenceCounter_weak()noexcept :p_impl(nullptr) {}
	inline const RefferenceCounter_weak<Allocator>& operator=(const RefferenceCounter<Allocator>& arg_refferenceCounter) {
		p_impl = arg_refferenceCounter.p_impl;
		return *this;
	}
	inline operator RefferenceCounter<Allocator>()const {
		return RefferenceCounter<Allocator>(p_impl);
	}
	inline bool _Increase_notZero() {
		return p_impl->inc_nz();
	}
	inline bool _Decrease_notZero() {
		return p_impl->dec_nz();
	}
	inline std::uint64_t count()const { return p_impl->count(); }
private:
	RefferenceCounter<Allocator>::template impl* p_impl;
};
}
}
}
#endif // !BUTI_RC_H
