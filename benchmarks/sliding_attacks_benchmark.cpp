#include "sliding_attacks_benchmark.h"

#include "attacks.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <string_view>
#include <vector>


namespace
{
    using Clock = std::chrono::steady_clock;

    constexpr int SAMPLE_COUNT = 9;

    constexpr double TARGET_SAMPLE_SECONDS = 0.30;
    constexpr double WARMUP_SECONDS = 1.0;

    constexpr std::size_t INPUT_COUNT = 4096;
    constexpr std::size_t INPUT_MASK = INPUT_COUNT - 1;

    static_assert(
        (INPUT_COUNT & (INPUT_COUNT - 1)) == 0
    );


    struct AttackInput
    {
        Square square = 0;
        Bitboard occupancy = 0;
    };


    struct BenchmarkResult
    {
        double bestNs = 0.0;
        double medianNs = 0.0;
        double meanNs = 0.0;
        double worstNs = 0.0;

        std::uint64_t iterations = 0;
    };


    struct TimedRun
    {
        double seconds = 0.0;
        Bitboard checksum = 0;
    };


    volatile Bitboard benchmarkSink = 0;


    std::uint64_t nextRandom(std::uint64_t& state)
    {
        state += 0x9E3779B97F4A7C15ULL;

        std::uint64_t value = state;

        value =
            (value ^ (value >> 30)) *
            0xBF58476D1CE4E5B9ULL;

        value =
            (value ^ (value >> 27)) *
            0x94D049BB133111EBULL;

        return value ^ (value >> 31);
    }


    std::array<AttackInput, INPUT_COUNT> generateInputs()
    {
        std::array<AttackInput, INPUT_COUNT> inputs{};

        std::uint64_t randomState =
            0xC0FFEE1234567890ULL;

        for (
            std::size_t i = 0;
            i < INPUT_COUNT;
            ++i
        )
        {
            const Square square =
                static_cast<Square>(
                    nextRandom(randomState) & 63
                );

            const Bitboard randomA =
                nextRandom(randomState);

            const Bitboard randomB =
                nextRandom(randomState);

            const Bitboard randomC =
                nextRandom(randomState);

            const Bitboard randomD =
                nextRandom(randomState);

            Bitboard occupancy = 0;

            switch (i % 3)
            {
                case 0:
                    // Roughly 50% occupancy.
                    occupancy = randomA;
                    break;

                case 1:
                    // Roughly 25% occupancy.
                    occupancy =
                        randomA & randomB;
                    break;

                default:
                    // Roughly 44% occupancy.
                    occupancy =
                        (randomA & randomB) |
                        (randomC & randomD);
                    break;
            }

            // The attacking piece itself would normally
            // be included in Position occupancy.
            occupancy |= getBit(square);

            inputs[i].square = square;
            inputs[i].occupancy = occupancy;
        }

        return inputs;
    }


    template <typename AttackFunction>
    TimedRun runTimedBatch(
        const std::array<AttackInput, INPUT_COUNT>& inputs,
        std::uint64_t iterations,
        AttackFunction attackFunction
    )
    {
        Bitboard checksum = 0;

        std::atomic_signal_fence(
            std::memory_order_seq_cst
        );

        const auto start = Clock::now();

        for (
            std::uint64_t i = 0;
            i < iterations;
            ++i
        )
        {
            const AttackInput& input =
                inputs[
                    static_cast<std::size_t>(i) &
                    INPUT_MASK
                ];

            checksum ^=
                attackFunction(
                    input.square,
                    input.occupancy
                );
        }

        const auto end = Clock::now();

        std::atomic_signal_fence(
            std::memory_order_seq_cst
        );

        // Prevent compiler from removing the calculations.
        benchmarkSink = checksum;

        const std::chrono::duration<double> elapsed =
            end - start;

        return TimedRun{
            elapsed.count(),
            checksum
        };
    }


