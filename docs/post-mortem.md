# Relatorio Post-Mortem Executivo - SDN-Scale

**Data da medicao:** 29/05/2026   
**Sistema avaliado:** Load Balancer SDN com tabela de regras `PacketRule`  
**Status:** resolvido em ambiente de benchmark; recomendacao pronta para decisao.

## 1. Incidente / contexto

O sistema simulado de roteamento SDN precisa inserir, buscar e remover regras de pacote em tempo real. A atividade identificou um risco de latencia quando muitas regras sao inseridas em sequencia: se a tabela interna ficar mal organizada, cada busca passa a exigir mais passos e o Load Balancer demora mais para decidir o encaminhamento.

Para tratar esse risco, comparamos duas estrategias de organizacao da tabela:

- **AVL:** mantem a tabela mais rigidamente organizada, favorecendo busca.
- **Red-Black (RBT):** aceita uma organizacao um pouco mais flexivel, reduzindo o custo de manutencao em insercoes e remocoes.

Em termos executivos: o problema nao era falta de capacidade funcional; era escolher a estrutura com menor custo operacional para manter a tabela organizada enquanto as regras mudam.

## 2. Impacto observado

No benchmark principal, a RBT teve menor tempo medio em 5 das 6 comparacoes finais. A AVL ficou levemente melhor apenas em busca no cenario ordenado.

Isso indica que, para uma tabela SDN com atualizacao frequente de regras, a Red-Black reduz a latencia total de escrita e remocao sem quebrar as garantias de consistencia.

## 3. Causa raiz em linguagem nao tecnica

A tabela precisa permanecer organizada para que uma regra seja encontrada rapidamente. Sempre que novas regras entram ou saem, a estrutura paga um custo para reorganizar essa tabela.

Tecnicamente, esse custo aparece como rotacoes, rebalanceamento e validacao de invariantes. Em linguagem de negocio:

- **Rotacao / rebalanceamento:** custo operacional para manter a tabela organizada.
- **Altura da arvore:** quantidade de passos ate encontrar uma regra.
- **Invariantes AVL/RBT:** garantias de consistencia apos mudancas.

A AVL controla esse equilibrio de forma mais rigorosa. Isso ajuda em busca, mas pode aumentar o custo quando ha muitas mudancas. A RBT usa uma regra de equilibrio mais pragmatica e, nos testes deste projeto, entregou melhor tempo geral para insercao e remocao.

## 4. Acao corretiva

Foram executados testes unitarios e benchmark com os dois modelos:

```powershell
build\bin\Debug\tests.exe
build\bin\Debug\benchmark.exe --n 100000 --runs 30 --seed 42 --output results\benchmark_raw.csv
```

Parametros da medicao:

| Parametro | Valor |
|---|---:|
| Regras por execucao | 100.000 |
| Repeticoes | 30 |
| Seed | 42 |
| Buscas por execucao | 10.000 |
| Remocoes por execucao | 20.000 (20%) |
| Cenarios | `ordered`, `random` |
| Fonte dos dados | `results/benchmark_raw.csv` |
| Graficos | `results/plots/` |

Os testes unitarios passaram: **8/8**.

## 5. Resultado medido

Tempos medios finais por operacao:

| Cenario | Operacao | AVL media | RBT media | Resultado executivo |
|---|---|---:|---:|---|
| ordered | insert | 75,600 ms | 45,347 ms | RBT 40,0% menor |
| ordered | search | 5,396 ms | 5,493 ms | AVL 1,8% menor |
| ordered | delete | 24,784 ms | 15,797 ms | RBT 36,3% menor |
| random | insert | 145,724 ms | 92,664 ms | RBT 36,4% menor |
| random | search | 5,719 ms | 5,154 ms | RBT 9,9% menor |
| random | delete | 29,071 ms | 16,243 ms | RBT 44,1% menor |

Rotacoes medias finais:

| Cenario | Operacao | AVL | RBT | Leitura executiva |
|---|---|---:|---:|---|
| ordered | insert | 99.983 | 99.969 | custo estrutural parecido; RBT ainda foi mais rapida |
| ordered | delete | 826 | 1.020 | RBT teve mais rotacoes, mas menor tempo total |
| random | insert | 69.974 | 58.426 | RBT exigiu menos manutencao |
| random | delete | 7.406 | 6.679 | RBT exigiu menos manutencao |

As operacoes de busca nao geram rotacoes, porque apenas consultam a tabela.

## 6. Decisao recomendada

Recomendamos usar **Red-Black Tree como estrutura padrao** para o Load Balancer SDN quando o perfil da carga envolver insercoes e remocoes frequentes de regras.

Justificativa executiva:

- reduziu o tempo medio de insercao em **40,0%** no cenario ordenado;
- reduziu o tempo medio de insercao em **36,4%** no cenario aleatorio;
- reduziu o tempo medio de remocao em ate **44,1%**;
- manteve validacao estrutural aprovada nos testes automatizados;
- perdeu apenas **1,8%** em busca no cenario ordenado, diferenca pequena diante do ganho nas operacoes de atualizacao.

Para uma operacao SDN dinamica, a RBT entrega melhor equilibrio entre desempenho e custo de manutencao. A AVL continua sendo uma alternativa valida quando a carga for majoritariamente de leitura e as regras mudarem pouco.