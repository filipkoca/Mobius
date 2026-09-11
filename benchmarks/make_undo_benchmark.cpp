#include "position.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string_view>
#include <vector>

#include "make_undo_benchmark.h"


namespace
{
    using Clock = std::chrono::steady_clock;

    constexpr int SAMPLE_COUNT = 9;

    // We want each timing sample to last roughly this long.
    // This reduces timer noise significantly.
    constexpr double TARGET_SAMPLE_SECONDS = 0.30;

    // Warm the CPU before collecting any numbers.
    constexpr double WARMUP_SECONDS = 2.0;


    struct BenchmarkCase
    {
        std::string_view name;

        Position position;

        StateInfo rootState;
        StateInfo childState;

        Move move = 0;
    };


    struct BenchmarkResult
    {
        double bestNs = 0.0;
        double medianNs = 0.0;
        double meanNs = 0.0;
        double worstNs = 0.0;

        std::uint64_t iterations = 0;
    };


    Move makeBenchmarkMove(
        Square from,
        Square to,
        MoveType type = MoveType::Normal,
        std::uint8_t data = 0
    )
    {
        return static_cast<Move>(
            static_cast<Move>(to) |
            (static_cast<Move>(from) << MOVE_FROM_SHIFT) |
            (static_cast<Move>(data) << MOVE_DATA_SHIFT) |
            (
                static_cast<Move>(
                    static_cast<std::uint8_t>(type)
                )
                << MOVE_TYPE_SHIFT
            )
        );
    }


    void resetCase(
        BenchmarkCase& benchmark,
        Color sideToMove,
        CastlingRights castlingRights = NO_CASTLING,
        Square enPassantSquare = NO_SQUARE,
        std::uint16_t halfmoveClock = 0
    )
    {
        benchmark.position.clear();

        benchmark.rootState = StateInfo{};
        benchmark.childState = StateInfo{};

        benchmark.rootState.castlingRights = castlingRights;
        benchmark.rootState.enPassantSquare = enPassantSquare;
        benchmark.rootState.halfmoveClock = halfmoveClock;
        benchmark.rootState.capturedPiece = NO_PIECE;

        benchmark.position.setState(benchmark.rootState);
        benchmark.position.setSideToMove(sideToMove);
    }


    void setupQuietMove(BenchmarkCase& benchmark)
    {
        benchmark.name = "Quiet knight";

        resetCase(
            benchmark,
            Color::White
        );

        benchmark.position.setPiece(
            makePiece(
                Color::White,
                PieceType::Knight
            ),
            6
        );

        // g1 -> f3
        benchmark.move =
            makeBenchmarkMove(6, 21);
    }


    void setupCapture(BenchmarkCase& benchmark)
    {
        benchmark.name = "Normal capture";

        resetCase(
            benchmark,
            Color::White
        );

        benchmark.position.setPiece(
            makePiece(
                Color::White,
                PieceType::Bishop
            ),
            26
        );

        benchmark.position.setPiece(
            makePiece(
                Color::Black,
                PieceType::Pawn
            ),
            53
        );

        // c4 x f7
        benchmark.move =
            makeBenchmarkMove(26, 53);
    }


    void setupDoublePawnPush(BenchmarkCase& benchmark)
    {
        benchmark.name = "Double pawn";

        resetCase(
            benchmark,
            Color::White
        );

        benchmark.position.setPiece(
            makePiece(
                Color::White,
                PieceType::Pawn
            ),
            12
        );

        // e2 -> e4
        benchmark.move =
            makeBenchmarkMove(12, 28);
    }


    void setupRookMove(BenchmarkCase& benchmark)
    {
        benchmark.name = "Rook rights";

        resetCase(
            benchmark,
            Color::White,
            ALL_CASTLING
        );

        benchmark.position.setPiece(
            makePiece(
                Color::White,
                PieceType::Rook
            ),
            7
        );

        // h1 -> h2
        benchmark.move =
            makeBenchmarkMove(7, 15);
    }


    void setupKingMove(BenchmarkCase& benchmark)
    {
        benchmark.name = "King rights";

        resetCase(
            benchmark,
            Color::White,
            ALL_CASTLING
        );

        benchmark.position.setPiece(
            makePiece(
                Color::White,
                PieceType::King
            ),
            4
        );

        // e1 -> e2
        benchmark.move =
            makeBenchmarkMove(4, 12);
    }


