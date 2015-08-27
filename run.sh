#!/bin/bash

# Reglas generales
MAX_PACKS=1000000
amplificador=10
num_threads=1
num_clients=4

# reglas Internet
num_port=1820

echo "Compilando..."
make all
echo "Done"

# echo nop > /sys/kernel/debug/tracing/current_tracer
# echo 0 > /sys/kernel/debug/tracing/tracing_on
# echo function_graph > /sys/kernel/debug/tracing/current_tracer

#./internetUDPServerTesis --packets $MAX_PACKS --threads $num_threads --port $num_port --verbose&
./unixUDPServerTesis --packets $MAX_PACKS --threads $num_threads --verbose&
pid=$!
sleep 1

for ((i=1 ; $i<=$num_clients ; i++))
{
  #./internetUDPClientTesis --packets $(($MAX_PACKS*$amplificador)) --ip 127.0.0.1 >> /dev/null &
  ./unixUDPClientTesis --packets $(($MAX_PACKS*$amplificador)) >> /dev/null &
}

wait $pid

make clean

# cat /sys/kernel/debug/tracing/trace > "Trace_"$num_sockets"Sockets_"$num_threads"Threads_UNIX.txt"
# > /sys/kernel/debug/tracing/set_ftrace_pid

echo "Done"
