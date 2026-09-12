#include "attack_detector_benchmark.h"
#include "attack_detector.h"
#include "attacks.h"
#include "bitboard.h"
#include "position.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string_view>
#include <vector>


namespace
{
    using Clock = std::chrono::steady_clock;

    constexpr int SAMPLE_COUNT = 9;
    constexpr double TARGET_SAMPLE_SECONDS = 0.20;
    constexpr double WARMUP_SECONDS = 1.0;

    constexpr std::size_t INPUT_COUNT = 4096;
    constexpr std::size_t INPUT_MASK = INPUT_COUNT - 1;

    static_assert((INPUT_COUNT & INPUT_MASK) == 0);


    volatile std::uint64_t benchmarkSink = 0;


    enum class InputPattern
    {
        NoAttack,
        CheapAttack,
        SliderAttack,
        Mixed
    };


    enum class AttackerType
    {
        None,
        Pawn,
        Knight,
        King,
        Bishop,
        Rook,
        Queen
    };


    struct DetectorInput
    {
        Position position;
        Square target = 0;
        Color attackerColor = Color::Black;
        Bitboard occupancy = 0;
    };


    struct BenchmarkResult
    {
        double bestNs = 0.0;
        double medianNs = 0.0;
        double worstNs = 0.0;
        std::uint64_t iterations = 0;
    };


    Square makeTarget(std::size_t index)
    {
        // Keep targets between rank 2 and rank 7 so both pawn directions
        // always have at least one possible attacking square.
        return static_cast<Square>(
            8 + ((index * 37) % 48)
        );
    }


    Square firstSquare(Bitboard bitboard)
    {
        return getLSBit(bitboard);
    }


    void addBlackAttacker(
        Position& position,
        Square target,
        AttackerType attackerType
    )
    {
        Bitboard candidateSquares = 0;
        PieceType pieceType = PieceType::None;

        switch (attackerType)
        {
            case AttackerType::None:
                return;

            case AttackerType::Pawn:
                // Reverse pawn geometry: squares from which a black pawn
                // attacks target are WHITE pawn attacks from target.
                candidateSquares =
                    PAWN_ATTACKS[
                        static_cast<std::size_t>(Color::White)
                    ][target];
                pieceType = PieceType::Pawn;
                break;

            case AttackerType::Knight:
                candidateSquares = KNIGHT_ATTACKS[target];
                pieceType = PieceType::Knight;
                break;

            case AttackerType::King:
                candidateSquares = KING_ATTACKS[target];
                pieceType = PieceType::King;
                break;

            case AttackerType::Bishop:
                candidateSquares = bishopAttacks(target, 0);
                pieceType = PieceType::Bishop;
                break;

            case AttackerType::Rook:
                candidateSquares = rookAttacks(target, 0);
                pieceType = PieceType::Rook;
                break;

            case AttackerType::Queen:
                // Put queens on a diagonal so they are found by the
                // bishop/queen branch of isSquareAttacked().
                candidateSquares = bishopAttacks(target, 0);
                pieceType = PieceType::Queen;
                break;
        }

        const Square attackerSquare =
            firstSquare(candidateSquares);

        position.setPiece(
            makePiece(Color::Black, pieceType),
            attackerSquare
        );
    }


    AttackerType attackerForPattern(
        InputPattern pattern,
        std::size_t index
    )
    {
        if (pattern == InputPattern::NoAttack)
        {
            return AttackerType::None;
        }

        if (pattern == InputPattern::CheapAttack)
        {
            switch (index % 3)
            {
                case 0:
                    return AttackerType::Pawn;
                case 1:
                    return AttackerType::Knight;
                default:
                    return AttackerType::King;
            }
        }

        if (pattern == InputPattern::SliderAttack)
        {
            switch (index % 3)
            {
                case 0:
                    return AttackerType::Bishop;
                case 1:
                    return AttackerType::Rook;
                default:
                    return AttackerType::Queen;
            }
        }

        switch (index % 7)
        {
            case 0:
                return AttackerType::None;
            case 1:
                return AttackerType::Pawn;
            case 2:
                return AttackerType::Knight;
            case 3:
                return AttackerType::King;
            case 4:
                return AttackerType::Bishop;
            case 5:
                return AttackerType::Rook;
            default:
                return AttackerType::Queen;
        }
    }


