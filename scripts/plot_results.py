#!/usr/bin/env python3
import argparse
import csv
import math
from collections import defaultdict
from pathlib import Path

try:
    import matplotlib.pyplot as plt
except ModuleNotFoundError as exc:
    raise SystemExit(
        "matplotlib is required to generate plots. Install it with: "
        "python -m pip install -r requirements.txt"
    ) from exc


def parse_args():
    parser = argparse.ArgumentParser(description="Generate SDN-Scale benchmark plots.")
    parser.add_argument("--input", default="results/benchmark_raw.csv")
    parser.add_argument("--output-dir", default="results/plots")
    return parser.parse_args()


def load_rows(path):
    rows = []
    with open(path, newline="", encoding="utf-8") as handle:
        for row in csv.DictReader(handle):
            row["n"] = int(row["n"])
            row["tempo_ns"] = int(row["tempo_ns"])
            row["rotacoes"] = int(row["rotacoes"])
            row["run_id"] = int(row["run_id"])
            rows.append(row)
    if not rows:
        raise SystemExit(f"no rows found in {path}")
    return rows


def mean(values):
    return sum(values) / len(values)


def stdev(values):
    if len(values) < 2:
        return 0.0
    avg = mean(values)
    variance = sum((value - avg) ** 2 for value in values) / (len(values) - 1)
    return math.sqrt(variance)


def grouped_series(rows, operation, metric):
    grouped = defaultdict(list)
    for row in rows:
        if row["operacao"] == operation:
            key = (row["scenario"], row["estrutura"], row["n"])
            grouped[key].append(row[metric])

    series = defaultdict(list)
    for (scenario, structure, n), values in grouped.items():
        series[(scenario, structure)].append((n, mean(values), stdev(values)))

    for values in series.values():
        values.sort(key=lambda item: item[0])
    return series


def plot_line(rows, operation, metric, ylabel, title, output_path):
    plt.figure(figsize=(10, 6))
    for (scenario, structure), values in sorted(grouped_series(rows, operation, metric).items()):
        xs = [item[0] for item in values]
        ys = [item[1] for item in values]
        plt.plot(xs, ys, marker="o", linewidth=1.8, markersize=3, label=f"{scenario} - {structure}")
    plt.title(title)
    plt.xlabel("Volume (n)")
    plt.ylabel(ylabel)
    plt.grid(True, alpha=0.25)
    plt.legend()
    plt.tight_layout()
    plt.savefig(output_path, dpi=160)
    plt.close()


def final_operation_values(rows):
    grouped = defaultdict(list)
    max_n = {}
    for row in rows:
        key = (row["scenario"], row["operacao"], row["estrutura"], row["run_id"])
        max_n[key] = max(max_n.get(key, 0), row["n"])

    for row in rows:
        key = (row["scenario"], row["operacao"], row["estrutura"], row["run_id"])
        if row["n"] == max_n[key]:
            label = f"{row['operacao']}\n{row['scenario']}\n{row['estrutura']}"
            grouped[label].append(row["tempo_ns"])
    return grouped


def plot_boxplot(rows, output_path):
    grouped = final_operation_values(rows)
    labels = sorted(grouped)
    values = [grouped[label] for label in labels]
    plt.figure(figsize=(12, 6))
    plt.boxplot(values, labels=labels, showmeans=True)
    plt.title("Distribuicao dos tempos finais por operacao")
    plt.xlabel("Operacao / cenario / estrutura")
    plt.ylabel("Tempo (ns)")
    plt.xticks(rotation=35, ha="right")
    plt.grid(True, axis="y", alpha=0.25)
    plt.tight_layout()
    plt.savefig(output_path, dpi=160)
    plt.close()


def main():
    args = parse_args()
    rows = load_rows(args.input)
    output_dir = Path(args.output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    plot_line(
        rows,
        "insert",
        "tempo_ns",
        "Tempo acumulado (ns)",
        "Volume x Tempo de Insercao",
        output_dir / "insert_time.png",
    )
    plot_line(
        rows,
        "search",
        "tempo_ns",
        "Tempo acumulado (ns)",
        "Volume x Tempo de Busca",
        output_dir / "search_time.png",
    )
    plot_line(
        rows,
        "insert",
        "rotacoes",
        "Rotacoes acumuladas",
        "Volume x Rotacoes Acumuladas",
        output_dir / "rotations.png",
    )
    plot_line(
        rows,
        "delete",
        "tempo_ns",
        "Tempo acumulado (ns)",
        "Delecoes x Tempo",
        output_dir / "delete_time.png",
    )
    plot_boxplot(rows, output_dir / "operation_boxplot.png")

    print(f"plots written to {output_dir}")


if __name__ == "__main__":
    main()
