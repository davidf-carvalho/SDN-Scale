#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "avl/AVLTree.hpp"
#include "models/PacketRule.hpp"
#include "rbt/RedBlackTree.hpp"

namespace {

using Clock = std::chrono::high_resolution_clock;
using Ns = std::chrono::nanoseconds;

struct Args {
    std::size_t n = 100000;
    std::size_t runs = 30;
    std::size_t step = 1000;
    std::size_t searches = 10000;
    std::size_t deletions = 0;
    std::size_t warmup = 1000;
    std::uint32_t seed = 42;
    std::string output = "results/benchmark_raw.csv";
};

struct CsvWriter {
    explicit CsvWriter(const std::string& path) : out(path) {
        if (!out) {
            throw std::runtime_error("could not open CSV output: " + path);
        }
        out << "scenario,operacao,estrutura,n,tempo_ns,rotacoes,run_id\n";
    }

    void row(const std::string& scenario,
             const std::string& operation,
             const std::string& structure,
             std::size_t n,
             long long elapsed_ns,
             std::uint64_t rotations,
             std::size_t run_id) {
        out << scenario << ','
            << operation << ','
            << structure << ','
            << n << ','
            << elapsed_ns << ','
            << rotations << ','
            << run_id << '\n';
    }

    std::ofstream out;
};

void print_usage(const char* exe) {
    std::cout
        << "Usage: " << exe << " [options]\n"
        << "  --n VALUE              Number of PacketRule entries (default: 100000)\n"
        << "  --runs VALUE           Repetitions per scenario (default: 30)\n"
        << "  --step VALUE           CSV sample interval (default: 1000)\n"
        << "  --searches VALUE       Searches per run (default: 10000)\n"
        << "  --deletions VALUE      Deletions per run (default: n / 5)\n"
        << "  --warmup VALUE         Warm-up operations before measuring (default: 1000)\n"
        << "  --seed VALUE           Shared deterministic seed (default: 42)\n"
        << "  --output PATH          CSV output path (default: results/benchmark_raw.csv)\n"
        << "  --help                 Show this message\n";
}

std::size_t parse_size(const std::string& value, const std::string& name) {
    char* end = nullptr;
    const unsigned long long parsed = std::strtoull(value.c_str(), &end, 10);
    if (!end || *end != '\0') {
        throw std::invalid_argument("invalid numeric value for " + name + ": " + value);
    }
    return static_cast<std::size_t>(parsed);
}

Args parse_args(int argc, char** argv) {
    Args args;
    for (int i = 1; i < argc; ++i) {
        const std::string key = argv[i];
        if (key == "--help") {
            print_usage(argv[0]);
            std::exit(0);
        }
        if (i + 1 >= argc) {
            throw std::invalid_argument("missing value for option: " + key);
        }
        const std::string value = argv[++i];
        if (key == "--n") args.n = parse_size(value, key);
        else if (key == "--runs") args.runs = parse_size(value, key);
        else if (key == "--step") args.step = parse_size(value, key);
        else if (key == "--searches") args.searches = parse_size(value, key);
        else if (key == "--deletions") args.deletions = parse_size(value, key);
        else if (key == "--warmup") args.warmup = parse_size(value, key);
        else if (key == "--seed") args.seed = static_cast<std::uint32_t>(parse_size(value, key));
        else if (key == "--output") args.output = value;
        else throw std::invalid_argument("unknown option: " + key);
    }

    if (args.n == 0 || args.runs == 0 || args.step == 0 || args.searches == 0) {
        throw std::invalid_argument("--n, --runs, --step and --searches must be greater than zero");
    }
    if (args.deletions == 0) {
        args.deletions = args.n / 5;
    }
    if (args.deletions > args.n) {
        throw std::invalid_argument("--deletions cannot be greater than --n");
    }
    return args;
}

sdn::PacketRule make_rule(std::uint32_t id, std::mt19937& rng) {
    std::uniform_int_distribution<std::uint32_t> ip_dist(1, 0x00FFFFFFu);
    std::uniform_int_distribution<int> priority_dist(0, 255);
    return sdn::PacketRule{
        id,
        0x0A000000u | ip_dist(rng),
        0xC0A80000u | (ip_dist(rng) & 0x0000FFFFu),
        static_cast<std::uint8_t>(priority_dist(rng)),
    };
}

std::vector<sdn::PacketRule> make_rules(std::size_t n, std::uint32_t seed, bool shuffled) {
    std::vector<std::uint32_t> ids(n);
    std::iota(ids.begin(), ids.end(), 1u);

    std::mt19937 rng(seed);
    if (shuffled) {
        std::shuffle(ids.begin(), ids.end(), rng);
    }

    std::vector<sdn::PacketRule> rules;
    rules.reserve(n);
    for (std::uint32_t id : ids) {
        rules.push_back(make_rule(id, rng));
    }
    return rules;
}

std::vector<std::uint32_t> make_random_ids(std::size_t count,
                                           std::size_t max_id,
                                           std::uint32_t seed) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<std::uint32_t> dist(1, static_cast<std::uint32_t>(max_id));
    std::vector<std::uint32_t> ids;
    ids.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        ids.push_back(dist(rng));
    }
    return ids;
}

