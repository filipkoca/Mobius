<div>
<img src="assets/mobius_banner.png" alt="Mobius Logo">
</div>

---
# Chess Engine: *Mobius*

Mobius is a chess engine programmed from the scratch in C++20.
The goal of the project is to create my own serious engine without the use of external
libraries and sign Mobius up to an official engine chess tournament.

The project is currently in development.

## Technologies

- C++20
- CMake
- Ninja / Make
- console application

The project does not require any external libraries.

## Requirements

- C++20 compatable compiler
- CMake
- Ninja/Make

## Build

### Debug

```bash
cmake -S . -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build cmake-build-debug
```

### Release

```bash
cmake -S . -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build-release
```

## Run

### Debug

Build and run:

```bash
cmake --build cmake-build-debug --target mobius && ./cmake-build-debug/mobius
```

### Release

Build and run:

```bash
cmake --build cmake-build-release --target mobius && ./cmake-build-release/mobius
```

For engine performance measurements, the `Release` configuration is recommended.

## Tests

Correctness tests use assertions and should be run in the `Debug` configuration.

Build and run all tests:

```bash
cmake --build cmake-build-debug --target mobius_tests && ./cmake-build-debug/mobius_tests
```

## Benchmarks

Performance benchmarks are kept separate from correctness tests and should be run in the `Release` configuration.

Build and run all benchmarks:

```bash
cmake --build cmake-build-release --target mobius_benchmarks && ./cmake-build-release/mobius_benchmarks
```

The make/undo benchmark performs a CPU warm-up before measurement, runs multiple timed samples, and reports median, best, and worst time per `makeMove()` + `undoMove()` pair together with throughput in millions of pairs per second.

## Documentation

Technical documentation is located in the `docs/` directory.

- [Core documentation](docs/core.md)

Additional documentation will be added as more parts of the engine are implemented.

## Development and Commit Protocol

Format:

`TYPE: short description`

Used types:

- `FIX`
- `TEST`
- `FEAT`
- `REFACTOR`
- `DOCS`
- `SETUP`
- `PERF`
- `CHORE`

Example:

```text
FEAT: add FEN parser
TEST: add core FEN integration tests
PERF: optimize piece movement updates
DOCS: document core representation
```

## License

Mobius is distributed under the MIT License.

See the [LICENSE](LICENSE) file for details.