    std::vector<DetectorInput> makeInputs(
        InputPattern pattern
    )
    {
        std::vector<DetectorInput> inputs(INPUT_COUNT);

        for (std::size_t i = 0; i < INPUT_COUNT; ++i)
        {
            DetectorInput& input = inputs[i];

            input.position.clear();
            input.target = makeTarget(i);
            input.attackerColor = Color::Black;

            addBlackAttacker(
                input.position,
                input.target,
                attackerForPattern(pattern, i)
            );

            input.occupancy = input.position.pieces();
        }

        return inputs;
    }


    std::uint64_t runIsSquareAttackedBatch(
        const std::vector<DetectorInput>& inputs,
        std::uint64_t iterations
    )
    {
        std::uint64_t checksum = 0;

        for (std::uint64_t i = 0; i < iterations; ++i)
        {
            const DetectorInput& input =
                inputs[static_cast<std::size_t>(i) & INPUT_MASK];

            checksum +=
                isSquareAttacked(
                    input.position,
                    input.target,
                    input.attackerColor,
                    input.occupancy
                )
                ? 1ULL
                : 0ULL;
        }

        return checksum;
    }


    std::uint64_t runAttackersToBatch(
        const std::vector<DetectorInput>& inputs,
        std::uint64_t iterations
    )
    {
        std::uint64_t checksum = 0;

        for (std::uint64_t i = 0; i < iterations; ++i)
        {
            const DetectorInput& input =
                inputs[static_cast<std::size_t>(i) & INPUT_MASK];

            checksum +=
                attackersTo(
                    input.position,
                    input.target,
                    input.attackerColor,
                    input.occupancy
                );
        }

        return checksum;
    }


    double timeIsSquareAttacked(
        const std::vector<DetectorInput>& inputs,
        std::uint64_t iterations
    )
    {
        const auto start = Clock::now();

        const std::uint64_t checksum =
            runIsSquareAttackedBatch(inputs, iterations);

        const auto end = Clock::now();

        benchmarkSink = benchmarkSink ^ checksum;

        return std::chrono::duration<double>(
            end - start
        ).count();
    }


    double timeAttackersTo(
        const std::vector<DetectorInput>& inputs,
        std::uint64_t iterations
    )
    {
        const auto start = Clock::now();

        const std::uint64_t checksum =
            runAttackersToBatch(inputs, iterations);

        const auto end = Clock::now();

        benchmarkSink = benchmarkSink ^ checksum;

        return std::chrono::duration<double>(
            end - start
        ).count();
    }


