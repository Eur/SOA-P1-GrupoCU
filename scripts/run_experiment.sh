#!/usr/bin/env bash
# Proportionality experiment (test cases 3 and 4).
# Step 1: runs the scheduler once per seed and stores one summary CSV per run.
#
# Usage: scripts/run_experiment.sh [output_dir]
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BINARY="$ROOT/lottery_scheduler"
OUT_DIR="${1:-$ROOT/results/experiment}"

SEEDS=30
DISPATCHES=10000
QUANTUM=1000

# case name -> input file
CASES=(
  "case3:$ROOT/tests/equal_shares_input_files/equal_shares_input_file.csv"
  "case4:$ROOT/tests/proportionality_input_files/proportionality_input_file.csv"
)

make -C "$ROOT" all >/dev/null

for entry in "${CASES[@]}"; do
  name="${entry%%:*}"
  input="${entry#*:}"
  mkdir -p "$OUT_DIR/$name"
  echo "Running $name ($SEEDS seeds, $DISPATCHES dispatches, quantum $QUANTUM)"
  for seed in $(seq 1 "$SEEDS"); do
    "$BINARY" --input "$input" --mode quantum --quantum "$QUANTUM" \
      --seed "$seed" --max-dispatches "$DISPATCHES" \
      --summary "$OUT_DIR/$name/summary_seed_$seed.csv" >/dev/null
  done
done

echo "Summaries written to $OUT_DIR"
