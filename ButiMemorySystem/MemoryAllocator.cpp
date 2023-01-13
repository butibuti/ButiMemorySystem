#include"stdafx.h"
#include<mutex>
#include"MemoryAllocator.h"
#include<iostream>
#include<unordered_map>
#include<cassert>
std::mutex mtx_alloc;

constexpr std::uint64_t fieldSize = 0x4fffffff;

struct Field {
	Field() {
		m_memory = reinterpret_cast<char*>(malloc(fieldSize));
	}
	~Field() {
		free(m_memory);
	}
	char* m_memory;
	std::uint64_t m_index=0;
	std::unordered_map<void*, std::uint32_t> m_map_use_ptr;
	std::unordered_map<std::uint32_t,std::vector<void*>> m_map_release_ptr;
	void* GetPtr(const std::uint32_t arg_size)
	{

		if (!m_map_release_ptr.count(arg_size)) {
			m_map_release_ptr.emplace(arg_size, std::vector<void*>());
		}else
		if (m_map_release_ptr.at(arg_size).size()) {
			auto output = *m_map_release_ptr.at(arg_size).rbegin();
			m_map_release_ptr.at(arg_size).pop_back();
			m_map_use_ptr.emplace(output, arg_size);
			return output;
		}
		assert(m_index < fieldSize);
		m_map_use_ptr.emplace(&m_memory[m_index], arg_size);
		auto output = &m_memory[m_index];
		m_index += arg_size;
		return output;
	}
	void Release(void* arg_ptr) {
		m_map_release_ptr.at(m_map_use_ptr.at(arg_ptr)).push_back(arg_ptr);
		m_map_use_ptr.erase(arg_ptr);

	}
};

Field g_field;

void* ButiMemorySystem::Allocator::allocate(std::uint32_t arg_size)
{
	std::lock_guard lock(mtx_alloc);
	return g_field.GetPtr((arg_size + 0xf) & ~0xf);
}

void* ButiMemorySystem::Allocator::allocateLarge(std::uint64_t arg_size)
{
	std::lock_guard lock(mtx_alloc);
	return g_field.GetPtr((arg_size + 0xf) & ~0xf);
}

void* ButiMemorySystem::Allocator::allocateArray(std::uint32_t arg_size)
{
	std::lock_guard lock(mtx_alloc);
	return	g_field.GetPtr((arg_size + 0xf) & ~0xf);
}

void* ButiMemorySystem::Allocator::allocateCustomAlign(std::uint64_t arg_size,std::int32_t alignment)
{
	std::lock_guard lock(mtx_alloc);
	return g_field.GetPtr((arg_size + 0xf) & ~0xf);
}

void ButiMemorySystem::Allocator::deallocate(void* arg_ptr)
{
	std::lock_guard lock(mtx_alloc);
	g_field.Release(arg_ptr);
}

void ButiMemorySystem::Allocator::deallocateBt(void* arg_ptr)
{
	deallocate(arg_ptr);
}

void ButiMemorySystem::Allocator::deallocateArray(void* arg_ptr)
{
	std::lock_guard lock(mtx_alloc);
	g_field.Release(arg_ptr);
}
