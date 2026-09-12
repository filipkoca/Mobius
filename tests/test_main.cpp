#include "core_tests.h"
#include "make_undo_tests.h"
#include "attacks_tests.h"

#include <iostream>


int main()
{
    std::cout << "============================\n";
    std::cout << "       MOBIUS TESTS\n";
    std::cout << "============================\n";

    runCoreTests();
    runMakeUndoTests();
    runAttackTests();

    std::cout << "\n============================\n";
    std::cout << "     ALL TESTS PASSED\n";
    std::cout << "============================\n";

    return 0;
}