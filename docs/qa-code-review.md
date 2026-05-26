# QA Code Review - AVL vs Red-Black

## Status

Aprovacao tecnica condicionada ao build do alvo `tests` e execucao completa do GoogleTest em ambiente com CMake/compilador C++ disponivel.

## Pontos revisados

- A AVL ja possuia validacao parcial; a revisao formalizou `validateAVL(const AVLTree&)` com falha por excecao e manteve `validate() noexcept` para uso em demo/benchmark.
- A Red-Black ja cobria raiz preta, regra vermelho-vermelho e black-height uniforme; a revisao formalizou `validateRBT(const RedBlackTree&)` e tornou explicita a checagem do NIL preto.
- A duplicidade dos helpers inline da AVL foi removida de `AVLTree.cpp`, evitando conflito com as definicoes do header.
- O alvo `tests` foi integrado ao CMake com GoogleTest e casos pequenos que validam invariantes apos insercoes, upsert e remocoes basicas.

## Analise comparativa

- AVL: melhor quando a carga e majoritariamente de busca, porque mantem altura mais restrita com fator de balanceamento `|FB| <= 1`.
- Red-Black: melhor quando ha maior volume de insercoes/remocoes, porque aceita altura um pouco maior em troca de menos manutencao estrutural e uso de recoloracao.
- No contexto de Load Balancer SDN, a escolha deve acompanhar o perfil real da tabela de regras: leitura intensiva favorece AVL; atualizacao continua de regras favorece Red-Black.
