#pragma once

#include <cstdint>
#include <string>

namespace sdn {

/**
 * PacketRule - Regra de fluxo SDN gerenciada pelo Load Balancer.
 *
 * Representa uma entrada na tabela de fluxo: define como pacotes
 * que casam com src_ip/dst_ip devem ser tratados, ordenados por
 * prioridade (maior valor = maior prioridade de correspondência).
 */
struct PacketRule {
    uint32_t id;        // Identificador único da regra
    uint32_t src_ip;    // IP de origem  (ex: 192.168.0.1 → 0xC0A80001)
    uint32_t dst_ip;    // IP de destino (ex: 10.0.0.1    → 0x0A000001)
    uint8_t  priority;  // Prioridade: 0 (baixa) … 255 (alta)

    /** Ordem natural: por id (chave única da BST). */
    bool operator<(const PacketRule& o)  const noexcept { return id < o.id;  }
    bool operator>(const PacketRule& o)  const noexcept { return id > o.id;  }
    bool operator==(const PacketRule& o) const noexcept { return id == o.id; }
};

} // namespace sdn