    void setupPromotion(BenchmarkCase& benchmark)
    {
        benchmark.name = "Promotion";

        resetCase(
            benchmark,
            Color::White
        );

        benchmark.position.setPiece(
            makePiece(
                Color::White,
                PieceType::Pawn
            ),
            48
        );

        // a7 -> a8 = Queen
        benchmark.move =
            makeBenchmarkMove(
                48,
                56,
                MoveType::Promotion,
                3
            );
    }


    void setupEnPassant(BenchmarkCase& benchmark)
    {
        benchmark.name = "En passant";

        resetCase(
            benchmark,
            Color::White,
            NO_CASTLING,
            43
        );

        benchmark.position.setPiece(
            makePiece(
                Color::White,
                PieceType::Pawn
            ),
            36
        );

        benchmark.position.setPiece(
            makePiece(
                Color::Black,
                PieceType::Pawn
            ),
            35
        );

        // e5 x d6 en passant
        benchmark.move =
            makeBenchmarkMove(
                36,
                43,
                MoveType::EnPassant
            );
    }


    void setupCastling(BenchmarkCase& benchmark)
    {
        benchmark.name = "Castling";

        resetCase(
            benchmark,
            Color::White,
            ALL_CASTLING
        );

        benchmark.position.setPiece(
            makePiece(
                Color::White,
                PieceType::King
            ),
            4
        );

        benchmark.position.setPiece(
            makePiece(
                Color::White,
                PieceType::Rook
            ),
            7
        );

        // e1 -> g1
        benchmark.move =
            makeBenchmarkMove(
                4,
                6,
                MoveType::Castling
            );
    }


    void setupCases(
        std::array<BenchmarkCase, 8>& cases
    )
    {
        setupQuietMove(cases[0]);
        setupCapture(cases[1]);
        setupDoublePawnPush(cases[2]);
        setupRookMove(cases[3]);
        setupKingMove(cases[4]);
        setupPromotion(cases[5]);
        setupEnPassant(cases[6]);
        setupCastling(cases[7]);
    }


    inline void executePair(
        BenchmarkCase& benchmark
    )
    {
        benchmark.position.makeMove(
            benchmark.move,
            benchmark.childState
        );

        benchmark.position.undoMove(
            benchmark.move,
            benchmark.rootState
        );
    }


    double runTimedBatch(
        BenchmarkCase& benchmark,
        std::uint64_t iterations
    )
    {
        // Prevent the compiler from moving work across
        // the timing boundaries.
        std::atomic_signal_fence(
            std::memory_order_seq_cst
        );

        auto start = Clock::now();

        for (
            std::uint64_t i = 0;
            i < iterations;
            i++
        )
        {
            executePair(benchmark);
        }

        auto end = Clock::now();

        std::atomic_signal_fence(
            std::memory_order_seq_cst
        );

        // Check outside the measured region.
        if (!benchmark.position.isValid())
        {
            std::cerr
                << "Position corrupted in benchmark: "
                << benchmark.name
                << '\n';

            std::terminate();
        }

        if (
            &benchmark.position.getState() !=
            &benchmark.rootState
        )
        {
            std::cerr
                << "State pointer corrupted in benchmark: "
                << benchmark.name
                << '\n';

            std::terminate();
        }

        std::chrono::duration<double> elapsed =
            end - start;

        return elapsed.count();
    }


