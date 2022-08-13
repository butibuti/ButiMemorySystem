#include <iostream>
#include"ButiPtr.h"
#define _CRTDBG_MAP_ALLOC
class Test {
public:
    ButiEngine::Value_ptr<Test> m_vlp_test;
    std::int32_t m_data = 128;
};
class Test_Derived :public Test{
public:
};

std::int32_t main()
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    ButiEngine::GarbageCollection::GarbageCollector::Start();
    {
        auto vlp_test = ButiEngine::make_value<Test>();
        auto vlp_test_derive = ButiEngine::make_value<Test_Derived>();
        vlp_test->m_vlp_test = vlp_test_derive;
        vlp_test_derive->m_vlp_test = vlp_test;

        vlp_test = nullptr;
        vlp_test_derive = nullptr;
    }

    ButiEngine::GarbageCollection::GarbageCollector::GetInstance()->Collect();
    ButiEngine::GarbageCollection::GarbageCollector::End();
    return 0;
}
