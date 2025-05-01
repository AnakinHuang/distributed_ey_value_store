#!/usr/bin/env bash
set -euo pipefail

# ---------------------------------------
# Configuration
# ---------------------------------------
CONFIG="client_config.txt"
CLIENT="./client"
NODE="./node"
LOGDIR="logs"
BENCHDIR="benchmarks"
OPS=1000                # for the micro‐benchmark
NODES=(A B C)           # replica IDs

# ---------------------------------------
# Prepare directories & cleanup trap
# ---------------------------------------
rm -rf "$LOGDIR" "$BENCHDIR"
mkdir -p "$LOGDIR" "$BENCHDIR"

# Indexed array for PIDs
declare -a PIDS

cleanup() {
  echo "Cleaning up replicas..."
  for pid in "${PIDS[@]}"; do
    kill "$pid" 2>/dev/null || true
  done
}
trap cleanup EXIT

# Helper to kill a node by its ID
kill_node(){
  local target=$1
  for i in "${!NODES[@]}"; do
    if [ "${NODES[i]}" = "$target" ]; then
      kill "${PIDS[i]}" 2>/dev/null || true
      wait "${PIDS[i]}" 2>/dev/null || true
      return
    fi
  done
}

# ---------------------------------------
# 1) Start replicas
# ---------------------------------------
echo "--- Starting replicas A, B, C ---"
for i in "${!NODES[@]}"; do
  id=${NODES[i]}
  $NODE "$id" "$CONFIG" >"$LOGDIR/node${id}.log" 2>&1 &
  PIDS[i]=$!
done
sleep 1

# ---------------------------------------
# 2) Smoke Tests
# ---------------------------------------
pass_count=0
total_count=0

run_test(){
  local name=$1 script=$2 out=$3 pattern=$4
  printf "\n[%-20s] " "$name"
  echo -e "$script" | $CLIENT client1 "$CONFIG" >"$LOGDIR/$out" 2>&1
  total_count=$((total_count+1))
  if grep -qE "$pattern" "$LOGDIR/$out"; then
    echo PASS; pass_count=$((pass_count+1))
  else
    echo FAIL
  fi
}

run_test "cold GET"            "get foo\nexit"               "cold_get.log"      "^GET response: *\$"
run_test "basic PUT/GET"       "put foo bar\nget foo\nexit"  "basic_put_get.log" "GET response: bar"

# Kill C for failure test
kill_node C
sleep 1
run_test "one-node-down PUT/GET" \
  "put gamma three\nget gamma\nexit" "failure_put_get.log" "GET response: three"

# Report smoke‐test results
echo -e "\n=== SMOKE TEST SUMMARY ==="
echo "Passed $pass_count of $total_count smoke tests."

# ---------------------------------------
# 3) Restart C for healthy benchmark
# ---------------------------------------
echo -e "\n--- Restoring node C for healthy benchmark ---"
for i in "${!NODES[@]}"; do
  if [ "${NODES[i]}" = "C" ]; then
    $NODE C "$CONFIG" >"$LOGDIR/nodeC.log" 2>&1 &
    PIDS[i]=$!
    break
  fi
done
sleep 1

# ---------------------------------------
# 4) Micro‐Benchmarking PUT throughput
# ---------------------------------------
echo -e "\n=== MICRO‐BENCHMARK: $OPS PUTs ==="
echo "mode,ops,total_us,throughput_ops_per_sec" >"$BENCHDIR/put_throughput.csv"

bench_puts(){
  local mode=$1 down=$2
  if [ -n "$down" ]; then
    echo "--- Killing $down for failure benchmark ---"
    kill_node "$down"
    sleep 1
  fi

  echo "--- Mode: $mode; sending $OPS PUTs ---"
  cmds=""
  for i in $(seq 1 $OPS); do
    cmds+="put k$i v$i"$'\n'
  done
  cmds+="exit"$'\n'

  start=$(date +%s%6N)
  printf "%s" "$cmds" | $CLIENT client1 "$CONFIG" >/dev/null 2>&1
  end=$(date +%s%6N)

  dtus=$(( end - start ))
  throughput=$(awk -v ops=$OPS -v dt=$dtus 'BEGIN{ printf "%.2f", ops/(dt/1e6) }')
  echo "  $mode: ${dtus}µs, ${throughput} ops/s"
  echo "$mode,$OPS,$dtus,$throughput" >>"$BENCHDIR/put_throughput.csv"
}

bench_puts "healthy" ""
bench_puts "one-down" "B"

# ---------------------------------------
# 5) Final Results
# ---------------------------------------
echo -e "\n=== RESULTS ==="
echo "Logs directory:       $LOGDIR/"
echo "Benchmark CSV file:   $BENCHDIR/put_throughput.csv"

# Exit non-zero if any smoke-test failed
exit $(( total_count - pass_count ))
