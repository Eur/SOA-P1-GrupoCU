#!/usr/bin/env bash
# Proportionality experiment (test cases 3 and 4).
# Runs the scheduler once per seed and collects the raw data in CSV files.
# No statistics are computed here: they are calculated later from these files.
#
# Output (default results/experiment/):
#   experiment_results.csv  case,seed,task_id,tickets,work_units_completed,dispatches,observed_share
#   convergence.csv         case,seed,dispatch,task_id,cumulative_share
#
# Usage: scripts/run_experiment.sh [output_dir]
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BINARY="$ROOT/lottery_scheduler"
OUT_DIR="${1:-$ROOT/results/experiment}"
WORK_DIR="$(mktemp -d)"
trap 'rm -rf "$WORK_DIR"' EXIT

SEEDS=30
DISPATCHES=10000
QUANTUM=1000
STEP=100   # convergence sampling: one point every STEP dispatches

# case name -> input file
CASES=(
  "case3:$ROOT/tests/equal_shares_input_files/equal_shares_input_file.csv"
  "case4:$ROOT/tests/proportionality_input_files/proportionality_input_file.csv"
)

make -C "$ROOT" all >/dev/null
mkdir -p "$OUT_DIR"

echo "case,seed,task_id,tickets,work_units_completed,dispatches,observed_share" > "$OUT_DIR/experiment_results.csv"
echo "case,seed,dispatch,task_id,cumulative_share" > "$OUT_DIR/convergence.csv"

for entry in "${CASES[@]}"; do
  name="${entry%%:*}"
  input="${entry#*:}"
  echo "Running $name ($SEEDS seeds, $DISPATCHES dispatches, quantum $QUANTUM)"

  for seed in $(seq 1 "$SEEDS"); do
    summary="$WORK_DIR/${name}_summary_$seed.csv"
    events="$WORK_DIR/${name}_events_$seed.csv"

    "$BINARY" --input "$input" --mode quantum --quantum "$QUANTUM" \
      --seed "$seed" --max-dispatches "$DISPATCHES" \
      --log "$events" --summary "$summary" >/dev/null

    # Final result per task (summary columns: 1 id, 2 tickets, 4 completed, 5 dispatches, 9 share).
    awk -F, -v c="$name" -v s="$seed" 'FNR > 1 { printf "%s,%d,%d,%d,%d,%d,%s\n", c, s, $1, $2, $4, $5, $9 }' \
      "$summary" >> "$OUT_DIR/experiment_results.csv"

    # Cumulative share of work per task, sampled from the event log.
    awk -v c="$name" -v s="$seed" -v step="$STEP" '
      /run_units=/ {
        match($0, /dispatch=[0-9]+/);  d  = substr($0, RSTART + 9,  RLENGTH - 9)  + 0
        match($0, /winner_id=[0-9]+/); id = substr($0, RSTART + 10, RLENGTH - 10) + 0
        match($0, /run_units=[0-9]+/); u  = substr($0, RSTART + 10, RLENGTH - 10) + 0
        done[id] += u; total += u; if (id > max_id) max_id = id
        if (d % step == 0)
          for (t = 1; t <= max_id; t++)
            printf "%s,%d,%d,%d,%.6f\n", c, s, d, t, done[t] / total
      }' "$events" >> "$OUT_DIR/convergence.csv"
  done
done

echo "Data written to $OUT_DIR"
