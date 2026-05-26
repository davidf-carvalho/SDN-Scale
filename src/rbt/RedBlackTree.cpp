#include "RedBlackTree.hpp"

#include <stdexcept>
#include <utility>

namespace sdn {

// ─────────────────────────────────────────────────────────────────────────────
// Construção / destruição
// ─────────────────────────────────────────────────────────────────────────────

RedBlackTree::RedBlackTree(size_t reserve)
    : nil_(new Node()), root_(nil_), size_(0), rotations_(0)
{
    // NIL sentinela: preto, aponta para si mesmo
    nil_->red    = false;
    nil_->left   = nil_;
    nil_->right  = nil_;
    nil_->parent = nil_;

    if (reserve > 0) free_list_.reserve(reserve);
}

RedBlackTree::~RedBlackTree() {
    clear();
    for (Node* n : free_list_) delete n;
    delete nil_;
}

RedBlackTree::RedBlackTree(RedBlackTree&& o) noexcept
    : nil_(o.nil_), root_(o.root_), size_(o.size_),
      rotations_(o.rotations_), free_list_(std::move(o.free_list_))
{
    o.nil_       = nullptr;
    o.root_      = nullptr;
    o.size_      = 0;
    o.rotations_ = 0;
}

RedBlackTree& RedBlackTree::operator=(RedBlackTree&& o) noexcept {
    if (this != &o) {
        clear();
        for (Node* n : free_list_) delete n;
        delete nil_;

        nil_       = o.nil_;
        root_      = o.root_;
        size_      = o.size_;
        rotations_ = o.rotations_;
        free_list_ = std::move(o.free_list_);

        o.nil_       = nullptr;
        o.root_      = nullptr;
        o.size_      = 0;
        o.rotations_ = 0;
    }
    return *this;
}

// ─────────────────────────────────────────────────────────────────────────────
// Pool de memória
// ─────────────────────────────────────────────────────────────────────────────

RedBlackTree::Node* RedBlackTree::alloc_node(const PacketRule& rule) {
    if (!free_list_.empty()) {
        Node* n = free_list_.back();
        free_list_.pop_back();
        n->rule   = rule;
        n->left   = nil_;
        n->right  = nil_;
        n->parent = nil_;
        n->red    = true;
        return n;
    }
    return new Node(rule, nil_);
}

void RedBlackTree::release_node(Node* n) noexcept {
    free_list_.push_back(n);
}

// ─────────────────────────────────────────────────────────────────────────────
// Rotações
// ─────────────────────────────────────────────────────────────────────────────

// rotate_left(x):  x sobe à esquerda de y; y se torna raiz da sub-árvore.
void RedBlackTree::rotate_left(Node* x) noexcept {
    Node* y  = x->right;
    x->right = y->left;

    if (y->left != nil_) y->left->parent = x;

    y->parent = x->parent;
    if      (x->parent == nil_)        root_            = y;
    else if (x == x->parent->left)     x->parent->left  = y;
    else                               x->parent->right = y;

    y->left   = x;
    x->parent = y;
    ++rotations_;
}

// rotate_right(y): y sobe à direita de x; x se torna raiz da sub-árvore.
void RedBlackTree::rotate_right(Node* y) noexcept {
    Node* x  = y->left;
    y->left  = x->right;

    if (x->right != nil_) x->right->parent = y;

    x->parent = y->parent;
    if      (y->parent == nil_)        root_            = x;
    else if (y == y->parent->right)    y->parent->right = x;
    else                               y->parent->left  = x;

    x->right  = y;
    y->parent = x;
    ++rotations_;
}

// ─────────────────────────────────────────────────────────────────────────────
// Inserção
// ─────────────────────────────────────────────────────────────────────────────

/**
 * insert_fixup — restaura as propriedades 2 e 4 após inserção de z (vermelho).
 *
 * Três casos (espelhados para quando o pai está à direita do avô):
 *   Caso 1: tio vermelho  → recolorir pai, tio e avô; subir z ao avô.
 *   Caso 2: z é filho dir → rotate_left(pai) para converter em caso 3.
 *   Caso 3: z é filho esq → recolorir pai/avô + rotate_right(avô).
 */
void RedBlackTree::insert_fixup(Node* z) noexcept {
    while (z->parent->red) {
        if (z->parent == z->parent->parent->left) {
            Node* uncle = z->parent->parent->right;

            if (uncle->red) {                        // Caso 1
                z->parent->red         = false;
                uncle->red             = false;
                z->parent->parent->red = true;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {         // Caso 2
                    z = z->parent;
                    rotate_left(z);
                }
                z->parent->red         = false;      // Caso 3
                z->parent->parent->red = true;
                rotate_right(z->parent->parent);
            }
        } else {                                     // Espelho
            Node* uncle = z->parent->parent->left;

            if (uncle->red) {                        // Caso 1
                z->parent->red         = false;
                uncle->red             = false;
                z->parent->parent->red = true;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {          // Caso 2
                    z = z->parent;
                    rotate_right(z);
                }
                z->parent->red         = false;      // Caso 3
                z->parent->parent->red = true;
                rotate_left(z->parent->parent);
            }
        }
    }
    root_->red = false; // Prop 2
}

void RedBlackTree::insert(const PacketRule& rule) {
    Node* z = alloc_node(rule);
    Node* y = nil_;
    Node* x = root_;

    while (x != nil_) {
        y = x;
        if      (rule.id < x->rule.id) x = x->left;
        else if (rule.id > x->rule.id) x = x->right;
        else {
            // id já existe: upsert semântico
            x->rule = rule;
            release_node(z);
            return;
        }
    }

    z->parent = y;
    if      (y == nil_)            root_    = z;
    else if (rule.id < y->rule.id) y->left  = z;
    else                           y->right = z;

    insert_fixup(z);
    ++size_;
}

// ─────────────────────────────────────────────────────────────────────────────
// Busca (iterativa)
// ─────────────────────────────────────────────────────────────────────────────

const PacketRule* RedBlackTree::search(uint32_t id) const noexcept {
    const Node* x = root_;
    while (x != nil_) {
        if      (id < x->rule.id) x = x->left;
        else if (id > x->rule.id) x = x->right;
        else                      return &x->rule;
    }
    return nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
// Remoção
// ─────────────────────────────────────────────────────────────────────────────

void RedBlackTree::transplant(Node* u, Node* v) noexcept {
    if      (u->parent == nil_)       root_            = v;
    else if (u == u->parent->left)    u->parent->left  = v;
    else                              u->parent->right = v;
    v->parent = u->parent;
}

RedBlackTree::Node* RedBlackTree::min_node(Node* n, Node* nil) noexcept {
    while (n->left != nil) n = n->left;
    return n;
}

/**
 * remove_fixup — restaura propriedades após remoção de nó preto.
 * x é o nó que "herdou" a negritude extra (double-black).
 *
 * Quatro casos (espelhados para x à direita):
 *   Caso 1: irmão vermelho         → recolorir + rotacionar; converte para 2/3/4.
 *   Caso 2: irmão preto, filhos p. → recolorir irmão; subir x.
 *   Caso 3: irmão preto, fil.dir p.→ recolorir + rotate_right(irmão); converte p/ 4.
 *   Caso 4: irmão preto, fil.dir v.→ recolorir + rotate_left(pai); termina.
 */
void RedBlackTree::remove_fixup(Node* x) noexcept {
    while (x != root_ && !x->red) {
        if (x == x->parent->left) {
            Node* w = x->parent->right;

            if (w->red) {                               // Caso 1
                w->red         = false;
                x->parent->red = true;
                rotate_left(x->parent);
                w = x->parent->right;
            }
            if (!w->left->red && !w->right->red) {      // Caso 2
                w->red = true;
                x = x->parent;
            } else {
                if (!w->right->red) {                   // Caso 3
                    w->left->red = false;
                    w->red       = true;
                    rotate_right(w);
                    w = x->parent->right;
                }
                w->red         = x->parent->red;        // Caso 4
                x->parent->red = false;
                w->right->red  = false;
                rotate_left(x->parent);
                x = root_;
            }
        } else {                                        // Espelho
            Node* w = x->parent->left;

            if (w->red) {                               // Caso 1
                w->red         = false;
                x->parent->red = true;
                rotate_right(x->parent);
                w = x->parent->left;
            }
            if (!w->right->red && !w->left->red) {      // Caso 2
                w->red = true;
                x = x->parent;
            } else {
                if (!w->left->red) {                    // Caso 3
                    w->right->red = false;
                    w->red        = true;
                    rotate_left(w);
                    w = x->parent->left;
                }
                w->red         = x->parent->red;        // Caso 4
                x->parent->red = false;
                w->left->red   = false;
                rotate_right(x->parent);
                x = root_;
            }
        }
    }
    x->red = false;
}

bool RedBlackTree::remove(uint32_t id) noexcept {
    // Localiza o nó
    Node* z = root_;
    while (z != nil_) {
        if      (id < z->rule.id) z = z->left;
        else if (id > z->rule.id) z = z->right;
        else break;
    }
    if (z == nil_) return false;

    Node* y              = z;
    bool  y_original_red = y->red;
    Node* x;

    if (z->left == nil_) {
        x = z->right;
        transplant(z, z->right);
    } else if (z->right == nil_) {
        x = z->left;
        transplant(z, z->left);
    } else {
        // Dois filhos: substitui pelo sucessor in-order
        y              = min_node(z->right, nil_);
        y_original_red = y->red;
        x              = y->right;

        if (y->parent == z) {
            x->parent = y;
        } else {
            transplant(y, y->right);
            y->right         = z->right;
            y->right->parent = y;
        }
        transplant(z, y);
        y->left         = z->left;
        y->left->parent = y;
        y->red          = z->red;
    }

    release_node(z);
    --size_;

    if (!y_original_red) remove_fixup(x);

    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Métricas
// ─────────────────────────────────────────────────────────────────────────────

int RedBlackTree::height_rec(const Node* n) const noexcept {
    if (n == nil_) return 0;
    const int lh = height_rec(n->left);
    const int rh = height_rec(n->right);
    return 1 + (lh > rh ? lh : rh);
}

int RedBlackTree::height() const noexcept {
    return height_rec(root_);
}

int RedBlackTree::black_height() const noexcept {
    int bh    = 0;
    const Node* x = root_;
    while (x != nil_) {
        if (!x->red) ++bh;
        x = x->left;
    }
    return bh;
}

// ─────────────────────────────────────────────────────────────────────────────
// Validação das 5 propriedades RBT (uso exclusivo em QA / testes)
// ─────────────────────────────────────────────────────────────────────────────

bool RedBlackTree::validate_rec(const Node* n, int& bh) const noexcept {
    if (n == nil_) { bh = 0; return true; }

    // Prop 4: nó vermelho não pode ter filho vermelho
    if (n->red && (n->left->red || n->right->red)) return false;

    int lbh = 0, rbh = 0;
    if (!validate_rec(n->left,  lbh)) return false;
    if (!validate_rec(n->right, rbh)) return false;

    // Prop 5: black-height igual em ambos os lados
    if (lbh != rbh) return false;

    bh = lbh + (n->red ? 0 : 1);
    return true;
}

bool RedBlackTree::validate() const noexcept {
    if (!nil_ || nil_->red) return false;          // Prop 3
    if (root_ == nullptr) return false;
    if (root_ != nil_ && root_->red) return false; // Prop 2
    int bh = 0;
    return validate_rec(root_, bh);
}

// ─────────────────────────────────────────────────────────────────────────────
// Limpeza
// ─────────────────────────────────────────────────────────────────────────────

void RedBlackTree::destroy_rec(Node* n) noexcept {
    if (n == nil_) return;
    destroy_rec(n->left);
    destroy_rec(n->right);
    release_node(n);
}

void RedBlackTree::clear() noexcept {
    destroy_rec(root_);
    root_ = nil_;
    size_ = 0;
    // rotations_ é acumulado — não resetar intencionalmente
}

void validateRBT(const RedBlackTree& tree) {
    if (!tree.validate()) {
        throw std::logic_error("Red-Black Tree invariant violation");
    }
}

} // namespace sdn
