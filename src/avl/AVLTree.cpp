#include "AVLTree.hpp"

#include <cassert>
#include <stdexcept>
#include <utility>

namespace sdn {

// ─────────────────────────────────────────────────────────────────────────────
// Construção / destruição
// ─────────────────────────────────────────────────────────────────────────────

AVLTree::AVLTree(size_t reserve)
    : root_(nullptr), size_(0), rotations_(0)
{
    if (reserve > 0) {
        free_list_.reserve(reserve);
    }
}

AVLTree::~AVLTree() {
    clear();
    // Libera memória real de todos os nós no pool
    for (Node* n : free_list_) {
        delete n;
    }
}

AVLTree::AVLTree(AVLTree&& o) noexcept
    : root_(o.root_)
    , size_(o.size_)
    , rotations_(o.rotations_)
    , free_list_(std::move(o.free_list_))
{
    o.root_      = nullptr;
    o.size_      = 0;
    o.rotations_ = 0;
}

AVLTree& AVLTree::operator=(AVLTree&& o) noexcept {
    if (this != &o) {
        clear();
        for (Node* n : free_list_) delete n;

        root_      = o.root_;
        size_      = o.size_;
        rotations_ = o.rotations_;
        free_list_ = std::move(o.free_list_);

        o.root_      = nullptr;
        o.size_      = 0;
        o.rotations_ = 0;
    }
    return *this;
}

// ─────────────────────────────────────────────────────────────────────────────
// Pool de memória
// ─────────────────────────────────────────────────────────────────────────────

AVLTree::Node* AVLTree::alloc_node(const PacketRule& rule) {
    if (!free_list_.empty()) {
        // Reutiliza nó já alocado: evita chamada ao alocador global
        Node* n = free_list_.back();
        free_list_.pop_back();
        n->rule   = rule;
        n->left   = nullptr;
        n->right  = nullptr;
        n->height = 1;
        return n;
    }
    return new Node(rule); // apenas quando o pool está esgotado
}

void AVLTree::release_node(Node* n) noexcept {
    // Mantém o bloco de memória vivo para reutilização futura
    free_list_.push_back(n);
}

// ─────────────────────────────────────────────────────────────────────────────
// Rotações — núcleo do balanceamento AVL
// ─────────────────────────────────────────────────────────────────────────────

/**
 *      y                 x
 *     / \               / \
 *    x   T3    →       T1  y
 *   / \                   / \
 *  T1  T2               T2  T3
 */
AVLTree::Node* AVLTree::rotate_right(Node* y) noexcept {
    Node* x  = y->left;
    Node* T2 = x->right;

    x->right = y;
    y->left  = T2;

    update_height(y); // y desce: atualiza primeiro
    update_height(x); // x sobe: atualiza depois

    ++rotations_;
    return x;
}

/**
 *    x                   y
 *   / \                 / \
 *  T1   y     →        x   T3
 *      / \            / \
 *     T2  T3         T1  T2
 */
AVLTree::Node* AVLTree::rotate_left(Node* x) noexcept {
    Node* y  = x->right;
    Node* T2 = y->left;

    y->left  = x;
    x->right = T2;

    update_height(x);
    update_height(y);

    ++rotations_;
    return y;
}

/**
 * rebalance — aplica a rotação correta conforme o fator de balanceamento.
 *
 * Casos tratados (fb = fator de balanceamento do nó n):
 *   fb > +1, filho esq pesado à esq  → rotação simples direita  (LL)
 *   fb > +1, filho esq pesado à dir  → rotação dupla  esq+dir   (LR)
 *   fb < -1, filho dir pesado à dir  → rotação simples esquerda (RR)
 *   fb < -1, filho dir pesado à esq  → rotação dupla  dir+esq   (RL)
 */
AVLTree::Node* AVLTree::rebalance(Node* n) noexcept {
    update_height(n);
    const int fb = balance_factor(n);

    // ── Caso LL ──────────────────────────────────────────────────────────
    if (fb > 1 && balance_factor(n->left) >= 0) {
        return rotate_right(n);
    }
    // ── Caso LR ──────────────────────────────────────────────────────────
    if (fb > 1 && balance_factor(n->left) < 0) {
        n->left = rotate_left(n->left);
        return rotate_right(n);
    }
    // ── Caso RR ──────────────────────────────────────────────────────────
    if (fb < -1 && balance_factor(n->right) <= 0) {
        return rotate_left(n);
    }
    // ── Caso RL ──────────────────────────────────────────────────────────
    if (fb < -1 && balance_factor(n->right) > 0) {
        n->right = rotate_right(n->right);
        return rotate_left(n);
    }

    return n; // já balanceado
}

// ─────────────────────────────────────────────────────────────────────────────
// Inserção
// ─────────────────────────────────────────────────────────────────────────────

AVLTree::Node* AVLTree::insert_rec(Node* n, const PacketRule& rule, bool& inserted) {
    if (!n) {
        inserted = true;
        return alloc_node(rule);
    }

    if (rule.id < n->rule.id) {
        n->left  = insert_rec(n->left,  rule, inserted);
    } else if (rule.id > n->rule.id) {
        n->right = insert_rec(n->right, rule, inserted);
    } else {
        // id já existe: substitui a regra (upsert semântico)
        n->rule   = rule;
        inserted  = false;
        return n;
    }

    return rebalance(n);
}

void AVLTree::insert(const PacketRule& rule) {
    bool inserted = false;
    root_ = insert_rec(root_, rule, inserted);
    if (inserted) ++size_;
}

// ─────────────────────────────────────────────────────────────────────────────
// Busca
// ─────────────────────────────────────────────────────────────────────────────

const PacketRule* AVLTree::search_rec(const Node* n, uint32_t id) noexcept {
    // Loop iterativo: sem overhead de chamada recursiva, cache-friendly
    while (n) {
        if      (id < n->rule.id) n = n->left;
        else if (id > n->rule.id) n = n->right;
        else                      return &n->rule;
    }
    return nullptr;
}

const PacketRule* AVLTree::search(uint32_t id) const noexcept {
    return search_rec(root_, id);
}

// ─────────────────────────────────────────────────────────────────────────────
// Remoção
// ─────────────────────────────────────────────────────────────────────────────

AVLTree::Node* AVLTree::min_node(Node* n) noexcept {
    while (n->left) n = n->left;
    return n;
}

AVLTree::Node* AVLTree::remove_rec(Node* n, uint32_t id, bool& removed) noexcept {
    if (!n) return nullptr;

    if (id < n->rule.id) {
        n->left  = remove_rec(n->left,  id, removed);
    } else if (id > n->rule.id) {
        n->right = remove_rec(n->right, id, removed);
    } else {
        // Nó encontrado
        removed = true;

        if (!n->left || !n->right) {
            // Zero ou um filho: substitui diretamente
            Node* child = n->left ? n->left : n->right;
            release_node(n);
            return child; // pode ser nullptr (nó era folha)
        }

        // Dois filhos: substitui pelo sucessor in-order (mínimo da sub-dir)
        Node* successor = min_node(n->right);
        n->rule  = successor->rule;                              // copia dados
        n->right = remove_rec(n->right, successor->rule.id, removed); // remove sucessor
        removed  = true; // garante contagem correta após a recursão acima
    }

    return rebalance(n);
}

bool AVLTree::remove(uint32_t id) noexcept {
    bool removed = false;
    root_ = remove_rec(root_, id, removed);
    if (removed) --size_;
    return removed;
}

// ─────────────────────────────────────────────────────────────────────────────
// Métricas
// ─────────────────────────────────────────────────────────────────────────────

int AVLTree::height() const noexcept {
    return node_height(root_);
}

// ─────────────────────────────────────────────────────────────────────────────
// Validação da invariante AVL (uso exclusivo em QA / testes)
// ─────────────────────────────────────────────────────────────────────────────

bool AVLTree::validate_rec(const Node* n, int& out_height) noexcept {
    if (!n) {
        out_height = 0;
        return true;
    }

    int lh = 0, rh = 0;
    if (!validate_rec(n->left,  lh)) return false;
    if (!validate_rec(n->right, rh)) return false;

    const int fb = lh - rh;
    if (fb < -1 || fb > 1) return false; // violação da invariante

    // Verifica consistência da altura armazenada
    const int expected = 1 + (lh > rh ? lh : rh);
    if (n->height != expected) return false;

    out_height = expected;
    return true;
}

bool AVLTree::validate() const noexcept {
    int h = 0;
    return validate_rec(root_, h);
}

// ─────────────────────────────────────────────────────────────────────────────
// Limpeza
// ─────────────────────────────────────────────────────────────────────────────

void AVLTree::destroy_rec(Node* n) noexcept {
    if (!n) return;
    destroy_rec(n->left);
    destroy_rec(n->right);
    release_node(n); // devolve ao pool; não chama delete
}

void AVLTree::clear() noexcept {
    destroy_rec(root_);
    root_ = nullptr;
    size_ = 0;
    // rotations_ é acumulado — não resetar aqui intencionalmente
}

} // namespace sdn
