// mutate_source — standalone CLI for Bin2Wrong source mutators.
// Strips AFL++ harness; takes a C file in, writes mutated C file out.
//
// Usage:
//   mutate_source --input file.c --output file.c [--seed N] [--mode M] [--count N]
//   --mode -1 = random, 0..8 = specific mutator from _mutator_options.
//
// Yonsei addition, 2026-05-01.
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <cstdlib>

extern "C" {
  void src_code_mutation_seeded(uint8_t *buf, size_t buf_size, char *pointer,
                                unsigned long seed, int mode_idx);
}

static const char *MUTATOR_NAMES[] = {
  "AssignmentMutator", "ConstantMutator", "DeleteMutator",
  "DuplicateMutator",  "ExpressionMutator", "JumpMutator",
  "StringMutator",     "SwitchMutator",     "GotoMutator"
};
static const int N_MUTATORS = 9;

static void usage(const char *argv0) {
  std::cerr << "Usage: " << argv0
            << " --input <file.c> --output <file.c> [--seed N] [--mode M] [--count N]\n";
  std::cerr << "  --mode -1 = random, 0..8 = specific mutator:\n";
  for (int i = 0; i < N_MUTATORS; i++) {
    std::cerr << "    " << i << " = " << MUTATOR_NAMES[i] << "\n";
  }
}

int main(int argc, char **argv) {
  std::string input_path, output_path;
  unsigned long seed = 0;
  int mode_idx = -1;
  int count = 1;

  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];
    if (arg == "--input" && i + 1 < argc)         input_path  = argv[++i];
    else if (arg == "--output" && i + 1 < argc)   output_path = argv[++i];
    else if (arg == "--seed" && i + 1 < argc)     seed        = std::stoul(argv[++i]);
    else if (arg == "--mode" && i + 1 < argc)     mode_idx    = std::stoi(argv[++i]);
    else if (arg == "--count" && i + 1 < argc)    count       = std::stoi(argv[++i]);
    else if (arg == "-h" || arg == "--help")      { usage(argv[0]); return 0; }
    else                                           { usage(argv[0]); return 1; }
  }

  if (input_path.empty() || output_path.empty()) {
    usage(argv[0]);
    return 1;
  }

  std::ifstream fin(input_path);
  if (!fin) {
    std::cerr << "[mutate_source] cannot open input: " << input_path << "\n";
    return 1;
  }
  std::stringstream ss;
  ss << fin.rdbuf();
  std::string content = ss.str();

  // generous output buffer (mutators may duplicate; 10x + slack)
  size_t out_size = content.length() * 10 + 4096;
  std::vector<char> out_buf(out_size, 0);

  if (count <= 1) {
    src_code_mutation_seeded(
      reinterpret_cast<uint8_t *>(const_cast<char *>(content.c_str())),
      content.length(),
      out_buf.data(),
      seed,
      mode_idx
    );

    std::ofstream fout(output_path);
    if (!fout) {
      std::cerr << "[mutate_source] cannot write output: " << output_path << "\n";
      return 1;
    }
    fout << out_buf.data();
    return 0;
  }

  // batch mode: --output is treated as prefix; produces output_001.c, output_002.c, ...
  for (int i = 0; i < count; i++) {
    std::fill(out_buf.begin(), out_buf.end(), 0);
    src_code_mutation_seeded(
      reinterpret_cast<uint8_t *>(const_cast<char *>(content.c_str())),
      content.length(),
      out_buf.data(),
      seed + (unsigned long)i,
      mode_idx
    );
    char path_i[1024];
    std::snprintf(path_i, sizeof(path_i), "%s_%03d.c", output_path.c_str(), i);
    std::ofstream fout(path_i);
    fout << out_buf.data();
  }
  return 0;
}
