#ifndef RNG
#define RNG

#include "../pch.h"

class RandomGenerator {
public:
    // Get the singleton instance
    static RandomGenerator& getInstance() {
        static RandomGenerator instance;
        return instance;
    }

    // Delete copy/move operations
    RandomGenerator(const RandomGenerator&) = delete;
    RandomGenerator& operator=(const RandomGenerator&) = delete;
    RandomGenerator(RandomGenerator&&) = delete;
    RandomGenerator& operator=(RandomGenerator&&) = delete;

    // Generate a random integer in [min, max]
    int getInt(int min, int max) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(rng);
    }

    // Generate a random float in [min, max]
    float getFloat(float min, float max) {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(rng);
    }

    // Generate a random double in [min, max]
    double getDouble(double min, double max) {
        std::uniform_real_distribution<double> dist(min, max);
        return dist(rng);
    }

private:
    // Private constructor
    RandomGenerator() : rng(std::random_device{}()) {}

    std::mt19937 rng;
};

#endif
