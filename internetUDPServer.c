#define _GNU_SOURCE

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include "../ssocket/ssocket.h"

#include <sched.h>
#include <unistd.h>

#include <getopt.h>
#include <string.h>

#include <fcntl.h>

//Definiciones
#define BUF_SIZE 512
#define DEFAULT_PORT 1820
// Nombres Schedulling
#define equitativeSched "equitativeSched"
#define dummySched "dummySched"
#define pairSched "pairSched"
#define impairSched "impairSched"
#define numaPairSched "numaPairSched"

//Variables
int first_pack = 0;
struct timeval dateInicio, dateFin;
pthread_mutex_t lock;
int mostrarInfo = 0;
int distribuiteCPUs = 0;
int MAX_PACKS = 0;
int NTHREADS = 0;
int DESTINATION_PORT = DEFAULT_PORT;
int reuseport = 0;
double segundos;
char* schedu = "SO";
int cpuAssign = 0;
// variable de control
int enabledTrace = 0;

//Variables para FTRACE
int trace_fd = -1;
char *trace_path = "/sys/kernel/debug/tracing/tracing_on";
int pid_trace_fd = -1;
char *pid_trace_path = "/sys/kernel/debug/tracing/set_ftrace_pid";
//originalmente era no pid
int marker_fd = -1;
char *marker_path = "/sys/kernel/debug/tracing/trace_marker";


void print_usage(){
    printf("Uso: ./internetUDPServer [--verbose] --packets <num> --threads <num> --port <num> [--reuseport] [--scheduler <SchedSchemmaName> --setcpu <num>]\n");
}

void print_config(){
    printf("Detalles de la prueba:\n");
    printf("\tTracert de llamadas:\t");
    enabledTrace ? printf("Activado\n") : printf("Apagado\n");
    printf("\tPuerto a escuchar:\t%d\n", DESTINATION_PORT);
    printf("\tPaquetes a recibir:\t%d de %d bytes\n", MAX_PACKS, BUF_SIZE);
    printf("\tUso de ReusePort:\t");
    reuseport ? printf("Activado\n") : printf("Apagado\n");
    printf("\tThreads que compartirán el socket:\t%d\n", NTHREADS);
    printf("\tScheduller usado:\t%s\n",schedu);
    printf("\tAsignación a CPU:\t");
	strcmp(schedu,dummySched)==0 ? printf("CPU %d\n", cpuAssign) : printf("Segun SO\n");
}

void parseArgs(int argc, char **argv){
	int c;
	int digit_optind = 0;
	while (1){
		int this_option_optind = optind ? optind : 1;
        int option_index = 0;

		static struct option long_options[] = {
			{"packets", required_argument, 0, 'd'},
			{"threads", required_argument, 0, 't'},
			{"port", required_argument, 0, 'p'},
			{"scheduler", required_argument, 0, 's'},
			{"setcpu", required_argument, 0, 'c'},
			{"reuseport", no_argument, 0, 'r'},
			{"verbose", no_argument, 0, 'v'},
			{0, 0, 0, 0}
		};

         c = getopt_long (argc, argv, "vrd:t:p:s:c:",
         long_options, &option_index);

         if (c == -1)
         	break;

         switch (c){
			case 'v':
				printf ("Modo Verboso\n");
				mostrarInfo = 1;
				break;

			case 'r':
				reuseport = 1;
				break;

			case 'd':
				MAX_PACKS = atoi(optarg);
				break;

			case 't':
				NTHREADS = atoi(optarg);
				break;

			case 'p':
				DESTINATION_PORT = atoi(optarg);
				break;

			case 's':
				schedu = optarg;
				break;

			case 'c':
				cpuAssign = atoi(optarg);
				break;

			default:
				printf("Error: La función getopt_long ha retornado un carácter desconocido. El carácter es = %c\n", c);
				print_usage();
				exit(1);
         }
	}
}

void llamadaHilo(int socket_fd){
	char buf[BUF_SIZE];
	int lectura;

	if(mostrarInfo) printf("Socket Operativo: %d, \t CPU: %d\n", socket_fd, sched_getcpu());

	int i;
	int paquetesParaAtender = MAX_PACKS/NTHREADS;

  //Marca FTRACE
  if(enabledTrace) write(marker_fd, "MITRACE UDP: Nuevo Thread\n", 26);

	for(i = 0; i < paquetesParaAtender; i++) {
    if(enabledTrace) write(marker_fd, "MITRACE UDP: Comienza el read del socket\n", 41);
		//lectura = recv(socket_fd, buf, BUF_SIZE, 0);
		lectura = read(socket_fd, buf, BUF_SIZE);
		if(lectura <= 0) {
			fprintf(stderr, "Error en el read del socket (%d)\n", lectura);
			exit(1);
		}
		if(first_pack==0) {
			pthread_mutex_lock(&lock);
			if(first_pack == 0) {
				if(mostrarInfo)	printf("got first pack\n");
				first_pack = 1;
				//Medir Inicio
				gettimeofday(&dateInicio, NULL);
			}
			pthread_mutex_unlock(&lock);
		}
	}

	if(mostrarInfo) printf("Fin Socket Operativo: %d, \t CPU: %d\n", socket_fd, sched_getcpu());
}

