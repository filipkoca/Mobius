#include "make_undo_benchmark.h"

#include <iostream>


int main()
{
    std::cout << "========================================\n";
    std::cout << "          MOBIUS BENCHMARKS\n";
    std::cout << "========================================\n\n";

    runMakeUndoBenchmark();

    std::cout << "\n========================================\n";
    std::cout << "        BENCHMARKS FINISHED\n";
    std::cout << "========================================\n";

    return 0;
}