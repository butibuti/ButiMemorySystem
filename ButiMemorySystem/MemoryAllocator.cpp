#include"stdafx.h"
#include<mutex>
#include"MemoryAllocator.h"
#include<iostream>
std::mutex mtx_alloc;

void* ButiMemorySystem::Allocator::allocate(std::uint32_t arg_size)
{
	std::lock_guard lock(mtx_alloc);
	return (operator new(arg_size));
}

void* ButiMemorySystem::Allocator::allocateLarge(std::uint64_t arg_size)
{
	std::lock_guard lock(mtx_alloc);
	return (operator new(arg_size));
}

void* ButiMemorySystem::Allocator::allocateArray(std::uint32_t arg_size)
{
	std::lock_guard lock(mtx_alloc);
	return	operator new[](arg_size);
}

void* ButiMemorySystem::Allocator::allocateCustomAlign(std::uint64_t arg_size,std::int32_t alignment)
{
	std::lock_guard lock(mtx_alloc);
	return (operator new(arg_size));
}

void ButiMemorySystem::Allocator::deallocate(void* arg_ptr)
{
	std::lock_guard lock(mtx_alloc);
	operator delete(arg_ptr);
}

void ButiMemorySystem::Allocator::deallocateBt(void* arg_ptr)
{
	deallocate(arg_ptr);
}

void ButiMemorySystem::Allocator::deallocateArray(void* arg_ptr)
{
	std::lock_guard lock(mtx_alloc);
	operator delete[](arg_ptr);
}
