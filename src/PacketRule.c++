#ifndef PACKET_RULE_HPP
#define PACKET_RULE_HPP

#include <string>
#include <iostream>

class PacketRule {
private:
    int id;
    std::string sourceIP;
    std::string destIP;
    int priority;

public:
    // Construtor padrão
    PacketRule() : id(0), sourceIP(""), destIP(""), priority(0) {}

    // Construtor completo
    PacketRule(int id, std::string src, std::string dst, int pr)
        : id(id), sourceIP(src), destIP(dst), priority(pr) {}

    // Getters
    int getId() const { return id; }
    std::string getSourceIP() const { return sourceIP; }
    std::string getDestIP() const { return destIP; }
    int getPriority() const { return priority; }

    // Setters (opcional, mas útil)
    void setId(int newId) { id = newId; }
    void setSourceIP(const std::string& src) { sourceIP = src; }
    void setDestIP(const std::string& dst) { destIP = dst; }
    void setPriority(int pr) { priority = pr; }

    // Operadores de comparação (ESSENCIAL pra árvore)
    bool operator<(const PacketRule& other) const {
        return id < other.id;
    }

    bool operator>(const PacketRule& other) const {
        return id > other.id;
    }

    bool operator==(const PacketRule& other) const {
        return id == other.id;
    }

    // Impressão (debug)
    void print() const {
        std::cout << "[ID: " << id
                  << ", Src: " << sourceIP
                  << ", Dst: " << destIP
                  << ", Priority: " << priority << "]\n";
    }
};

// Sobrecarga do << (pra usar cout direto)
inline std::ostream& operator<<(std::ostream& os, const PacketRule& rule) {
    os << "[ID: " << rule.getId()
       << ", Src: " << rule.getSourceIP()
       << ", Dst: " << rule.getDestIP()
       << ", Priority: " << rule.getPriority() << "]";
    return os;
}

#endif