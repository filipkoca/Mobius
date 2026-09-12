#include "attack_detector_benchmark.h"
#include "make_undo_benchmark.h"
#include "sliding_attacks_benchmark.h"

#include <iostream>


int main()
{
    std::cout << "========================================\n";
    std::cout << "          MOBIUS BENCHMARKS\n";
    std::cout << "========================================\n\n";

    runMakeUndoBenchmark();
    std::cout << '\n';

    runSlidingAttacksBenchmark();
    std::cout << '\n';

    runAttackDetectorBenchmark();

    std::cout << "\n========================================\n";
    std::cout << "        BENCHMARKS FINISHED\n";
    std::cout << "========================================\n";

    return 0;
}
