# Standalone CLI for Bin2Wrong source mutators

Yonsei extension (branch `yonsei-llm4d`).

Strips the AFL++ harness and decompiler/compiler-randomization machinery.
Exposes the 9 source-mutator visitor classes
(Assignment, Constant, Delete, Duplicate, Expression, Jump, String, Switch, Goto)
as a standalone command-line tool, suitable for offline dataset generation
(e.g. ML training data augmentation from a fixed seed pool).

## Build

Requires LLVM/Clang dev libraries (matched). Tested on clangdev 18.

```bash
# example: conda env with clangdev=18 llvmdev=18 cmake
conda create -n bin2wrong -c conda-forge clangdev=18 llvmdev=18 cmake
conda activate bin2wrong

cd mutation/src/standalone-cli
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=$CONDA_PREFIX
make -j4
# -> build/mutate_source
```

## Use

```bash
./build/mutate_source --input seed.c --output mutated.c [--seed N] [--mode M] [--count N]
```

| flag | meaning |
|---|---|
| `--input <file>` | input C source |
| `--output <file>` | output C source (or prefix when `--count > 1`) |
| `--seed N` | RNG seed (default 0). Same seed + same input = same output. |
| `--mode M` | mutator selection: -1 (random), 0..8 (specific). default -1 |
| `--count N` | generate N mutations with seeds N+0, N+1, ... saved as `<output>_NNN.c` |

mutator indices:
| idx | name |
|---|---|
| 0 | AssignmentMutator |
| 1 | ConstantMutator |
| 2 | DeleteMutator |
| 3 | DuplicateMutator |
| 4 | ExpressionMutator |
| 5 | JumpMutator |
| 6 | StringMutator |
| 7 | SwitchMutator |
| 8 | GotoMutator |

## Modifications vs upstream FuturesLab/Bin2Wrong

Single-file change, marked with comment block:

- `mutation/src/code-mutators/CodeMutators.cpp` — appended `extern "C" void src_code_mutation_seeded(buf, size, out, seed, mode_idx)`.
  - Original `src_code_mutation` seeds RNG from `random_device` (non-reproducible).
  - New variant takes explicit seed for deterministic dataset generation.
  - Marked with `// === Yonsei extension: seeded standalone entry ===`.

New files:

- `mutation/src/standalone-cli/main.cpp` — CLI driver.
- `mutation/src/standalone-cli/CMakeLists.txt` — standalone build.
- `mutation/src/standalone-cli/README.md` — this file.

License: same as upstream Bin2Wrong (MIT).
