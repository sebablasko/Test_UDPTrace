#!/bin/bash

# Reglas generales
MAX_PACKS=1000000
amplificador=10
total_num_threads_per_socket="1 2 4 8 16 24 36 48 64 128"
num_clients=4

# reglas Internet
num_port=2820

# Compilar
echo "Compilando..."
make all
echo "Done"


# Recuperar trazas para:
#   -Internet y UNIX
#   -Multiples configuraciones de threads
#   -Chao con las repeticiones

for num_threads_per_socket in $total_num_threads_per_socket
do

	if (($num_threads_per_socket >= 0 & $num_threads_per_socket < 10)); then salida="Trace_00"$num_threads_per_socket"threads_"; fi
	if (($num_threads_per_socket >= 10 & $num_threads_per_socket < 100)); then salida="Trace_0"$num_threads_per_socket"threads_"; fi
	if (($num_threads_per_socket >= 100)); then salida="Trace_"$num_threads_per_socket"threads_"; fi

  # Prueba Internet UDP
  ./runInternet.sh $MAX_PACKS $amplificador $num_threads_per_socket $num_port $num_clients
  echo "Rescatando Trazas..."
  cat /sys/kernel/debug/tracing/trace > $salida"Internet.txt"
  echo "Done"
  # Limpiar pid
  > /sys/kernel/debug/tracing/set_ftrace_pid


  # # Prueba UNIX UDP
  # ./runUNIX.sh $MAX_PACKS $amplificador $num_threads_per_socket $num_port $num_clients
  # echo "Rescatando Trazas..."
  # cat /sys/kernel/debug/tracing/trace > $salida"UNIX.txt"
  # echo "Done"
  # # Limpiar pid
  # > /sys/kernel/debug/tracing/set_ftrace_pid


done
# Limpiar
echo "Limpiando..."
make clean
echo "Done"