int main(int argc, char **argv){

	// Paso 1.- Parsear Argumentos
	parseArgs(argc, argv);

	// Paso 2.- Validar Argumentos
	if(MAX_PACKS < 1 || NTHREADS < 1){
		printf("Error en el ingreso de parametros\n");
		print_usage();
		exit(1);
	}

  int pid = getpid();

	if(mostrarInfo)	print_config();
	if(mostrarInfo)	printf("El pid es %d\n", pid);

	// Paso 2.1.- Recuperar Total CPUs
	int totalCPUs = sysconf(_SC_NPROCESSORS_ONLN);
	if(mostrarInfo) printf("Total de Procesadores disponibles: %d\n", totalCPUs);

	// Paso 3.- Crear el Socket
	int socket_fd;
	char ports[10];
	sprintf(ports, "%d", DESTINATION_PORT);
	socket_fd = reuseport ? udp_bind_reuseport(ports) : udp_bind(ports);
	if(socket_fd < 0) {
		fprintf(stderr, "Error de bind al tomar el puerto\n");
		exit(1);
	}
	pthread_mutex_init(&lock, NULL);

	// Paso 4.- Configurar Threads
	pthread_t pids[NTHREADS];
  pthread_attr_t attr;
  cpu_set_t cpus;
  pthread_attr_init(&attr);


  // Paso 5.- Inicar FTRACE
  /* Primero, Abrir Archivos para cosas de FTRACE */
  if(enabledTrace){
    printf("Abrir archivos para FTRACE\n");
    printf("\t%s", trace_path);
    trace_fd = open(trace_path, O_WRONLY);
    if(trace_fd < 1){
      printf("\tError\n");
      exit(1);
    }else{
      printf("\tOK\n");
    }
    printf("\t%s", pid_trace_path);
    pid_trace_fd = open(pid_trace_path, O_WRONLY);
    if(pid_trace_fd < 1){
      printf("\tError\n");
      exit(1);
    }else{
      printf("\tOK\n");
    }
    printf("\t%s", marker_path);
    marker_fd = open(marker_path, O_WRONLY);
    if(marker_fd < 1){
      printf("\tError\n");
      exit(1);
    }else{
      printf("\tOK\n");
    }
    /* Y activar Ftrace */
    write(trace_fd, "1", 1);
    char tmp[12]={0x0};
    sprintf(tmp,"%d", pid);
    write(pid_trace_fd, tmp, sizeof(tmp));
  }

	// Paso 6.- Lanzar Threads segun el mecanismo de afinidad definido
	int i;
	for(i=0; i < NTHREADS; i++) {
		if(strcmp(schedu,equitativeSched)==0){
				/* Caso 1: Afinidad Equitativa de threads entre las cpu */
				CPU_ZERO(&cpus);
				CPU_SET(i%totalCPUs, &cpus);
				pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);
				pthread_create(&pids[i], &attr, llamadaHilo, socket_fd);
		}else if(strcmp(schedu,dummySched)==0){
				/* Caso 2: Afinidad Dummy, asigna todos los threads al cpu definido en cpuAssign */
				CPU_ZERO(&cpus);
				CPU_SET(cpuAssign, &cpus);
				pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);
				pthread_create(&pids[i], &attr, llamadaHilo, socket_fd);
		}else if(strcmp(schedu,pairSched)==0){
				/* Caso 3: Pair Affinity, asigna todos los threads a cpus con numeros pares */
				CPU_ZERO(&cpus);
				CPU_SET((2*i)%totalCPUs, &cpus);
				pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);
				pthread_create(&pids[i], &attr, llamadaHilo, socket_fd);
		}else if(strcmp(schedu,impairSched)==0){
				/* Caso 4: Impair Affinity, asigna todos los threads a cpus con numeros impares */
				CPU_ZERO(&cpus);
				CPU_SET((2*i+1)%totalCPUs, &cpus);
				pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);
				pthread_create(&pids[i], &attr, llamadaHilo, socket_fd);
		}else if(strcmp(schedu,numaPairSched)==0){
				/* Caso 5: Numa Pair Affinity, asigna todos los threads a cpus con numeros pares considerando numeracion NUMA */
				int j;
				j = (i%2)==0 ? i : (i-1) + totalCPUs/2;
				CPU_ZERO(&cpus);
				CPU_SET((i%2)*totalCPUs/2, &cpus);
				pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);
				pthread_create(&pids[i], &attr, llamadaHilo, socket_fd);
		}else{
				/* Caso 6: NoAffinity, el sistema asigna los movimientos de threads en CPU*/
				pthread_create(&pids[i], NULL, llamadaHilo, socket_fd);
		}
	}

	// Paso 7.- Esperar Threads
	for(i=0; i < NTHREADS; i++)
		pthread_join(pids[i], NULL);

  // Paso 8.- Apagar Ftrace
  write(marker_fd, "MITRACE UDP: Todos los threads terminados\n", 42);
  write(trace_fd, "0", 1);
  /* Y cierre de archivos */
  close(trace_fd);
  close(pid_trace_fd);
  close(marker_fd);

	// Paso 9.- Medir Fin
	gettimeofday(&dateFin, NULL);

	//Cerrar Socket
	close(socket_fd);

	segundos=(dateFin.tv_sec*1.0+dateFin.tv_usec/1000000.)-(dateInicio.tv_sec*1.0+dateInicio.tv_usec/1000000.);
	if(mostrarInfo){
		printf("Tiempo Total = %g\n", segundos);
		printf("QPS = %g\n", MAX_PACKS*1.0/segundos);
	}else{
		printf("%g, ", segundos);
	}
	exit(0);
}
