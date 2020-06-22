#include <vector>

template <class myClass>
std::vector<myClass> arrayToVec(myClass *arr, int size)
{
    std::vector<myClass> dest(arr, arr + size);
    return dest;
}