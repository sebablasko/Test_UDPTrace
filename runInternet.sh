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
echo "Evaluación UDP: ./internetUDPServerTesis --packets $MAX_PACKS --threads $num_threads --port $num_port ..."
./internetUDPServerTesis --packets $MAX_PACKS --threads $num_threads --port $num_port&
pid=$!
sleep 1

for ((i=1 ; $i<=$num_clients ; i++))
{
  ./internetUDPClientTesis --packets $(($MAX_PACKS*$amplificador)) --ip 127.0.0.1 >> /dev/null --port $num_port &
}

wait $pid
"Done"
