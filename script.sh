#!/usr/bin/env bash

SCRIPT="./L2_latency_cache_size_sweep"
OUTPUT="./csv/resultados.csv"

# Define start, end, and step
START=64
END=$((8192*1024))
STEP=1024

# Create CSV header
echo "valor,saida" > "$OUTPUT"

# Loop over generated values
for valor in $(seq $START $STEP $END); do
    saida=$($SCRIPT "$valor")
    saida_formatada=$(echo "$saida" | sed 's/"/""/g')
    echo "$valor,\"$saida_formatada\"" >> "$OUTPUT"
    sleep 10
done

