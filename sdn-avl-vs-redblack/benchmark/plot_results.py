"""
plot_results.py — Integrante 2 (DevOps & SRE)
SDN-Scale: AVL vs Red-Black Tree
Gera os 5 gráficos obrigatórios a partir dos CSVs do benchmark.

Instalar dependências:
    pip install pandas matplotlib seaborn

Executar (dentro da pasta benchmark/):
    python plot_results.py
"""

import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os

# ── Configurações ──────────────────────────────────────────────────────────────

DATA_DIR   = "../data"
OUTPUT_DIR = "../data"
os.makedirs(OUTPUT_DIR, exist_ok=True)

COLORS = {"AVL": "#1f77b4", "RBT": "#d62728"}

# ── Carrega CSVs ───────────────────────────────────────────────────────────────

df_insert = pd.read_csv(f"{DATA_DIR}/insert_results.csv")
df_search = pd.read_csv(f"{DATA_DIR}/search_results.csv")
df_delete = pd.read_csv(f"{DATA_DIR}/delete_results.csv")

# ── Gráfico (a) — Volume × Tempo de Inserção ──────────────────────────────────

fig, ax = plt.subplots(figsize=(10, 5))
for struct, grp in df_insert.groupby("estrutura"):
    agg = grp.groupby("n")["tempo_ns"].mean()
    ax.plot(agg.index, agg.values, label=struct, color=COLORS[struct])
ax.set_title("(a) Volume × Tempo de Inserção")
ax.set_xlabel("Número de elementos inseridos")
ax.set_ylabel("Tempo acumulado (ns)")
ax.legend()
ax.grid(True, linestyle="--", alpha=0.5)
plt.tight_layout()
plt.savefig(f"{OUTPUT_DIR}/grafico_a_insercao.png", dpi=150)
print("Salvo: grafico_a_insercao.png")
plt.close()

# ── Gráfico (b) — Volume × Tempo de Busca ─────────────────────────────────────

fig, ax = plt.subplots(figsize=(8, 5))
agg = df_search.groupby("estrutura")["tempo_ns"].agg(["mean", "std"]).reset_index()
bars = ax.bar(agg["estrutura"], agg["mean"],
              yerr=agg["std"], capsize=6,
              color=[COLORS[s] for s in agg["estrutura"]])
ax.set_title("(b) Tempo Médio de Busca (10.000 buscas aleatórias)")
ax.set_xlabel("Estrutura")
ax.set_ylabel("Tempo médio por busca (ns)")
ax.grid(True, axis="y", linestyle="--", alpha=0.5)
plt.tight_layout()
plt.savefig(f"{OUTPUT_DIR}/grafico_b_busca.png", dpi=150)
print("Salvo: grafico_b_busca.png")
plt.close()

# ── Gráfico (c) — Volume × Rotações Acumuladas ────────────────────────────────

fig, ax = plt.subplots(figsize=(10, 5))
for struct, grp in df_insert.groupby("estrutura"):
    agg = grp.groupby("n")["rotacoes"].mean()
    ax.plot(agg.index, agg.values, label=struct, color=COLORS[struct])
ax.set_title("(c) Volume × Rotações Acumuladas")
ax.set_xlabel("Número de elementos inseridos")
ax.set_ylabel("Rotações acumuladas")
ax.legend()
ax.grid(True, linestyle="--", alpha=0.5)
plt.tight_layout()
plt.savefig(f"{OUTPUT_DIR}/grafico_c_rotacoes.png", dpi=150)
print("Salvo: grafico_c_rotacoes.png")
plt.close()

# ── Gráfico (d) — Boxplot de tempos por operação ──────────────────────────────

df_insert["operacao"] = "insert"
df_search["operacao"] = "search"
df_delete["operacao"] = "delete"
df_all = pd.concat([df_insert, df_search, df_delete], ignore_index=True)

fig, ax = plt.subplots(figsize=(10, 6))
sns.boxplot(data=df_all, x="operacao", y="tempo_ns", hue="estrutura",
            palette=COLORS, ax=ax)
ax.set_title("(d) Boxplot de Tempos por Operação")
ax.set_xlabel("Operação")
ax.set_ylabel("Tempo (ns)")
ax.grid(True, axis="y", linestyle="--", alpha=0.5)
plt.tight_layout()
plt.savefig(f"{OUTPUT_DIR}/grafico_d_boxplot.png", dpi=150)
print("Salvo: grafico_d_boxplot.png")
plt.close()

# ── Gráfico (e) — Tempo de deleção × nós removidos ───────────────────────────

fig, ax = plt.subplots(figsize=(10, 5))
for struct, grp in df_delete.groupby("estrutura"):
    agg = grp.groupby("n")["tempo_ns"].mean()
    ax.plot(agg.index, agg.values, label=struct, color=COLORS[struct])
ax.set_title("(e) Tempo de Deleção × Nós Removidos")
ax.set_xlabel("Número de nós removidos")
ax.set_ylabel("Tempo acumulado (ns)")
ax.legend()
ax.grid(True, linestyle="--", alpha=0.5)
plt.tight_layout()
plt.savefig(f"{OUTPUT_DIR}/grafico_e_delecao.png", dpi=150)
print("Salvo: grafico_e_delecao.png")
plt.close()

print("\nTodos os gráficos gerados com sucesso!")