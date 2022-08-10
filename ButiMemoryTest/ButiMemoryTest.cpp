#include <iostream>
#include"ButiPtr.h"
std::int32_t main()
{
    auto vlp_test = ButiEngine::make_value<std::int32_t>();
    ButiEngine::Value_weak_ptr vwp_test = vlp_test;
    if (vwp_test.lock()) {
        std::cout << *vwp_test.lock();
    }
    vlp_test = nullptr;
    if(vwp_test.lock()) {
        std::cout << *vwp_test.lock();
    }
    return 0;
}
