#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include "../models/PacketRule.hpp"

namespace sdn {

/**
 * RedBlackTree - Árvore Red-Black otimizada para roteamento SDN.
 *
 * 5 propriedades garantidas em todo momento:
 *   1. Todo nó é vermelho ou preto.
 *   2. A raiz é preta.
 *   3. Toda folha (NIL sentinela) é preta.
 *   4. Filhos de um nó vermelho são pretos.
 *   5. Todo caminho raiz→folha tem o mesmo número de nós pretos (black-height).
 *
 * Complexidades garantidas (n = nós na árvore):
 *   - Busca:    O(log n)   altura máxima ≤ 2·log₂(n+1)
 *   - Inserção: O(log n)   ≤ 2 rotações
 *   - Remoção:  O(log n)   ≤ 3 rotações
 *
 * Otimizações de desempenho:
 *   - Pool de nós com lista livre  → elimina overhead de malloc/free
 *   - Nó NIL sentinela compartilhado → elimina checagens nullptr em hot paths
 *   - Parent pointer + fixup iterativo → sem overhead de chamadas recursivas
 *   - Busca iterativa                 → sem overhead de pilha
 *   - noexcept em hot paths           → sem overhead de exception table
 *   - Sem herança virtual             → dispatch direto
 */
class RedBlackTree {
public:
    explicit RedBlackTree(size_t reserve = 0);
    ~RedBlackTree();

    RedBlackTree(const RedBlackTree&)            = delete;
    RedBlackTree& operator=(const RedBlackTree&) = delete;
    RedBlackTree(RedBlackTree&&)                 noexcept;
    RedBlackTree& operator=(RedBlackTree&&)      noexcept;

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

    /** Número acumulado de rotações (simples). */
    [[nodiscard]] uint64_t rotation_count() const noexcept { return rotations_; }

    /**
     * Black-height: número de nós pretos em qualquer caminho raiz→NIL.
     * Usado pelo QA para verificar propriedade 5.
     */
    [[nodiscard]] int black_height() const noexcept;

    /**
     * Valida as 5 propriedades Red-Black em toda a árvore.
     * O(n) — use apenas em testes/QA, não em produção.
     * @return true se a árvore está correta.
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
        Node*      parent;
        bool       red;   // true = vermelho, false = preto

        Node() noexcept
            : left(nullptr), right(nullptr), parent(nullptr), red(false) {}

        Node(const PacketRule& r, Node* nil) noexcept
            : rule(r), left(nil), right(nil), parent(nil), red(true) {}
    };

    Node*    nil_;       // sentinela NIL — sempre preto, compartilhado
    Node*    root_;
    size_t   size_;
    uint64_t rotations_;

    std::vector<Node*> free_list_;

    // ── Alocação de nós ──────────────────────────────────────────────────

    Node* alloc_node(const PacketRule& rule);
    void  release_node(Node* n) noexcept;

    // ── Rotações ─────────────────────────────────────────────────────────

    void rotate_left (Node* x) noexcept;
    void rotate_right(Node* y) noexcept;

    // ── Fixup pós-inserção / pós-remoção ─────────────────────────────────

    void insert_fixup(Node* z)           noexcept;
    void remove_fixup(Node* x)           noexcept;
    void transplant  (Node* u, Node* v)  noexcept;

    // ── Helpers ──────────────────────────────────────────────────────────

    static Node* min_node(Node* n, Node* nil) noexcept;

    int  height_rec  (const Node* n)              const noexcept;
    bool validate_rec(const Node* n, int& bh)     const noexcept;
    void destroy_rec (Node* n)                          noexcept;
};

} // namespace sdn
