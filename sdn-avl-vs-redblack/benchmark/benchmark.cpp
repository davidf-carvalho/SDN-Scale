/**
 * benchmark.cpp — Integrante 2 (DevOps & SRE)
 * SDN-Scale: AVL vs Red-Black Tree
 * Disciplina: Estrutura de Dados II — Prof. Ricardo Sekeff
 *
 * Coleta tempos em nanossegundos para inserção, busca e deleção.
 * Gera CSV no formato: operacao,estrutura,n,tempo_ns,rotacoes,run_id
 *
 * Compilar:
 *   g++ -O2 -std=c++17 -o benchmark benchmark.cpp \
 *       ../src/avl/AVLTree.cpp ../src/rbt/RedBlackTree.cpp
 */

#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "../src/avl/AVLTree.hpp"
#include "../src/rbt/RedBlackTree.hpp"
#include "../src/models/PacketRule.hpp"

// ─── Configurações ────────────────────────────────────────────────────────────

static constexpr uint64_t SEED         = 42;
static constexpr int      N_TOTAL      = 100'000;
static constexpr int      N_RUNS       = 30;      // repetições por experimento
static constexpr int      WARMUP_RUNS  = 1'000;   // warm-up antes de medir
static constexpr int      N_SEARCH     = 10'000;  // buscas aleatórias
static constexpr int      N_DELETE_PCT = 20;      // % de nós deletados

// ─── Utilitários de tempo ─────────────────────────────────────────────────────

using Clock     = std::chrono::high_resolution_clock;
using Nanosecs  = std::chrono::nanoseconds;

inline int64_t now_ns() {
    return std::chrono::duration_cast<Nanosecs>(
               Clock::now().time_since_epoch()).count();
}

// ─── Gerador de dados ─────────────────────────────────────────────────────────

/**
 * Gera N PacketRules com IDs sequenciais (pior caso para BST comum).
 * Usando a seed compartilhada para reprodutibilidade.
 */
std::vector<sdn::PacketRule> generate_sequential(int n, uint64_t seed) {
    std::mt19937 rng(seed);
    std::vector<sdn::PacketRule> rules;
    rules.reserve(n);
    for (int i = 1; i <= n; ++i) {
        rules.emplace_back(
            static_cast<uint32_t>(i),
            static_cast<uint32_t>(rng()),
            static_cast<uint32_t>(rng()),
            static_cast<uint8_t>(rng() % 256)
        );
    }
    return rules;
}

/**
 * Gera N PacketRules com IDs aleatórios (caso médio).
 */
std::vector<sdn::PacketRule> generate_random(int n, uint64_t seed) {
    std::mt19937 rng(seed + 1);
    std::vector<sdn::PacketRule> rules;
    rules.reserve(n);
    std::vector<uint32_t> ids(n);
    for (int i = 0; i < n; ++i) ids[i] = i + 1;
    std::shuffle(ids.begin(), ids.end(), rng);
    for (int i = 0; i < n; ++i) {
        rules.emplace_back(
            ids[i],
            static_cast<uint32_t>(rng()),
            static_cast<uint32_t>(rng()),
            static_cast<uint8_t>(rng() % 256)
        );
    }
    return rules;
}

// ─── CSV ──────────────────────────────────────────────────────────────────────

struct CsvRow {
    std::string operacao;
    std::string estrutura;
    int         n;
    int64_t     tempo_ns;
    uint64_t    rotacoes;
    int         run_id;
};

void write_csv(const std::string& path, const std::vector<CsvRow>& rows) {
    std::ofstream f(path);
    f << "operacao,estrutura,n,tempo_ns,rotacoes,run_id\n";
    for (const auto& r : rows)
        f << r.operacao << ',' << r.estrutura << ',' << r.n << ','
          << r.tempo_ns << ',' << r.rotacoes  << ',' << r.run_id << '\n';
    std::cout << "[CSV] Salvo em: " << path << " (" << rows.size() << " linhas)\n";
}

// ─── Experimento 1 — Inserção em lotes de 1000 ───────────────────────────────

template<typename Tree>
void bench_insert(const std::string& nome,
                  const std::vector<sdn::PacketRule>& rules,
                  int run_id,
                  std::vector<CsvRow>& out)
{
    Tree tree;

    // warm-up
    for (int i = 0; i < WARMUP_RUNS && i < (int)rules.size(); ++i)
        tree.insert(rules[i]);
    tree.clear();

    // medição a cada 1000 inserções
    int batch = 1000;
    for (int n = batch; n <= N_TOTAL; n += batch) {
        Tree t;
        auto t0 = now_ns();
        for (int i = 0; i < n; ++i)
            t.insert(rules[i]);
        auto t1 = now_ns();

        out.push_back({"insert", nome, n, t1 - t0, t.rotation_count(), run_id});
    }
}

