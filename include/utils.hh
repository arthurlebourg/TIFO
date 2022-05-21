#pragma once
#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>

template <typename T, typename G>
T saturate_cast(G value, T min, T max);

#include "utils.hxx"
