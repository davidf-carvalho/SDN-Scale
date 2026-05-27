# SDN-Scale

Comparative study of AVL Tree and Red-Black Tree for SDN (Software Defined Networking) packet rule management.

## Build

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

Install Python plotting dependencies:

```bash
python -m pip install -r requirements.txt
```

## Run

```bash
./build/bin/main
./build/bin/tests
./build/bin/benchmark --n 100000 --runs 30 --seed 42
python scripts/plot_results.py --input results/benchmark_raw.csv
```

For a quick benchmark smoke test:

```bash
./build/bin/benchmark --n 1000 --runs 2 --step 100 --searches 200 --deletions 200
python scripts/plot_results.py
```

## Structure

| Path | Description |
|------|-------------|
| `src/avl/` | AVL Tree implementation |
| `src/rbt/` | Red-Black Tree implementation |
| `src/models/` | PacketRule model header |
| `src/PacketRule.c++` | PacketRule class (id, IPs, priority) |
| `tests/` | Unit tests |
| `benchmark/` | Performance benchmarks (100k ops) |
| `scripts/` | Plot generation from benchmark CSVs |
| `results/` | Raw CSVs and generated benchmark plots |
| `docs/` | Reports and notes |
