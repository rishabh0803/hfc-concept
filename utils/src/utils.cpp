#include <vector>
#include <cstdlib>
#include <cstdint>
#include <ctime>

using namespace std;

void fill_random(vector<uint32_t> &v, size_t fill_count, uint32_t min = 0, uint32_t max = RAND_MAX)
{
    srand(time(NULL)); // setting up seed value

    if (v.size() != fill_count)
        v.resize(fill_count);

    for (int i = 0; i < fill_count; ++i)
    {
        v[i] = rand() % (max - min) + min;
    }
}