    std::uint64_t calibrateIsSquareAttacked(
        const std::vector<DetectorInput>& inputs
    )
    {
        std::uint64_t iterations = 1'000'000;

        double seconds =
            timeIsSquareAttacked(inputs, iterations);

        if (seconds <= 0.0)
        {
            return iterations;
        }

        const double scale =
            TARGET_SAMPLE_SECONDS / seconds;

        iterations = static_cast<std::uint64_t>(
            static_cast<double>(iterations) * scale
        );

        if (iterations < 100'000)
        {
            iterations = 100'000;
        }

        return iterations;
    }


    std::uint64_t calibrateAttackersTo(
        const std::vector<DetectorInput>& inputs
    )
    {
        std::uint64_t iterations = 1'000'000;

        double seconds =
            timeAttackersTo(inputs, iterations);

        if (seconds <= 0.0)
        {
            return iterations;
        }

        const double scale =
            TARGET_SAMPLE_SECONDS / seconds;

        iterations = static_cast<std::uint64_t>(
            static_cast<double>(iterations) * scale
        );

        if (iterations < 100'000)
        {
            iterations = 100'000;
        }

        return iterations;
    }


    BenchmarkResult benchmarkIsSquareAttacked(
        const std::vector<DetectorInput>& inputs
    )
    {
        BenchmarkResult result;

        result.iterations =
            calibrateIsSquareAttacked(inputs);

        std::vector<double> samples;
        samples.reserve(SAMPLE_COUNT);

        for (int sample = 0; sample < SAMPLE_COUNT; ++sample)
        {
            const double seconds =
                timeIsSquareAttacked(
                    inputs,
                    result.iterations
                );

            const double nsPerOperation =
                seconds * 1'000'000'000.0 /
                static_cast<double>(result.iterations);

            samples.push_back(nsPerOperation);
        }

        std::sort(samples.begin(), samples.end());

        result.bestNs = samples.front();
        result.medianNs = samples[SAMPLE_COUNT / 2];
        result.worstNs = samples.back();

        return result;
    }


    BenchmarkResult benchmarkAttackersTo(
        const std::vector<DetectorInput>& inputs
    )
    {
        BenchmarkResult result;

        result.iterations =
            calibrateAttackersTo(inputs);

        std::vector<double> samples;
        samples.reserve(SAMPLE_COUNT);

        for (int sample = 0; sample < SAMPLE_COUNT; ++sample)
        {
            const double seconds =
                timeAttackersTo(
                    inputs,
                    result.iterations
                );

            const double nsPerOperation =
                seconds * 1'000'000'000.0 /
                static_cast<double>(result.iterations);

            samples.push_back(nsPerOperation);
        }

        std::sort(samples.begin(), samples.end());

        result.bestNs = samples.front();
        result.medianNs = samples[SAMPLE_COUNT / 2];
        result.worstNs = samples.back();

        return result;
    }


    void printResult(
        std::string_view name,
        const BenchmarkResult& result
    )
    {
        const double millionOperationsPerSecond =
            1000.0 / result.medianNs;

        std::cout
            << std::left
            << std::setw(31)
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
            << millionOperationsPerSecond
            << '\n';
    }


    void warmUp(
        const std::vector<DetectorInput>& inputs
    )
    {
        const auto start = Clock::now();
        std::uint64_t calls = 0;

        while (
            std::chrono::duration<double>(
                Clock::now() - start
            ).count() < WARMUP_SECONDS
        )
        {
            const std::uint64_t batchSize = 100'000;

            benchmarkSink = benchmarkSink ^
                runIsSquareAttackedBatch(
                    inputs,
                    batchSize
                );

            benchmarkSink = benchmarkSink ^
                runAttackersToBatch(
                    inputs,
                    batchSize
                );

            calls += batchSize * 2;
        }

        std::cout
            << "Warm-up complete ("
            << calls
            << " requested detector calls).\n\n";
    }
}


void runAttackDetectorBenchmark()
{
    std::cout << "=======================================\n";
    std::cout << "    MOBIUS ATTACK DETECTOR BENCHMARK\n";
    std::cout << "=======================================\n\n";

    const std::vector<DetectorInput> noAttackInputs =
        makeInputs(InputPattern::NoAttack);

    const std::vector<DetectorInput> cheapAttackInputs =
        makeInputs(InputPattern::CheapAttack);

    const std::vector<DetectorInput> sliderAttackInputs =
        makeInputs(InputPattern::SliderAttack);

    const std::vector<DetectorInput> mixedInputs =
        makeInputs(InputPattern::Mixed);

    std::cout
        << "Warming attack detection for "
        << std::fixed
        << std::setprecision(2)
        << WARMUP_SECONDS
        << " seconds...\n";

    warmUp(mixedInputs);

    std::cout
        << std::left
        << std::setw(31)
        << "Case"
        << std::right
        << std::setw(12)
        << "Median ns"
        << std::setw(12)
        << "Best ns"
        << std::setw(12)
        << "Worst ns"
        << std::setw(14)
        << "M calls/s"
        << '\n';

    std::cout
        << std::string(81, '-')
        << '\n';

    printResult(
        "isAttacked / no attacker",
        benchmarkIsSquareAttacked(noAttackInputs)
    );

    printResult(
        "isAttacked / cheap attacker",
        benchmarkIsSquareAttacked(cheapAttackInputs)
    );

    printResult(
        "isAttacked / slider attacker",
        benchmarkIsSquareAttacked(sliderAttackInputs)
    );

    printResult(
        "isAttacked / mixed",
        benchmarkIsSquareAttacked(mixedInputs)
    );

    std::cout
        << std::string(81, '-')
        << '\n';

    printResult(
        "attackersTo / no attacker",
        benchmarkAttackersTo(noAttackInputs)
    );

    printResult(
        "attackersTo / cheap attacker",
        benchmarkAttackersTo(cheapAttackInputs)
    );

    printResult(
        "attackersTo / slider attacker",
        benchmarkAttackersTo(sliderAttackInputs)
    );

    printResult(
        "attackersTo / mixed",
        benchmarkAttackersTo(mixedInputs)
    );

    std::cout
        << std::string(81, '-')
        << '\n';

    std::cout
        << "One operation = one attack detector request.\n";
}
