#include"stdafx.h"
#include"GarbageCollection.h"
#include"RefferenceCounter.h"
#include<map>
#include<unordered_map>
namespace ButiEngine {
namespace GarbageCollection {
class ControlSet;
class ChildStorage {

public:
	ChildStorage(GarbageCollector* arg_p_gc, rcImpl* arg_p_rc);
	ChildStorage(ChildStorage&& arg_other) noexcept;
	~ChildStorage() {}
	ChildStorage& operator=(const ChildStorage&) = delete;

	void RegistChild(rcImpl* arg_p_rcImpl, const std::uint32_t arg_index, const std::unique_lock<std::mutex>& arg_locker);

	inline rcImpl* GetObject()const { return m_p_rc; }
	std::map<std::uint32_t, rcImpl*>& GetObjects(){ return m_childObjects; }
private:
	GarbageCollector* m_p_gc;
	rcImpl* m_p_rc;
	std::map<std::uint32_t, rcImpl*> m_childObjects;
};


class GarbageCollector::Impl {
public:
	Impl(GarbageCollector* arg_p_gc);
	~Impl();
	void Collect();
	std::unique_lock<std::mutex> Lock() { return std::unique_lock<std::mutex>(m_mtx_gc); }
	void AddObject(const void* arg_objectPtr, rcImpl* arg_p_object, const std::unique_lock<std::mutex>& arg_locker);
	void RemoveObject(const rcImpl* arg_p_object, const std::unique_lock<std::mutex>& arg_locker);
	bool RegistChild(const void* arg_objectPtr, rcImpl* arg_p_object, const std::unique_lock<std::mutex>& arg_locker);
	ControlSet& GetControlSet() { return *m_p_controlSet; }
	inline const std::mutex& GetMutex()const { return m_mtx_gc; }
	ChildStorage& GetChild(rcImpl* arg_p_parentObjectobject) {
		return m_storages.at(arg_p_parentObjectobject->GetPtr());
	}
private:
	using storage_container_type = std::pmr::map<const void*, ChildStorage, std::greater<>>;
	using storage_iterator_type = typename storage_container_type::iterator;

	using object_container_type = std::pmr::unordered_map<rcImpl*, storage_iterator_type>;
	GarbageCollector* m_p_gc;
	storage_container_type m_storages;
	object_container_type m_rootObjects;
	object_container_type m_childObjects;
	ControlSet* m_p_controlSet;
	std::mutex m_mtx_gc;
};


ChildStorage::ChildStorage(GarbageCollector* arg_p_gc,rcImpl* arg_p_rc):m_p_gc(arg_p_gc),m_p_rc(arg_p_rc)
{}

ChildStorage::ChildStorage(ChildStorage&& arg_other) noexcept :m_p_gc(arg_other.m_p_gc), m_p_rc(arg_other.m_p_rc)
{}

void ChildStorage::RegistChild(rcImpl* arg_p_rcImpl, const std::uint32_t arg_index, const std::unique_lock<std::mutex>&arg_locker)
{
	assert(arg_locker&& arg_locker.mutex() == &m_p_gc->GetMutex());
	if (arg_p_rcImpl) {
		if (m_childObjects.count(arg_index)) {
			m_childObjects.at(arg_index) = arg_p_rcImpl;
		}
		else {
			m_childObjects.emplace(arg_index, arg_p_rcImpl);
		}
	}
	else {
		if (m_childObjects.count(arg_index)) {
			m_childObjects.erase(arg_index);
		}
	}
}

void DecChild(ChildStorage& arg_childStorage, GarbageCollector::Impl* arg_p_gc) {
	for (auto& child : arg_childStorage.GetObjects()) {
		if (child.second->GetColor() == SmartPtrDetail::RefferenceCounter::ObjectColor::Gray)continue;
		child.second->dec();
		child.second->SetColor(SmartPtrDetail::RefferenceCounter::ObjectColor::Gray);
		DecChild(arg_p_gc->GetChild(child.second), arg_p_gc);
	}
}
void IncChild(ChildStorage& arg_childStorage, GarbageCollector::Impl* arg_p_gc) {
	for (auto& child : arg_childStorage.GetObjects()) {
		if (child.second->GetColor() == SmartPtrDetail::RefferenceCounter::ObjectColor::Black)continue;
		child.second->inc();
		child.second->SetColor(SmartPtrDetail::RefferenceCounter::ObjectColor::Black);
		IncChild(arg_p_gc->GetChild(child.second), arg_p_gc);
	}
}

constexpr std::int32_t controlSetSize = 512;
class ControlSet {
public:
	ControlSet(GarbageCollector::Impl* arg_p_gcImpl):m_p_gcImpl(arg_p_gcImpl){}
	void PushPurpleObject(rcImpl* arg_p_rc) {
		m_vec_p_rc.push_back(arg_p_rc);
		if (m_vec_p_rc.size() > 512) {
			MarkGray();
		}
	}
	void MarkGray() {
		for (auto p_rc : m_vec_p_rc) {
			DecChild(m_p_gcImpl->GetChild(p_rc), m_p_gcImpl);
		}
		for (auto p_rc : m_vec_p_rc) {
			if (p_rc->count()) {
				IncChild(m_p_gcImpl->GetChild(p_rc), m_p_gcImpl);
			}
		}
		//ここで回収用コード
		m_vec_p_rc.clear();
	}
private:
	GarbageCollector::Impl* m_p_gcImpl;
	std::vector<rcImpl*> m_vec_p_rc;
};

}
}
ButiEngine::GarbageCollection::GarbageCollector::Impl::Impl(GarbageCollector* arg_p_gc):m_p_gc(arg_p_gc)
{
	m_p_controlSet = new ControlSet(this);
}