// ─── Experimento 2 — Busca ────────────────────────────────────────────────────

template<typename Tree>
void bench_search(const std::string& nome,
                  const std::vector<sdn::PacketRule>& rules,
                  int run_id,
                  std::vector<CsvRow>& out)
{
    // popula árvore completa
    Tree tree;
    for (const auto& r : rules) tree.insert(r);

    // gera IDs aleatórios para busca
    std::mt19937 rng(SEED + run_id);
    std::uniform_int_distribution<uint32_t> dist(1, N_TOTAL);

    // warm-up
    for (int i = 0; i < WARMUP_RUNS; ++i)
        tree.search(dist(rng));

    // medição
    auto t0 = now_ns();
    for (int i = 0; i < N_SEARCH; ++i)
        tree.search(dist(rng));
    auto t1 = now_ns();

    int64_t avg_ns = (t1 - t0) / N_SEARCH;
    out.push_back({"search", nome, N_SEARCH, avg_ns, tree.rotation_count(), run_id});
}

// ─── Experimento 3 — Deleção ──────────────────────────────────────────────────

template<typename Tree>
void bench_delete(const std::string& nome,
                  const std::vector<sdn::PacketRule>& rules,
                  int run_id,
                  std::vector<CsvRow>& out)
{
    // popula árvore completa
    Tree tree;
    for (const auto& r : rules) tree.insert(r);

    // seleciona 20% dos IDs para deletar
    int n_del = N_TOTAL * N_DELETE_PCT / 100;
    std::mt19937 rng(SEED + run_id + 1000);
    std::vector<uint32_t> del_ids;
    del_ids.reserve(n_del);
    std::vector<uint32_t> all_ids(N_TOTAL);
    for (int i = 0; i < N_TOTAL; ++i) all_ids[i] = rules[i].id;
    std::shuffle(all_ids.begin(), all_ids.end(), rng);
    for (int i = 0; i < n_del; ++i) del_ids.push_back(all_ids[i]);

    // warm-up (separa árvore auxiliar)
    Tree tree_wu;
    for (int i = 0; i < WARMUP_RUNS && i < (int)rules.size(); ++i)
        tree_wu.insert(rules[i]);
    for (int i = 0; i < WARMUP_RUNS && i < n_del; ++i)
        tree_wu.remove(del_ids[i]);

    // medição por nó deletado
    int step = n_del / 10; // 10 pontos na curva
    for (int k = step; k <= n_del; k += step) {
        Tree t;
        for (const auto& r : rules) t.insert(r);
        auto t0 = now_ns();
        for (int i = 0; i < k; ++i) t.remove(del_ids[i]);
        auto t1 = now_ns();
        out.push_back({"delete", nome, k, t1 - t0, t.rotation_count(), run_id});
    }
}

// ─── Main ─────────────────────────────────────────────────────────────────────

int main() {
    std::cout << "=== SDN-Scale Benchmark — seed=" << SEED
              << ", N=" << N_TOTAL
              << ", runs=" << N_RUNS << " ===\n\n";

    // gera os dois conjuntos de dados
    auto seq_rules = generate_sequential(N_TOTAL, SEED);
    auto rnd_rules = generate_random    (N_TOTAL, SEED);

    std::vector<CsvRow> rows_insert, rows_search, rows_delete;

    for (int run = 1; run <= N_RUNS; ++run) {
        std::cout << "Run " << run << "/" << N_RUNS << "...\n";

        // ── Inserção sequencial ──
        bench_insert<sdn::AVLTree>      ("AVL", seq_rules, run, rows_insert);
        bench_insert<sdn::RedBlackTree> ("RBT", seq_rules, run, rows_insert);

        // ── Busca ──
        bench_search<sdn::AVLTree>      ("AVL", seq_rules, run, rows_search);
        bench_search<sdn::RedBlackTree> ("RBT", seq_rules, run, rows_search);

        // ── Deleção ──
        bench_delete<sdn::AVLTree>      ("AVL", seq_rules, run, rows_delete);
        bench_delete<sdn::RedBlackTree> ("RBT", seq_rules, run, rows_delete);
    }

    // salva CSVs
    write_csv("../data/insert_results.csv",  rows_insert);
    write_csv("../data/search_results.csv",  rows_search);
    write_csv("../data/delete_results.csv",  rows_delete);

    std::cout << "\nBenchmark concluído!\n";
    return 0;
}
