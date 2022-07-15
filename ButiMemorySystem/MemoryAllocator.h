#pragma once
#ifndef MEMORYALLOCATOR_H

#include <cstdint>
#ifdef BUTIMEMORYSYSTEM_EXPORTS
#define BUTIMEMORYSYSTEM_API __declspec(dllexport)
#else
#define BUTIMEMORYSYSTEM_API __declspec(dllimport)
#endif
namespace ButiMemorySystem {

extern class Allocator {
public:
	BUTIMEMORYSYSTEM_API static void* allocate(std::uint32_t arg_size);
	BUTIMEMORYSYSTEM_API static void* allocate_large(std::uint64_t arg_size);
	BUTIMEMORYSYSTEM_API static void* allocate_customAlign(std::uint64_t arg_size, std::int32_t alignment);
	BUTIMEMORYSYSTEM_API static void deallocate(void* arg_ptr);
	BUTIMEMORYSYSTEM_API static void deallocate_bt(void* arg_ptr);

	template <typename T,typename... Args>
	static inline constexpr T* allocate(Args&&... args) {
		auto output =reinterpret_cast<T*>( allocate(sizeof(T)));
		new (output) T(args...);
		return output;
	}
	template<typename T>
	static inline constexpr void deallocate(T* arg_ptr) {
		arg_ptr->~T();
		deallocate(reinterpret_cast<void*>(arg_ptr));
	}
	template<typename T>
	static inline constexpr void deallocate(const T* arg_ptr) {
		arg_ptr->~T();
		deallocate(const_cast<void*>(reinterpret_cast<const void*>(arg_ptr)));
	}
private:

};
}
#endif // !MEMORYALLOCATOR_H
