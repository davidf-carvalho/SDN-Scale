#include <iostream>
#include <iomanip>
#include <iterator>
#include "avl/AVLTree.hpp"
#include "rbt/RedBlackTree.hpp"

using namespace sdn;

static PacketRule rules[] = {
    {10, 0xC0A80001, 0x0A000001, 100},
    { 5, 0xC0A80002, 0x0A000002,  50},
    {20, 0xC0A80003, 0x0A000003, 200},
    { 3, 0xC0A80004, 0x0A000004,  10},
    { 7, 0xC0A80005, 0x0A000005,  75},
    {15, 0xC0A80006, 0x0A000006, 150},
    {25, 0xC0A80007, 0x0A000007, 250},
};

static void print_rule(const char* label, const PacketRule* r) {
    if (r)
        std::cout << label
                  << " id=" << r->id
                  << " src=0x" << std::hex << r->src_ip
                  << " dst=0x" << r->dst_ip
                  << " prio=" << std::dec << (int)r->priority
                  << "\n";
    else
        std::cout << label << " (nao encontrado)\n";
}

template<typename Tree>
static void demo(const char* name, Tree& tree) {
    std::cout << "╔══════════════════════════════════════╗\n";
    std::cout << "║  " << std::left << std::setw(36) << name << "║\n";
    std::cout << "╚══════════════════════════════════════╝\n";

    std::cout << ">>> Inserindo " << std::size(rules) << " regras\n";
    for (auto& r : rules) tree.insert(r);

    std::cout << "Tamanho  : " << tree.size()           << "\n";
    std::cout << "Altura   : " << tree.height()          << "\n";
    std::cout << "Rotacoes : " << tree.rotation_count()  << "\n";
    std::cout << "Valido   : " << (tree.validate() ? "sim" : "NAO") << "\n\n";

    std::cout << ">>> Buscas\n";
    print_rule("id=10 ->", tree.search(10));
    print_rule("id= 5 ->", tree.search( 5));
    print_rule("id=99 ->", tree.search(99));

    std::cout << "\n>>> Removendo id=5\n";
    std::cout << "Removido : " << (tree.remove(5) ? "sim" : "nao") << "\n";
    print_rule("id= 5 ->", tree.search(5));
    std::cout << "Tamanho  : " << tree.size()  << "\n";
    std::cout << "Valido   : " << (tree.validate() ? "sim" : "NAO") << "\n\n";

    std::cout << ">>> Upsert id=10 (prio=255)\n";
    tree.insert({10, 0xC0A80001, 0x0A000001, 255});
    print_rule("id=10 ->", tree.search(10));
    std::cout << "Tamanho  : " << tree.size() << " (deve ser igual — upsert)\n\n";
}

int main() {
    AVLTree      avl(16);
    RedBlackTree rbt(16);

    demo("AVL Tree", avl);
    demo("Red-Black Tree", rbt);

    std::cout << "╔══════════════════════════════════════╗\n";
    std::cout << "║  Comparativo                         ║\n";
    std::cout << "╚══════════════════════════════════════╝\n";
    std::cout << std::left
              << std::setw(12) << "Metrica"
              << std::setw(10) << "AVL"
              << std::setw(10) << "RBT" << "\n";
    std::cout << std::string(32, '-') << "\n";
    std::cout << std::setw(12) << "Altura"
              << std::setw(10) << avl.height()
              << std::setw(10) << rbt.height() << "\n";
    std::cout << std::setw(12) << "Rotacoes"
              << std::setw(10) << avl.rotation_count()
              << std::setw(10) << rbt.rotation_count() << "\n";
    std::cout << std::setw(12) << "Tamanho"
              << std::setw(10) << avl.size()
              << std::setw(10) << rbt.size() << "\n";

    return 0;
}
