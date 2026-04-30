#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include "../models/PacketRule.hpp"

namespace sdn {

/**
 * AVLTree - Árvore AVL otimizada para roteamento SDN.
 *
 * Invariante de balanceamento: para todo nó N,
 *   |altura(N.esq) - altura(N.dir)| <= 1
 *
 * Complexidades garantidas (n = nós na árvore):
 *   - Busca:   O(log n)   altura máxima ≈ 1.44 · log₂(n+2)
 *   - Inserção: O(log n)  ≤ 1 rotação dupla ou 2 simples
 *   - Remoção: O(log n)   O(log n) rotações no pior caso
 *
 * Otimizações de desempenho aplicadas:
 *   - Pool de nós com lista livre → elimina overhead de malloc/free
 *   - Altura armazenada no nó → atualização O(1) sem recálculo
 *   - Funções críticas marcadas [[gnu::always_inline]] / inline
 *   - noexcept em hot paths → sem overhead de exception table
 *   - Sem herança virtual → dispatch direto
 */
class AVLTree {
public:
    explicit AVLTree(size_t reserve = 0);
    ~AVLTree();

    AVLTree(const AVLTree&)            = delete;
    AVLTree& operator=(const AVLTree&) = delete;
    AVLTree(AVLTree&&)                 noexcept;
    AVLTree& operator=(AVLTree&&)      noexcept;

    // ── Operações principais ──────────────────────────────────────────────

    /** Insere ou substitui uma regra pelo id. */
    void insert(const PacketRule& rule);

    /**
     * Remove a regra com o id informado.
     * @return true se encontrou e removeu, false se não existia.
     */
    bool remove(uint32_t id) noexcept;

    /**
     * Busca a regra com o id informado.
     * @return ponteiro para a regra (válido enquanto a árvore existir)
     *         ou nullptr se não encontrada.
     */
    [[nodiscard]] const PacketRule* search(uint32_t id) const noexcept;

    // ── Métricas para benchmark e QA ─────────────────────────────────────

    /** Altura da raiz (0 = árvore vazia). */
    [[nodiscard]] int      height()         const noexcept;

    /** Número de regras armazenadas. */
    [[nodiscard]] size_t   size()           const noexcept { return size_; }

    /** Número acumulado de rotações (simples + duplas). */
    [[nodiscard]] uint64_t rotation_count() const noexcept { return rotations_; }

    /**
     * Valida a invariante AVL em toda a árvore.
     * O(n) — use apenas em testes/QA, não em produção.
     * @return true se a árvore está corretamente balanceada.
     */
    [[nodiscard]] bool validate() const noexcept;

    /** Remove todos os nós e devolve memória ao pool interno. */
    void clear() noexcept;

private:
    // ── Nó interno ───────────────────────────────────────────────────────

    struct Node {
        PacketRule rule;
        Node*      left;
        Node*      right;
        int        height; // altura da sub-árvore enraizada aqui (folha = 1)

        explicit Node(const PacketRule& r) noexcept
            : rule(r), left(nullptr), right(nullptr), height(1) {}
    };

    Node*    root_;
    size_t   size_;
    uint64_t rotations_;

    // Pool de nós reciclados: evita pressão no alocador global.
    std::vector<Node*> free_list_;

    // ── Alocação de nós ──────────────────────────────────────────────────

    Node* alloc_node(const PacketRule& rule);
    void  release_node(Node* n) noexcept; // devolve ao pool (não destrói)

    // ── Helpers de altura e fator de balanceamento (inline crítico) ──────

    static inline int node_height(const Node* n) noexcept {
        return n ? n->height : 0;
    }

    static inline int balance_factor(const Node* n) noexcept {
        return node_height(n->left) - node_height(n->right);
    }

    static inline void update_height(Node* n) noexcept {
        const int lh = node_height(n->left);
        const int rh = node_height(n->right);
        n->height = 1 + (lh > rh ? lh : rh);
    }

    // ── Rotações ─────────────────────────────────────────────────────────

    Node* rotate_right(Node* y) noexcept; // rotação simples à direita
    Node* rotate_left (Node* x) noexcept; // rotação simples à esquerda

    /**
     * Reequilibra o nó n após inserção/remoção.
     * Aplica a rotação necessária (LL, RR, LR, RL) e retorna a nova raiz
     * do sub-nó.
     */
    Node* rebalance(Node* n) noexcept;

    // ── Recursão principal ───────────────────────────────────────────────

    Node* insert_rec(Node* n, const PacketRule& rule, bool& inserted);
    Node* remove_rec(Node* n, uint32_t id, bool& removed) noexcept;

    /** Retorna o nó de menor chave na sub-árvore (usado na remoção). */
    static Node* min_node(Node* n) noexcept;

    // ── Busca ────────────────────────────────────────────────────────────

    static const PacketRule* search_rec(const Node* n, uint32_t id) noexcept;

    // ── QA / validação ───────────────────────────────────────────────────

    static bool validate_rec(const Node* n, int& out_height) noexcept;

    // ── Liberação em massa ───────────────────────────────────────────────

    void destroy_rec(Node* n) noexcept;
};

} // namespace sdn
