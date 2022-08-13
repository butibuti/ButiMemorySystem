#ifndef BUTI_GC_H
#define BUTI_GC_H
#include"ButiList.h"
#include<deque>
#include<mutex>
namespace ButiEngine {
namespace SmartPtrDetail {
namespace RefferenceCounter {
class IRCImpl;
}
}

namespace GarbageCollection {
using rcImpl = SmartPtrDetail::RefferenceCounter::IRCImpl;
class GarbageCollector {
public:
	BUTIMEMORYSYSTEM_API void Collect();
	BUTIMEMORYSYSTEM_API void AddObject(const void* arg_objectPtr, rcImpl* arg_p_object);
	BUTIMEMORYSYSTEM_API void RegistChild(const void* arg_objectPtr, rcImpl* arg_p_object);
	BUTIMEMORYSYSTEM_API void PushPurpleObject(rcImpl* arg_p_object);
	BUTIMEMORYSYSTEM_API void RemoveObject(const rcImpl* arg_p_object);

	BUTIMEMORYSYSTEM_API static void Start();
	BUTIMEMORYSYSTEM_API static void End();
	BUTIMEMORYSYSTEM_API static GarbageCollector* GetInstance();
	const std::mutex& GetMutex()const;
	class Impl;
private:
	Impl* m_p_impl;
	GarbageCollector();
	~GarbageCollector();
};
}
}

#endif // !BUTI_GC_H