ButiEngine::GarbageCollection::GarbageCollector::Impl::~Impl()
{
	delete m_p_controlSet;
}

void ButiEngine::GarbageCollection::GarbageCollector::Impl::Collect()
{
	m_p_controlSet->MarkGray();
}

void ButiEngine::GarbageCollection::GarbageCollector::Collect()
{
	m_p_impl->Collect();
}

void ButiEngine::GarbageCollection::GarbageCollector::AddObject(const void* arg_objectPtr,rcImpl* arg_p_object)
{
	auto lock = m_p_impl->Lock();
	m_p_impl->AddObject(arg_objectPtr, arg_p_object, lock);
}

void ButiEngine::GarbageCollection::GarbageCollector::RegistChild(const void* arg_objectPtr, rcImpl* arg_p_object)
{
	auto lock = m_p_impl->Lock();
	m_p_impl->RegistChild(arg_objectPtr, arg_p_object, lock);
}

void ButiEngine::GarbageCollection::GarbageCollector::PushPurpleObject(rcImpl* arg_p_object)
{
	return m_p_impl->GetControlSet().PushPurpleObject(arg_p_object);
}

void ButiEngine::GarbageCollection::GarbageCollector::RemoveObject(const rcImpl* arg_p_object)
{
	auto lock = m_p_impl->Lock();
	m_p_impl->RemoveObject(arg_p_object, lock);
}


ButiEngine::GarbageCollection::GarbageCollector* g_p_gcInstance = nullptr;
ButiEngine::GarbageCollection::GarbageCollector::GarbageCollector()
{
	m_p_impl = new Impl(this);
}
ButiEngine::GarbageCollection::GarbageCollector::~GarbageCollector()
{
	delete m_p_impl;
}
void ButiEngine::GarbageCollection::GarbageCollector::Start()
{
	if (!g_p_gcInstance)g_p_gcInstance = new GarbageCollector();
}
void ButiEngine::GarbageCollection::GarbageCollector::End()
{
	if (g_p_gcInstance)delete g_p_gcInstance;
}
ButiEngine::GarbageCollection::GarbageCollector* ButiEngine::GarbageCollection::GarbageCollector::GetInstance()
{
	return g_p_gcInstance;
}

const std::mutex& ButiEngine::GarbageCollection::GarbageCollector::GetMutex() const
{
	return m_p_impl->GetMutex();
}

void ButiEngine::GarbageCollection::GarbageCollector::Impl::AddObject(const void* arg_objectPtr,rcImpl* arg_p_object, const std::unique_lock<std::mutex>& arg_locker)
{
	m_storages.emplace(arg_p_object->GetPtr(), ChildStorage(m_p_gc,arg_p_object));
	RegistChild(arg_objectPtr, arg_p_object, arg_locker);
}

void ButiEngine::GarbageCollection::GarbageCollector::Impl::RemoveObject(const rcImpl* arg_p_object, const std::unique_lock<std::mutex>& arg_locker)
{
	m_storages.erase(arg_p_object->GetPtr());
}

bool ButiEngine::GarbageCollection::GarbageCollector::Impl::RegistChild(const void* arg_objectPtr, rcImpl* arg_p_object, const std::unique_lock<std::mutex>& arg_locker)
{
	bool isChild = false;
	auto itr= m_storages.lower_bound(arg_objectPtr);
	if (itr != m_storages.end()) {
		auto dataEnd = static_cast<const char*>( itr->first )+ itr->second.GetObject()->GetSize();
		if (arg_objectPtr >= itr->first && arg_objectPtr < dataEnd) {
			itr->second.RegistChild(arg_p_object, reinterpret_cast<std::uint64_t>(arg_objectPtr)- reinterpret_cast<std::uint64_t>(itr->first),arg_locker);
			isChild = true;
		}
	}
	return isChild;
}