    template <typename AttackFunction>
    std::uint64_t calibrateIterations(
        const std::array<AttackInput, INPUT_COUNT>& inputs,
        AttackFunction attackFunction
    )
    {
        std::uint64_t iterations = 100'000;

        while (true)
        {
            const TimedRun run =
                runTimedBatch(
                    inputs,
                    iterations,
                    attackFunction
                );

            if (
                run.seconds >=
                TARGET_SAMPLE_SECONDS * 0.5
            )
            {
                const double scale =
                    TARGET_SAMPLE_SECONDS /
                    run.seconds;

                std::uint64_t calibrated =
                    static_cast<std::uint64_t>(
                        static_cast<double>(iterations) *
                        scale
                    );

                if (calibrated < 100'000)
                {
                    calibrated = 100'000;
                }

                return calibrated;
            }

            iterations *= 4;
        }
    }


    template <typename AttackFunction>
    BenchmarkResult benchmarkFunction(
        const std::array<AttackInput, INPUT_COUNT>& inputs,
        AttackFunction attackFunction
    )
    {
        const std::uint64_t iterations =
            calibrateIterations(
                inputs,
                attackFunction
            );

        std::vector<double> samples;
        samples.reserve(SAMPLE_COUNT);

        for (
            int sample = 0;
            sample < SAMPLE_COUNT;
            ++sample
        )
        {
            const TimedRun run =
                runTimedBatch(
                    inputs,
                    iterations,
                    attackFunction
                );

            const double nsPerAttack =
                run.seconds *
                1'000'000'000.0 /
                static_cast<double>(iterations);

            samples.push_back(nsPerAttack);
        }

        std::sort(
            samples.begin(),
            samples.end()
        );

        BenchmarkResult result;

        result.iterations = iterations;

        result.bestNs =
            samples.front();

        result.medianNs =
            samples[samples.size() / 2];

        result.worstNs =
            samples.back();

        result.meanNs =
            std::accumulate(
                samples.begin(),
                samples.end(),
                0.0
            ) /
            static_cast<double>(
                samples.size()
            );

        return result;
    }


    void warmUp(
        const std::array<AttackInput, INPUT_COUNT>& inputs
    )
    {
        std::cout
            << "Warming attack generation for "
            << WARMUP_SECONDS
            << " seconds...\n";

        const auto start = Clock::now();

        std::uint64_t iterations = 0;
        Bitboard checksum = 0;

        while (true)
        {
            const AttackInput& input =
                inputs[
                    static_cast<std::size_t>(iterations) &
                    INPUT_MASK
                ];


            // Static attack tables.
            checksum ^=
                KNIGHT_ATTACKS[
                    input.square
                ];

            checksum ^=
                KING_ATTACKS[
                    input.square
                ];

            checksum ^=
                PAWN_ATTACKS[
                    static_cast<std::size_t>(
                        Color::White
                    )
                ][input.square];

            checksum ^=
                PAWN_ATTACKS[
                    static_cast<std::size_t>(
                        Color::Black
                    )
                ][input.square];


            // Basic sliding attacks.
            checksum ^=
                straightAttacksBasic(
                    input.square,
                    input.occupancy
                );

            checksum ^=
                diagonalAttacksBasic(
                    input.square,
                    input.occupancy
                );

            checksum ^=
                straightAttacksBasic(
                    input.square,
                    input.occupancy
                ) |
                diagonalAttacksBasic(
                    input.square,
                    input.occupancy
                );


            // PEXT sliding attacks.
            checksum ^=
                straightAttacksPext(
                    input.square,
                    input.occupancy
                );

            checksum ^=
                diagonalAttacksPext(
                    input.square,
                    input.occupancy
                );

            checksum ^=
                queenAttacksPext(
                    input.square,
                    input.occupancy
                );


            ++iterations;

            // Avoid querying the clock every iteration.
            if ((iterations & 0xFFFF) == 0)
            {
                const auto now = Clock::now();

                const std::chrono::duration<double> elapsed =
                    now - start;

                if (
                    elapsed.count() >=
                    WARMUP_SECONDS
                )
                {
                    break;
                }
            }
        }

        benchmarkSink = checksum;

        std::cout
            << "Warm-up complete ("
            << iterations * 10
            << " requested attack calls).\n\n";
    }


    void printResult(
        std::string_view name,
        const BenchmarkResult& result
    )
    {
        const double millionAttacksPerSecond =
            1000.0 / result.medianNs;

        std::cout
            << std::left
            << std::setw(20)
            << name

            << std::right
            << std::fixed
            << std::setprecision(2)

            << std::setw(12)
            << result.medianNs

            << std::setw(12)
            << result.bestNs

            << std::setw(12)
            << result.worstNs

            << std::setw(14)
            << millionAttacksPerSecond

            << '\n';
    }
}


void runSlidingAttacksBenchmark()
{
    const std::array<AttackInput, INPUT_COUNT> inputs =
        generateInputs();


    std::cout
        << "\n========================================\n"
        << "        MOBIUS ATTACK BENCHMARK\n"
        << "========================================\n\n";


    warmUp(inputs);


    std::cout
        << std::left
        << std::setw(20)
        << "Case"

        << std::right
        << std::setw(12)
        << "Median ns"

        << std::setw(12)
        << "Best ns"

        << std::setw(12)
        << "Worst ns"

        << std::setw(14)
        << "M attacks/s"

        << '\n';


    std::cout
        << std::string(70, '-')
        << '\n';


    // =====================================================
    // KNIGHT
    // =====================================================

    const BenchmarkResult knightResult =
        benchmarkFunction(
            inputs,
            [](
                Square square,
                Bitboard
            ) noexcept
            {
                return KNIGHT_ATTACKS[square];
            }
        );

    printResult(
        "Knight lookup",
        knightResult
    );


    // =====================================================
    // KING
    // =====================================================

    const BenchmarkResult kingResult =
        benchmarkFunction(
            inputs,
            [](
                Square square,
                Bitboard
            ) noexcept
            {
                return KING_ATTACKS[square];
            }
        );

    printResult(
        "King lookup",
        kingResult
    );


    // =====================================================
    // WHITE PAWN
    // =====================================================

    const BenchmarkResult whitePawnResult =
        benchmarkFunction(
            inputs,
            [](
                Square square,
                Bitboard
            ) noexcept
            {
                return PAWN_ATTACKS[
                    static_cast<std::size_t>(
                        Color::White
                    )
                ][square];
            }
        );

    printResult(
        "Pawn white",
        whitePawnResult
    );


    // =====================================================
    // BLACK PAWN
    // =====================================================

    const BenchmarkResult blackPawnResult =
        benchmarkFunction(
            inputs,
            [](
                Square square,
                Bitboard
            ) noexcept
            {
                return PAWN_ATTACKS[
                    static_cast<std::size_t>(
                        Color::Black
                    )
                ][square];
            }
        );

    printResult(
        "Pawn black",
        blackPawnResult
    );


    std::cout
        << std::string(70, '-')
        << '\n';


    // =====================================================
    // STRAIGHT BASIC
    // =====================================================

    const BenchmarkResult straightBasicResult =
        benchmarkFunction(
            inputs,
            [](
                Square square,
                Bitboard occupancy
            ) noexcept
            {
                return straightAttacksBasic(
                    square,
                    occupancy
                );
            }
        );

    printResult(
        "Straight basic",
        straightBasicResult
    );


    // =====================================================
    // STRAIGHT PEXT
    // =====================================================

    const BenchmarkResult straightPextResult =
        benchmarkFunction(
            inputs,
            [](
                Square square,
                Bitboard occupancy
            ) noexcept
            {
                return straightAttacksPext(
                    square,
                    occupancy
                );
            }
        );

    printResult(
        "Straight PEXT",
        straightPextResult
    );


    std::cout
        << std::string(70, '-')
        << '\n';


    // =====================================================
    // DIAGONAL BASIC
    // =====================================================

    const BenchmarkResult diagonalBasicResult =
        benchmarkFunction(
            inputs,
            [](
                Square square,
                Bitboard occupancy
            ) noexcept
            {
                return diagonalAttacksBasic(
                    square,
                    occupancy
                );
            }
        );

    printResult(
        "Diagonal basic",
        diagonalBasicResult
    );


    // =====================================================
    // DIAGONAL PEXT
    // =====================================================

    const BenchmarkResult diagonalPextResult =
        benchmarkFunction(
            inputs,
            [](
                Square square,
                Bitboard occupancy
            ) noexcept
            {
                return diagonalAttacksPext(
                    square,
                    occupancy
                );
            }
        );

    printResult(
        "Diagonal PEXT",
        diagonalPextResult
    );


    std::cout
        << std::string(70, '-')
        << '\n';


    // =====================================================
    // QUEEN BASIC
    // =====================================================

    const BenchmarkResult queenBasicResult =
        benchmarkFunction(
            inputs,
            [](
                Square square,
                Bitboard occupancy
            ) noexcept
            {
                return
                    straightAttacksBasic(
                        square,
                        occupancy
                    ) |
                    diagonalAttacksBasic(
                        square,
                        occupancy
                    );
            }
        );

    printResult(
        "Queen basic",
        queenBasicResult
    );


    // =====================================================
    // QUEEN PEXT
    // =====================================================

    const BenchmarkResult queenPextResult =
        benchmarkFunction(
            inputs,
            [](
                Square square,
                Bitboard occupancy
            ) noexcept
            {
                return queenAttacksPext(
                    square,
                    occupancy
                );
            }
        );

    printResult(
        "Queen PEXT",
        queenPextResult
    );


    std::cout
        << std::string(70, '-')
        << '\n';


    // =====================================================
    // SPEEDUPS
    // =====================================================

    const double straightSpeedup =
        straightBasicResult.medianNs /
        straightPextResult.medianNs;

    const double diagonalSpeedup =
        diagonalBasicResult.medianNs /
        diagonalPextResult.medianNs;

    const double queenSpeedup =
        queenBasicResult.medianNs /
        queenPextResult.medianNs;


    std::cout
        << "\nPEXT speedup:\n"

        << "  Straight: "
        << std::fixed
        << std::setprecision(2)
        << straightSpeedup
        << "x\n"

        << "  Diagonal: "
        << diagonalSpeedup
        << "x\n"

        << "  Queen:    "
        << queenSpeedup
        << "x\n";


    std::cout
        << "\nOne operation = one requested attack bitboard.\n";
}