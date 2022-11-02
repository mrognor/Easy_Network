#include <iostream>
#include <vector>
#include <set>
#include "EN_Functions.h"

int main()
{
    std::vector<int> TestVector = { 10, 1, 1, 1, 1 };

    EN::EN_ParallelFor p;
    p(0, (int)TestVector.size(), 2, [TestVector](int n)
        {
            EN::Delay(TestVector[n] * 1000);
            std::cout << "Lambda thread: " << std::this_thread::get_id() << " Iteration: " << n << std::endl;
        });
    
    p(TestVector.begin(), TestVector.end(), 2, [](std::vector<int>::iterator n)
        {
            EN::Delay(*n * 1000);
            std::cout << "Lambda iterator thread: " << std::this_thread::get_id() << " Element: " << *n << std::endl;
        });

    std::set<int> TestSet = { 10, 3, 1, 2, 7 };
    p(TestSet.begin(), TestSet.end(), 2, [](std::set<int>::iterator n)
        {
            EN::Delay(*n * 1000);
            std::cout << "Lambda iterator set thread: " << std::this_thread::get_id() << " Element: " << *n << std::endl;
        });
}
