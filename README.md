# SDN-Scale

Comparative study of AVL Tree and Red-Black Tree for SDN (Software Defined Networking) packet rule management.

## Build

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

## Run

```bash
./build/bin/main
./build/bin/tests
./build/bin/benchmark
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
| `docs/` | Reports and notes |