std::vector<std::uint32_t> make_delete_ids(std::size_t count,
                                           std::size_t max_id,
                                           std::uint32_t seed) {
    std::vector<std::uint32_t> ids(max_id);
    std::iota(ids.begin(), ids.end(), 1u);
    std::mt19937 rng(seed);
    std::shuffle(ids.begin(), ids.end(), rng);
    ids.resize(count);
    return ids;
}

template <typename Tree>
void warmup_tree(const std::vector<sdn::PacketRule>& rules) {
    Tree tree(rules.size());
    for (const auto& rule : rules) {
        tree.insert(rule);
    }
    for (const auto& rule : rules) {
        (void)tree.search(rule.id);
    }
    for (const auto& rule : rules) {
        (void)tree.remove(rule.id);
    }
}

void run_warmup(const Args& args) {
    if (args.warmup == 0) {
        return;
    }
    const std::size_t warmup_n = std::min(args.warmup, args.n);
    const auto rules = make_rules(warmup_n, args.seed, true);
    warmup_tree<sdn::AVLTree>(rules);
    warmup_tree<sdn::RedBlackTree>(rules);
}

template <typename Tree>
void populate(Tree& tree, const std::vector<sdn::PacketRule>& rules) {
    for (const auto& rule : rules) {
        tree.insert(rule);
    }
}

template <typename Tree>
void validate_or_throw(const Tree& tree,
                       const std::string& scenario,
                       const std::string& operation,
                       const std::string& structure,
                       std::size_t run_id,
                       std::size_t processed) {
    if (tree.validate()) {
        return;
    }

    std::ostringstream message;
    message << structure
            << " invariant failure"
            << " scenario=" << scenario
            << " operation=" << operation
            << " run=" << run_id
            << " processed=" << processed;
    throw std::runtime_error(message.str());
}

template <typename Tree>
void benchmark_insert(const std::string& scenario,
                      const std::string& structure,
                      const std::vector<sdn::PacketRule>& rules,
                      std::size_t step,
                      std::size_t run_id,
                      CsvWriter& csv) {
    Tree tree(rules.size());
    const auto rotation_start = tree.rotation_count();
    const auto start = Clock::now();

    for (std::size_t i = 0; i < rules.size(); ++i) {
        tree.insert(rules[i]);
        const std::size_t inserted = i + 1;
        if (inserted % step == 0 || inserted == rules.size()) {
            const auto elapsed = std::chrono::duration_cast<Ns>(Clock::now() - start).count();
            csv.row(scenario,
                    "insert",
                    structure,
                    inserted,
                    elapsed,
                    tree.rotation_count() - rotation_start,
                    run_id);
            validate_or_throw(tree, scenario, "insert", structure, run_id, inserted);
        }
    }
    validate_or_throw(tree, scenario, "insert", structure, run_id, rules.size());
}

template <typename Tree>
void benchmark_search(const std::string& scenario,
                      const std::string& structure,
                      const std::vector<sdn::PacketRule>& rules,
                      const std::vector<std::uint32_t>& search_ids,
                      std::size_t step,
                      std::size_t run_id,
                      CsvWriter& csv) {
    Tree tree(rules.size());
    populate(tree, rules);
    validate_or_throw(tree, scenario, "search", structure, run_id, 0);

    std::size_t found = 0;
    const auto start = Clock::now();
    for (std::size_t i = 0; i < search_ids.size(); ++i) {
        if (tree.search(search_ids[i]) != nullptr) {
            ++found;
        }
        const std::size_t searched = i + 1;
        if (searched % step == 0 || searched == search_ids.size()) {
            const auto elapsed = std::chrono::duration_cast<Ns>(Clock::now() - start).count();
            csv.row(scenario, "search", structure, searched, elapsed, 0, run_id);
            validate_or_throw(tree, scenario, "search", structure, run_id, searched);
        }
    }

    if (found != search_ids.size()) {
        throw std::runtime_error(structure + " missed an inserted key during search");
    }
}

