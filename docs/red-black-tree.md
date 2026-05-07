# RedBlackTree

Árvore Red-Black otimizada para armazenamento e busca de regras de roteamento SDN (`PacketRule`), indexadas pelo campo `id`.

---

## Complexidades

| Operação  | Caso médio | Pior caso     |
|-----------|-----------|---------------|
| `insert`  | O(log n)  | O(log n)      |
| `remove`  | O(log n)  | O(log n)      |
| `search`  | O(log n)  | O(log n)      |

Altura máxima garantida: `h ≤ 2 · log₂(n + 1)`.

Rotações por inserção: no máximo **2**.  
Rotações por remoção: no máximo **3**.

---

## As 5 propriedades Red-Black

A estrutura mantém as seguintes invariantes em todo momento:

1. Todo nó é **vermelho** ou **preto**.
2. A **raiz** é sempre **preta**.
3. Toda **folha** (nó NIL sentinela) é **preta**.
4. Os dois filhos de um nó **vermelho** são **pretos** (sem dois vermelhos consecutivos).
5. Todos os caminhos de um nó até suas folhas descendentes contêm o mesmo número de nós pretos (**black-height** uniforme).

---

## API pública

```cpp
RedBlackTree(size_t reserve = 0);
```
Constrói a árvore e aloca o nó NIL sentinela. `reserve` pré-aloca slots no pool interno — útil quando o volume esperado é conhecido.

---

```cpp
void insert(const PacketRule& rule);
```
Insere a regra. Se já existir um nó com o mesmo `id`, a regra é substituída (upsert). Chama `insert_fixup` para restaurar as propriedades 2 e 4.

---

```cpp
bool remove(uint32_t id) noexcept;
```
Remove o nó com o `id` informado. Retorna `true` se o nó existia, `false` caso contrário. Chama `remove_fixup` quando o nó removido era preto.

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
int      black_height()   const noexcept;
```
Métricas para benchmark e QA:
- `height()` — altura total da raiz.
- `size()` — número de regras armazenadas.
- `rotation_count()` — total acumulado de rotações desde a construção.
- `black_height()` — número de nós pretos em qualquer caminho raiz→NIL (verifica propriedade 5).

---

```cpp
bool validate() const noexcept;
```
Percorre toda a árvore verificando as 5 propriedades Red-Black. O(n) — use apenas em testes e QA, nunca em produção.

---

```cpp
void clear() noexcept;
```
Remove todos os nós, devolvendo a memória ao pool interno. O contador de rotações **não** é resetado.

---

## Pool de memória

A árvore mantém internamente um `free_list_` de nós reciclados — a mesma estratégia da AVLTree. Ao remover um nó, o bloco é retido no pool; ao inserir, o pool é consultado antes de chamar `new`. Isso elimina pressão no alocador global em cargas com muitos insert/remove intercalados.

O destrutor libera toda a memória real do pool e o nó NIL sentinela.

---

## NIL Sentinela

Em vez de usar `nullptr` para representar folhas, a árvore usa um único nó preto compartilhado (`nil_`). Todos os ponteiros `left`, `right` e `parent` de nós folha apontam para esse sentinela.

**Vantagem de desempenho:** elimina checagens de `nullptr` nos hot paths de rotação e fixup, reduzindo branch mispredictions em cargas intensas.

---

## Balanceamento pós-inserção (`insert_fixup`)

O novo nó é sempre inserido como **vermelho**. Se o pai também for vermelho, a propriedade 4 é violada e `insert_fixup` resolve com um dos três casos abaixo (espelhados para o lado direito):

| Caso | Condição                        | Ação                                      |
|------|---------------------------------|-------------------------------------------|
| 1    | Tio vermelho                    | Recolorir pai, tio e avô; subir ao avô   |
| 2    | Tio preto, z é filho direito    | `rotate_left(pai)` → converte para caso 3 |
| 3    | Tio preto, z é filho esquerdo   | Recolorir pai/avô + `rotate_right(avô)`  |

---

## Balanceamento pós-remoção (`remove_fixup`)

Só é chamado quando o nó removido era **preto** (remoção de nó vermelho nunca viola as propriedades). O nó `x` que herda a posição carrega um "extra de negritude" que precisa ser absorvido:

| Caso | Condição                                  | Ação                                          |
|------|-------------------------------------------|-----------------------------------------------|
| 1    | Irmão vermelho                            | Recolorir + `rotate_left(pai)` → casos 2/3/4 |
| 2    | Irmão preto, ambos os filhos pretos       | Recolorir irmão; subir `x`                   |
| 3    | Irmão preto, filho direito preto          | Recolorir + `rotate_right(irmão)` → caso 4   |
| 4    | Irmão preto, filho direito vermelho       | Recolorir + `rotate_left(pai)` → termina     |

---

## Comparativo AVL vs Red-Black

| Característica          | AVL                        | Red-Black                  |
|-------------------------|----------------------------|----------------------------|
| Altura máxima           | 1.44 · log₂(n+2)          | 2 · log₂(n+1)             |
| Rotações por inserção   | ≤ 2                        | ≤ 2                        |
| Rotações por remoção    | O(log n)                   | ≤ 3                        |
| Balanceamento           | Estrito (FB ≤ 1)           | Flexível (cor)             |
| Melhor para             | Leitura intensiva          | Escrita intensiva          |
| Custo de rebalanceamento| Mais rotações na remoção   | Mais recolorações          |

Em cenários SDN com inserção contínua de novas regras de fluxo, a Red-Black tende a ter menor latência de escrita por exigir menos rotações na remoção.

---

## Exemplo de uso

```cpp
#include "rbt/RedBlackTree.hpp"

sdn::RedBlackTree tree(/*reserve=*/1000);

tree.insert({42, 0xC0A80001, 0x0A000001, 10});
tree.insert({ 7, 0xC0A80002, 0x0A000002,  5});

const sdn::PacketRule* rule = tree.search(42);
if (rule) { /* usa rule->priority, rule->src_ip, etc. */ }

tree.remove(7);

// QA
assert(tree.validate());
assert(tree.black_height() > 0);
```

---

## Limitações

- Não é thread-safe. Acesso concorrente requer sincronização externa.
- Cópia (`copy constructor` / `operator=`) é desabilitada intencionalmente — use move semantics ou referências.
- `search` retorna ponteiro raw; o ponteiro se torna inválido se o nó for removido ou a árvore destruída.
- O nó NIL sentinela é alocado no heap; o destrutor é responsável por liberá-lo.
