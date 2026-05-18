#pragma once
#include <vector>
#include <cstdint> // instead of stdint.h
#include <cstdlib>

using namespace std;

void fill_random(vector<uint32_t> &v, size_t fill_count, uint32_t min = 0, uint32_t max = RAND_MAX);