template <typename Tree>
void benchmark_delete(const std::string& scenario,
                      const std::string& structure,
                      const std::vector<sdn::PacketRule>& rules,
                      const std::vector<std::uint32_t>& delete_ids,
                      std::size_t step,
                      std::size_t run_id,
                      CsvWriter& csv) {
    Tree tree(rules.size());
    populate(tree, rules);
    validate_or_throw(tree, scenario, "delete", structure, run_id, 0);
    const auto rotation_start = tree.rotation_count();
    const auto start = Clock::now();

    for (std::size_t i = 0; i < delete_ids.size(); ++i) {
        if (!tree.remove(delete_ids[i])) {
            throw std::runtime_error(structure + " failed to delete an inserted key");
        }
        const std::size_t removed = i + 1;
        if (removed % step == 0 || removed == delete_ids.size()) {
            const auto elapsed = std::chrono::duration_cast<Ns>(Clock::now() - start).count();
            csv.row(scenario,
                    "delete",
                    structure,
                    removed,
                    elapsed,
                    tree.rotation_count() - rotation_start,
                    run_id);
            validate_or_throw(tree, scenario, "delete", structure, run_id, removed);
        }
    }

    validate_or_throw(tree, scenario, "delete", structure, run_id, delete_ids.size());
}

void run_structure_benchmarks(const std::string& scenario,
                              const std::vector<sdn::PacketRule>& rules,
                              const std::vector<std::uint32_t>& search_ids,
                              const std::vector<std::uint32_t>& delete_ids,
                              const Args& args,
                              std::size_t run_id,
                              CsvWriter& csv) {
    benchmark_insert<sdn::AVLTree>(scenario, "AVL", rules, args.step, run_id, csv);
    benchmark_insert<sdn::RedBlackTree>(scenario, "RBT", rules, args.step, run_id, csv);

    benchmark_search<sdn::AVLTree>(scenario, "AVL", rules, search_ids, args.step, run_id, csv);
    benchmark_search<sdn::RedBlackTree>(scenario, "RBT", rules, search_ids, args.step, run_id, csv);

    benchmark_delete<sdn::AVLTree>(scenario, "AVL", rules, delete_ids, args.step, run_id, csv);
    benchmark_delete<sdn::RedBlackTree>(scenario, "RBT", rules, delete_ids, args.step, run_id, csv);
}

} // namespace

int main(int argc, char** argv) {
    try {
        const Args args = parse_args(argc, argv);
        const std::filesystem::path output_path(args.output);
        if (output_path.has_parent_path()) {
            std::filesystem::create_directories(output_path.parent_path());
        }

        std::cout << "SDN-Scale benchmark\n"
                  << "n=" << args.n
                  << " runs=" << args.runs
                  << " step=" << args.step
                  << " searches=" << args.searches
                  << " deletions=" << args.deletions
                  << " warmup=" << args.warmup
                  << " seed=" << args.seed
                  << "\n";

        run_warmup(args);
        CsvWriter csv(args.output);

        for (std::size_t run = 1; run <= args.runs; ++run) {
            const auto run_seed = static_cast<std::uint32_t>(args.seed + run - 1);
            const auto search_ids = make_random_ids(args.searches, args.n, run_seed + 100000u);
            const auto delete_ids = make_delete_ids(args.deletions, args.n, run_seed + 200000u);

            const auto ordered_rules = make_rules(args.n, args.seed, false);
            run_structure_benchmarks("ordered", ordered_rules, search_ids, delete_ids, args, run, csv);

            const auto random_rules = make_rules(args.n, args.seed, true);
            run_structure_benchmarks("random", random_rules, search_ids, delete_ids, args, run, csv);

            std::cout << "completed run " << run << "/" << args.runs << "\n";
        }

        std::cout << "CSV written to " << args.output << "\n";
    } catch (const std::exception& e) {
        std::cerr << "benchmark error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
