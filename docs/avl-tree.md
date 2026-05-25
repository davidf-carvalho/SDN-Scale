# AVLTree

Árvore AVL otimizada para armazenamento e busca de regras de roteamento SDN (`PacketRule`), indexadas pelo campo `id`.

---

## Complexidades

| Operação  | Caso médio | Pior caso     |
|-----------|-----------|---------------|
| `insert`  | O(log n)  | O(log n)      |
| `remove`  | O(log n)  | O(log n)      |
| `search`  | O(log n)  | O(log n)      |

Altura máxima garantida: `h < 1.44 · log₂(n + 2)`.

Rotações por inserção: no máximo 1 dupla ou 2 simples.  
Rotações por remoção: O(log n) no pior caso.

---

## API pública

```cpp
AVLTree(size_t reserve = 0);
```
Constrói a árvore. `reserve` pré-aloca slots no pool interno de nós — útil quando o volume esperado é conhecido.

---

```cpp
void insert(const PacketRule& rule);
```
Insere a regra. Se já existir um nó com o mesmo `id`, a regra é substituída (upsert).

---

```cpp
bool remove(uint32_t id) noexcept;
```
Remove o nó com o `id` informado. Retorna `true` se o nó existia, `false` caso contrário.

---

```cpp
const PacketRule* search(uint32_t id) const noexcept;
```
Busca iterativa. Retorna ponteiro para a regra (válido enquanto a árvore existir e o nó não for removido) ou `nullptr` se não encontrada.

---

```cpp
int      height()         const noexcept;
size_t   size()           const noexcept;
uint64_t rotation_count() const noexcept;
```
Métricas para benchmark: altura da raiz, número de regras armazenadas, e total acumulado de rotações desde a construção.

---

```cpp
bool validate() const noexcept;
```
Percorre toda a árvore verificando a invariante AVL e a consistência das alturas armazenadas. O(n) — use apenas em testes.

---

```cpp
void clear() noexcept;
```
Remove todos os nós, devolvendo a memória ao pool interno. O contador de rotações **não** é resetado.

---

## Pool de memória

A árvore mantém internamente um `free_list_` de nós reciclados. Ao remover um nó, o bloco de memória é retido no pool; ao inserir, o pool é consultado antes de chamar `new`. Isso elimina pressão no alocador global em cargas com muitos insert/remove intercalados.

O destrutor libera toda a memória real do pool.

---

## Balanceamento

Após cada inserção ou remoção, `rebalance()` é chamado no caminho de volta da recursão. Os quatro casos clássicos são tratados:

| Caso | Condição                          | Rotação aplicada       |
|------|-----------------------------------|------------------------|
| LL   | fb > 1, filho esq. não pesa à dir | Simples direita        |
| LR   | fb > 1, filho esq. pesa à dir     | Esquerda + direita     |
| RR   | fb < -1, filho dir. não pesa à esq| Simples esquerda       |
| RL   | fb < -1, filho dir. pesa à esq    | Direita + esquerda     |

---

## Exemplo de uso

```cpp
#include "avl/AVLTree.hpp"

sdn::AVLTree tree(/*reserve=*/1000);

tree.insert({.id = 42, .src_ip = 0xC0A80001, .dst_ip = 0x0A000001, .priority = 10});
tree.insert({.id = 7,  .src_ip = 0xC0A80002, .dst_ip = 0x0A000002, .priority = 5});

const sdn::PacketRule* rule = tree.search(42);
if (rule) { /* usa rule->priority, rule->src_ip, etc. */ }

tree.remove(7);

// QA
assert(tree.validate());
```

---

## Limitações

- Não é thread-safe. Acesso concorrente requer sincronização externa.
- Cópia (`copy constructor` / `operator=`) é desabilitada intencionalmente — use move semântics ou referências.
- `search` retorna ponteiro raw; o ponteiro se torna inválido se o nó for removido ou a árvore destruída.
