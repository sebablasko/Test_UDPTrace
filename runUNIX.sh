#!/bin/bash

#Recuperar parametros
MAX_PACKS=$1
shift 1
amplificador=$1
shift 1
num_threads=$1
shift 1
num_port=$1
shift 1
num_clients=$1


# Preparar Ftrace
echo nop > /sys/kernel/debug/tracing/current_tracer
echo 0 > /sys/kernel/debug/tracing/tracing_on
echo function_graph > /sys/kernel/debug/tracing/current_tracer


# Lanzar Programa
echo "Evaluación UNIX: ./unixUDPServerTesis --packets $MAX_PACKS --threads $num_threads --verbose&"
./unixUDPServerTesis --packets $MAX_PACKS --threads $num_threads --verbose&
pid=$!
sleep 1

for ((i=1 ; $i<=$num_clients ; i++))
{
  ./unixUDPClientTesis --packets $(($MAX_PACKS*$amplificador)) >> /dev/null &
}

wait $pid
"Done"
