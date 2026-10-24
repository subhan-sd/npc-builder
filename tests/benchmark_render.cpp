#include "npc/generator.hpp"
#include <iostream>
#include <chrono>
#include <vector>
#include <numeric>
#include <algorithm>

int main() {
    std::cout << "Benchmarking NPC core rendering performance...\n";
    auto app = npc::CharacterGenerator::generate_from_seed(42, 1);
    npc::PhysicalConditionSet cond;
    npc::AnimationCommand cmd{npc::AnimationState::Idle, npc::Direction::South, 0.0f, 1.0f};

    // Warm-up
    for (int i = 0; i < 1000; ++i) {
        auto buf = npc::CharacterGenerator::render(app, cond, cmd);
        (void)buf;
    }

    constexpr int ITERATIONS = 20000;
    std::vector<double> timings_us;
    timings_us.reserve(ITERATIONS);

    for (int i = 0; i < ITERATIONS; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        auto buf = npc::CharacterGenerator::render(app, cond, cmd);
        auto end = std::chrono::high_resolution_clock::now();
        (void)buf;

        double elapsed_us = std::chrono::duration<double, std::micro>(end - start).count();
        timings_us.push_back(elapsed_us);
    }

    std::sort(timings_us.begin(), timings_us.end());
    double sum = std::accumulate(timings_us.begin(), timings_us.end(), 0.0);
    double mean = sum / ITERATIONS;
    double median = timings_us[ITERATIONS / 2];
    double p95 = timings_us[static_cast<size_t>(ITERATIONS * 0.95)];
    double p99 = timings_us[static_cast<size_t>(ITERATIONS * 0.99)];

    std::cout << "Results for " << ITERATIONS << " renders:\n"
              << "  Mean:   " << mean << " us\n"
              << "  Median: " << median << " us\n"
              << "  P95:    " << p95 << " us\n"
              << "  P99:    " << p99 << " us\n";

    return 0;
}
