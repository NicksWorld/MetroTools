#include "mycommon.h"

static std::mt19937 sRandomEngine;

int RandomIntRange(const int left, const int right) {
    std::uniform_int_distribution<int> probability(left, right);
    return probability(sRandomEngine);
}

float RandomFloatRange(const float left, const float right) {
    std::uniform_real_distribution<float> probability(left, right);
    return probability(sRandomEngine);
}

float RandomFloat01() {
    return RandomFloatRange(0.0f, 1.0f);
}
