#pragma once
#include <cstdint>
#include <cstring>

namespace sdn {

/**
 * PacketRule - Regra de roteamento SDN.
 * Chave de ordenação: id (uint32_t).
 */
struct PacketRule {
    uint32_t id;           // chave única
    uint32_t ip_src;       // IP origem  (formato compacto)
    uint32_t ip_dst;       // IP destino (formato compacto)
    uint8_t  priority;     // prioridade 0–255

    PacketRule() noexcept : id(0), ip_src(0), ip_dst(0), priority(0) {}

    PacketRule(uint32_t id, uint32_t src, uint32_t dst, uint8_t prio) noexcept
        : id(id), ip_src(src), ip_dst(dst), priority(prio) {}
};

} // namespace sdn