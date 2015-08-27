#define _GNU_SOURCE

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../ssocket/ssocket.h"
#include <getopt.h>

//Definiciones
#define BUF_SIZE 512
#define MY_SOCK_PATH "unix_sock"

//Variables
int first_pack = 0;
struct timeval dateInicio, dateFin;
char buf[BUF_SIZE];
int mostrarInfo = 0;
int intensiveMode = 0;
int MAX_PACKS = 0;
double segundos;


void print_usage(){
    printf("Uso: ./unixUDPClient [verbose] [intensive] --packets <num> \n");
}

void print_config(){
    printf("Detalles de la prueba:\n");
    printf("\tPaquetes a enviar:\t%d\n", MAX_PACKS);
    printf("\tNombre a tomar:\t%s\n", MY_SOCK_PATH);
    if(intensiveMode) printf("\tModo Intensivo\n") ;
}

void parseArgs(int argc, char **argv){
	int c;
	int digit_optind = 0;

	while (1){
		int this_option_optind = optind ? optind : 1;
        int option_index = 0;

		static struct option long_options[] = {
			{"packets", required_argument, 0, 'd'},
			{"verbose", no_argument, 0, 'v'},
			{"intensive", no_argument, 0, 'f'},
			{0, 0, 0, 0}
		};

         c = getopt_long (argc, argv, "vfd:",
         long_options, &option_index);

         if (c == -1)
         	break;

         switch (c){

			case 'v':
				printf ("Modo Verboso\n");
				mostrarInfo = 1;
				break;

			case 'f':
				printf ("Modo Intensivo\n");
				intensiveMode = 1;
				break;

			case 'd':
				MAX_PACKS = atoi(optarg);
				break;

			default:
				printf("Error: La función getopt_long ha retornado un carácter desconocido. El carácter es = %c\n", c);
				print_usage();
				exit(1);
         }
	}
}

int main(int argc, char **argv) {

	// Paso 1.- Parsear Argumentos
	parseArgs(argc, argv);

	// Paso 2.- Validar Argumentos
	if(MAX_PACKS < 1) {
		printf("Error en el ingreso de parametros\n");
		print_usage();
		exit(1);
	}

	if(mostrarInfo)	print_config();

	// Paso 3.- Llenar Buffer con datos para mandar
	int i;
	for(i = 0; i < BUF_SIZE; i++)
		buf[i] = 'p'+i;

	// Paso 4.- Crear Socket
	int socket_fd;
	socket_fd = unix_udp_connect(MY_SOCK_PATH);
	if(socket_fd < 0) {
		fprintf(stderr, "Error al conectarse al socket Unix\n");
		exit(1);
	}
	//Medir Inicio
	gettimeofday(&dateInicio, NULL);

	// Paso 4.1.- Escribir en el socket
	for(i = 0; i < MAX_PACKS; i++){
		if(intensiveMode){
			write(socket_fd, buf, BUF_SIZE);
		}else{
			if(write(socket_fd, buf, BUF_SIZE) != BUF_SIZE) {
  			gettimeofday(&dateFin, NULL);
  			segundos = (dateFin.tv_sec+dateFin.tv_usec/1000000.)-(dateInicio.tv_sec*1.0+dateInicio.tv_usec/1000000.);
  			//fprintf(stderr, "Falla el write al servidor, envio %d paquetes\n", i);
  			//fprintf(stderr, "total time = %g\n", segundos);
  			break;
  		}
		}
	}

	//Medir Fin
	gettimeofday(&dateFin, NULL);

	segundos=(dateFin.tv_sec*1.0+dateFin.tv_usec/1000000.)-(dateInicio.tv_sec*1.0+dateInicio.tv_usec/1000000.);
	if(mostrarInfo){
		printf("Tiempo Total = %g\n", segundos);
		printf("QPS = %g\n", MAX_PACKS*1.0/segundos);
	}else{
		printf("%g \n", segundos);
	}
	exit(0);
	return 0;
}
