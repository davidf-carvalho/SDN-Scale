# SDN-Scale: AVL vs Red-Black  
## Otimização de Roteamento e Análise de Trade-offs  

**Disciplina:** Estrutura de Dados II  
**Professor:** Ricardo Sekeff  

---

## 1. Fundamentação Teórica: O Custo da Busca e do Balanceamento

No projeto de sistemas de alta disponibilidade, o tempo de resposta é ditado pela eficiência da busca em conjuntos massivos de dados. Em redes SDN, tabelas de fluxo utilizam o conceito de Longest Prefix Match ou busca por prioridade, onde o desempenho de uma Árvore Binária de Busca (BST) convencional é insuficiente devido à sua vulnerabilidade ao desbalanceamento.

---

### 1.1 Árvores AVL: O Rigor do Equilíbrio

A Árvore AVL (Adelson-Velsky e Landis) é uma estrutura estritamente balanceada. Sua invariante define que, para qualquer nó, a diferença de altura entre suas subárvores esquerda e direita (FB) deve satisfazer:


- **Custo de Busca:** O(log n), com altura máxima:  
  `h < 1.44 log₂(n + 2)`

- **Custo de Manutenção:**  
  Exige rotações frequentes → ideal para sistemas *read-intensive*.

---

### 1.2 Árvores Red-Black (RBT): O Equilíbrio Pragmático

Diferente da AVL, a Red-Black foca em um balanceamento mais flexível baseado em cores.

- **Custo de Busca:** O(log n), com altura:  
  `h ≤ 2 log₂(n + 1)`

- **Custo de Manutenção:**  
  Menos rotações, usa recoloração → ideal para sistemas *write-intensive*.

---

## 2. Cenário de Projeto: Roteamento de Borda em Tempo Real

Vocês devem atuar como engenheiros de infraestrutura otimizando um Load Balancer.  
O sistema atual sofre de latência excessiva quando novas regras são inseridas sequencialmente.

🎯 **Objetivo:**  
Comparar AVL vs Red-Black para encontrar a menor latência (em nanossegundos).

---

## 3. Atribuições do Grupo

### 👨‍💻 Integrante 1: Lead Software Engineer

**Missão:**
- Implementar:
  - `AVL_Router_Tree`
  - `RedBlack_Router_Tree`

**Desafios:**
- Gerenciar `PacketRule`:
  - ID
  - IP origem/destino
  - Prioridade

- Implementar:
  - Balanceamento AVL (|FB| ≤ 1)
  - Propriedades Red-Black (5 regras)

---

### ⚙️ Integrante 2: DevOps & SRE

**Missão:**
- Testes de carga

**Tarefas:**
- Mesma seed para ambas árvores
- Medir:
  - Insert
  - Search
  - Delete
- 100.000 entradas ordenadas
- Remover 20% dos nós

**Saída:**
- Gráficos (Volume x Tempo)

---

### 🧪 Integrante 3: QA & Analytics

**Missão:**
- Validar árvores

**Tarefas:**
- Verificar:
  - Altura AVL
  - Propriedades RBT
- Contar rotações
- Code Review obrigatório

---

## 4. Dinâmica de Integração

### 🔍 Code Review Obrigatório
- Integrante 1 não pode dar merge sem aprovação do Integrante 3

---

### 💥 Desafio da Deleção
- Remover 20% das regras
- Validar rebalanceamento

---

### 📊 Post-Mortem
Documento explicando:
- Problema
- Solução
- Impacto em negócio

Exemplo:
> "redução de latência de 50ms para 2ns"

---

## 5. Modelo de Entrega (Artigo Técnico)

⚠️ Obrigatório usar modelo SBC

---

### Estrutura sugerida:

- Introdução
- Fundamentação
- Metodologia
- Desenvolvimento
- Resultados
- Discussão
- Conclusão
- Referências
- Anexos (opcional)

---

## 6. Rúbrica de Avaliação

| Critério                  | Peso | Descrição |
|--------------------------|------|----------|
| Lógica AVL & RBT         | 4.0  | Balanceamento e ponteiros |
| Análise de Performance   | 3.0  | Precisão e testes |
| Gestão Git & Review      | 2.0  | Organização e histórico |
| Qualidade do Artigo      | 1.0  | Formatação e análise |

---

> "Na engenharia, não existe solução perfeita, apenas a solução correta para o problema certo."