    std::uint64_t calibrateIterations(
        BenchmarkCase& benchmark
    )
    {
        std::uint64_t iterations = 10'000;

        while (true)
        {
            double seconds =
                runTimedBatch(
                    benchmark,
                    iterations
                );

            if (
                seconds >=
                TARGET_SAMPLE_SECONDS * 0.5
            )
            {
                double scale =
                    TARGET_SAMPLE_SECONDS /
                    seconds;

                auto calibrated =
                    static_cast<std::uint64_t>(
                        static_cast<double>(iterations)
                        * scale
                    );

                if (calibrated < 10'000)
                {
                    calibrated = 10'000;
                }

                return calibrated;
            }

            iterations *= 4;
        }
    }


    BenchmarkResult benchmarkCase(
        BenchmarkCase& benchmark
    )
    {
        std::uint64_t iterations =
            calibrateIterations(benchmark);

        std::vector<double> samples;
        samples.reserve(SAMPLE_COUNT);

        for (
            int sample = 0;
            sample < SAMPLE_COUNT;
            sample++
        )
        {
            double seconds =
                runTimedBatch(
                    benchmark,
                    iterations
                );

            double nsPerPair =
                seconds *
                1'000'000'000.0 /
                static_cast<double>(iterations);

            samples.push_back(nsPerPair);
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
        std::array<BenchmarkCase, 8>& cases
    )
    {
        std::cout
            << "Warming CPU for "
            << WARMUP_SECONDS
            << " seconds...\n";

        auto start = Clock::now();

        std::uint64_t pairs = 0;

        while (true)
        {
            for (
                BenchmarkCase& benchmark :
                cases
            )
            {
                executePair(benchmark);
                ++pairs;
            }

            auto now = Clock::now();

            std::chrono::duration<double> elapsed =
                now - start;

            if (
                elapsed.count() >=
                WARMUP_SECONDS
            )
            {
                break;
            }
        }

        std::cout
            << "Warm-up complete ("
            << pairs
            << " make/undo pairs).\n\n";
    }


    double runMixedBatch(
        std::array<BenchmarkCase, 8>& cases,
        std::uint64_t rounds
    )
    {
        std::atomic_signal_fence(
            std::memory_order_seq_cst
        );

        auto start = Clock::now();

        for (
            std::uint64_t round = 0;
            round < rounds;
            round++
        )
        {
            for (
                BenchmarkCase& benchmark :
                cases
            )
            {
                executePair(benchmark);
            }
        }

        auto end = Clock::now();

        std::atomic_signal_fence(
            std::memory_order_seq_cst
        );

        std::chrono::duration<double> elapsed =
            end - start;

        return elapsed.count();
    }


    void printResult(
        std::string_view name,
        const BenchmarkResult& result
    )
    {
        double millionPairsPerSecond =
            1000.0 / result.medianNs;

        std::cout
            << std::left
            << std::setw(18)
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
            << millionPairsPerSecond

            << '\n';
    }


    void benchmarkMixed(
        std::array<BenchmarkCase, 8>& cases
    )
    {
        std::uint64_t rounds = 10'000;

        while (true)
        {
            double seconds =
                runMixedBatch(
                    cases,
                    rounds
                );

            if (
                seconds >=
                TARGET_SAMPLE_SECONDS * 0.5
            )
            {
                double scale =
                    TARGET_SAMPLE_SECONDS /
                    seconds;

                rounds =
                    static_cast<std::uint64_t>(
                        static_cast<double>(rounds)
                        * scale
                    );

                break;
            }

            rounds *= 4;
        }

        std::vector<double> samples;

        for (
            int sample = 0;
            sample < SAMPLE_COUNT;
            sample++
        )
        {
            double seconds =
                runMixedBatch(
                    cases,
                    rounds
                );

            std::uint64_t pairCount =
                rounds * cases.size();

            double nsPerPair =
                seconds *
                1'000'000'000.0 /
                static_cast<double>(pairCount);

            samples.push_back(nsPerPair);
        }

        std::sort(
            samples.begin(),
            samples.end()
        );

        BenchmarkResult result;

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

        printResult(
            "MIXED",
            result
        );
    }
}


void runMakeUndoBenchmark()
{
    std::array<BenchmarkCase, 8> cases{};

    setupCases(cases);

    std::cout
        << "========================================\n"
        << "       MOBIUS MAKE/UNDO BENCHMARK\n"
        << "========================================\n\n";

    /*
     * Warm-up is intentionally done before timing.
     *
     * This helps:
     * - CPU frequency ramp up
     * - instruction/data caches warm up
     * - branch predictor warm up
     * - pages become resident
     *
     * The warm-up results are discarded.
     */
    warmUp(cases);

    std::cout
        << std::left
        << std::setw(18)
        << "Case"

        << std::right
        << std::setw(12)
        << "Median ns"

        << std::setw(12)
        << "Best ns"

        << std::setw(12)
        << "Worst ns"

        << std::setw(14)
        << "M pairs/s"

        << '\n';

    std::cout
        << std::string(68, '-')
        << '\n';

    for (
        BenchmarkCase& benchmark :
        cases
    )
    {
        BenchmarkResult result =
            benchmarkCase(benchmark);

        printResult(
            benchmark.name,
            result
        );
    }

    std::cout
        << std::string(68, '-')
        << '\n';

    benchmarkMixed(cases);

    std::cout
        << '\n'
        << "One operation = makeMove + undoMove pair.\n";
}