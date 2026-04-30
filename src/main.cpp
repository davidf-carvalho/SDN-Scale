#include <iostream>
#include <iomanip>
#include <iterator>
#include "avl/AVLTree.hpp"

using namespace sdn;

static void print_rule(const char* label, const PacketRule* r) {
    if (r) {
        std::cout << label
                  << " id=" << r->id
                  << " src=" << std::hex << r->src_ip
                  << " dst=" << r->dst_ip
                  << " prio=" << std::dec << (int)r->priority
                  << "\n";
    } else {
        std::cout << label << " (não encontrado)\n";
    }
}

int main() {
    AVLTree tree(16);

    // Insere regras de fluxo SDN
    PacketRule rules[] = {
        {10, 0xC0A80001, 0x0A000001, 100},
        { 5, 0xC0A80002, 0x0A000002,  50},
        {20, 0xC0A80003, 0x0A000003, 200},
        { 3, 0xC0A80004, 0x0A000004,  10},
        { 7, 0xC0A80005, 0x0A000005,  75},
        {15, 0xC0A80006, 0x0A000006, 150},
        {25, 0xC0A80007, 0x0A000007, 250},
    };

    std::cout << "=== Inserindo " << std::size(rules) << " regras ===\n";
    for (auto& r : rules) tree.insert(r);

    std::cout << "Tamanho : " << tree.size()           << "\n";
    std::cout << "Altura  : " << tree.height()          << "\n";
    std::cout << "Rotacoes: " << tree.rotation_count()  << "\n";
    std::cout << "Valido  : " << (tree.validate() ? "sim" : "NAO") << "\n\n";

    std::cout << "=== Buscas ===\n";
    print_rule("id=10 ->", tree.search(10));
    print_rule("id= 5 ->", tree.search(5));
    print_rule("id=99 ->", tree.search(99));

    std::cout << "\n=== Removendo id=5 ===\n";
    bool ok = tree.remove(5);
    std::cout << "Removido: " << (ok ? "sim" : "nao") << "\n";
    print_rule("id= 5 ->", tree.search(5));
    std::cout << "Tamanho : " << tree.size()  << "\n";
    std::cout << "Valido  : " << (tree.validate() ? "sim" : "NAO") << "\n\n";

    std::cout << "=== Upsert id=10 (nova prioridade 255) ===\n";
    tree.insert({10, 0xC0A80001, 0x0A000001, 255});
    print_rule("id=10 ->", tree.search(10));
    std::cout << "Tamanho : " << tree.size() << " (deve ser igual — upsert)\n";

    return 0;
}
