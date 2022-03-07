#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <math.h>
#include <sys/timeb.h>

#define MIPUERTO 5011//atiende solicitud = cliente y servidor	

#define BACKLOG 10	 //cant max simultaneos atendidos

/*MANEJO DE HEBRAS*/
#define MAX_HEBRAS 100

/*VARIABLES DE MANEJO DE CLIENTES*/
#define N 100
#define L 50
#define C 20

/*FUNCIONES*/
void *conexion_cliente(void *parametro);
void sigchld_handler(int s);


/*VARIABLES DE HEBRAS*/
pthread_t tid[MAX_HEBRAS]; /*HEBRA QUE TRATA A CADA CLIENTE*/
pthread_attr_t atributos;
int new_fd[MAX_HEBRAS];	/*ARR DE SOCKETS*/
pthread_mutex_t protege_solicitud[MAX_HEBRAS]; /*ARR DE SEMAFOROS*/

/*VARIABLES DE MANEJO DE CLIENTES*/
typedef struct 
	{
	char archivo[C];
	}ARCHIVO;

typedef struct
	{
	int cli_id;
	char direccion[C];
	int cant_archivos;
	int disponible;
	ARCHIVO lista_archivos[L];
	}CLIENTE;

CLIENTE clientes[N];

ARCHIVO arch_solicitado[N];

int num_clientes=0;

int accion=0;

struct timeb inicio,fin;

double tiempo;

FILE *cabezal;

/**************************************************************************/
/*AQUI SE MANEJA LA CONEXION*/
void *conexion_cliente(void *parametro)
	{	
	int j=(int) parametro;
	int numbytes;
	char buffer[N];	
	char senal[C];
	char temporal[N];
	char temporal2[N];
	char *temporal3;
	char *temporal4;
	char *temporal5;
	char *resto;
	char temporal6[N];
	char temporal7[N];
	char temporal8[N];
	char nombre_arch[N];
	int k;
	int l;
	int z;
	int flag;
	int algo;
	int cli;
	int arch;
	int flag_conexion;

	FILE *lector;
	
	flag_conexion=1;

	strcpy(senal,"TERMINO\n");
	
	// el proceso hijo no necesita escuchar por nuevas conexiones, el padre escucha todos los clientes
	
	/* 1).- RECIBIR Y PROCESAR LISTA DE ARCHIVOS */
	
	k=0;
	flag=0;
	clientes[j].cant_archivos=0;
	
	ftime(&fin);
	tiempo=(double)(fin.time-inicio.time);
	
	printf("%d\t%.0f s\t\t%s\t\t(2) INCIO RX LISTA ARCHIVO DEL CLIENTE.\n",accion,tiempo,clientes[j].direccion);					
	accion=accion+1;
		

	do
		{
		/* RECIBIR */		
		if (numbytes=read(new_fd[j],buffer,N)==-1)
			{
			perror("Error al recibir linea");
			}
		
		/* PROCESAR CADENA */
		if (strcmp(buffer,"TERMINO")==0)
			{
			flag=1;
			}
		else
			{
			buffer[strlen(buffer)-2]='\0';
			strcpy(clientes[j].lista_archivos[k].archivo,buffer);
			clientes[j].cant_archivos=clientes[j].cant_archivos + 1;
			k=k+1;
			}
		}
	while (flag!=1);

	/*ENVIO DEL IDENTIFICADOR*/
	sprintf(temporal,"%i",j);
	write(new_fd[j],temporal,N);

	ftime(&fin);
	tiempo=(double)(fin.time-inicio.time);

	printf("%d\t%.0f s\t\t%s\t\t(2) FIN RX LISTA ARCHIVO DEL CLIENTE.\n",accion,tiempo,clientes[j].direccion);					
	accion=accion+1;
	
	strcpy(arch_solicitado[j].archivo,"NOHAYNADA");

	/* 2).- ESPERAR ALGUNA ACCION DEL CLIENTE */
	while(flag_conexion==1)
		{
		/*PRIMERO WRITE, LUEGO READ*/

		/*ESCRIBIR SOLICITUD DE ENVIO DE ARCHIVO*/

		if (strcmp(arch_solicitado[j].archivo,"NOHAYNADA")!=0)
			{
			/****SE SOLICITO EL ARCHIVO AL CLIENTE ... ENVIAR UNA SEÑAL****/

			ftime(&fin);
			tiempo=(double)(fin.time-inicio.time);
			
			printf("%d\t%.0f s\t\t%s\t\t(3) INCIO RX ARCHIVO CLI:%s ARCH:%s\n",accion,tiempo,clientes[j].direccion,clientes[j].direccion,arch_solicitado[j].archivo);
			accion=accion+1;

			sprintf(temporal,"SSA:%s:",arch_solicitado[j].archivo);
			
			write(new_fd[j],temporal,N);

			/*COMENZAR PROCESO DE RECEPCION*/
			read(new_fd[j],temporal,N);
			if(strcmp(temporal,"NOEXISTEARCHIVO")==0)
				{
				ftime(&fin);
				tiempo=(double)(fin.time-inicio.time);				

				printf("%d\t%.0f s\t\t%s\t\t(3) ERR RX: NO EXISTE ARCHIVO CLI:%s ARCH:%s\n",accion,tiempo,clientes[j].direccion,clientes[j].direccion,arch_solicitado[j].archivo);
				accion=accion+1;
				}
			else
				{
				
				/*CREAR EL ARCHIVO*/
				cabezal=fopen(arch_solicitado[j].archivo,"w");
			
				flag=0;
				do
					{
					read(new_fd[j],temporal,N);
				
					if (strcmp(temporal,"TERMINO")==0)
						{
						flag=1;
						}
					else
						{
						fprintf(cabezal,"%s",temporal);
						}
					}
				while(flag==0);

				/*TENEMOS EL ARCHIVO ARRIBA*/
				ftime(&fin);
				tiempo=(double)(fin.time-inicio.time);

				printf("%d\t%.0f s\t\t%s\t\t(3) FIN RX ARCHIVO CLI:%s ARCH:%s.\n",accion,tiempo,clientes[j].direccion,clientes[j].direccion,arch_solicitado[j].archivo);
				accion=accion+1;
			
				fclose(cabezal);					
				}
				
			/*LIMPIAR*/
			strcpy(arch_solicitado[j].archivo,"NOHAYNADA");
			}
		else
			{
			/****NO HAY SOLICITUDES DE RECEPCION ACA, QUE EL CLIENTE SUBA UN ARCHIVO****/

			/*NO HA LLEGADO NADA, WRITE PARA SACAR DEL CICLO AL CLIENTE*/
			strcpy(temporal,"NDA:");
			write(new_fd[j],temporal,N);
				
			/*ESCUCHANDO AL CLIENTE*/
			/*AQUI DEBIERA IR SOLICITUDES DE BAJADA DE DATOS A SERVIDOR*/
			read(new_fd[j],temporal,N);

			strcpy(temporal2,temporal);

			temporal3=strtok(temporal2,":");

			if(strcmp(temporal3,"SLC")==0)
				{
				ftime(&fin);
				tiempo=(double)(fin.time-inicio.time);

				printf("%d\t%.0f s\t\t%s\t\t(4) INICIO TX LISTA CLIENTES\n",accion,tiempo,clientes[j].direccion);
				accion=accion+1;

				/*SE ENVIA LA LISTA*/
				for (k=0;k<num_clientes;k++)
					{
					sprintf(temporal6,"%s:%i:",clientes[k].direccion,clientes[k].disponible);
					write(new_fd[j],temporal6,N);
					}
				/*INDICA FIN DE LISTA*/
				write(new_fd[j],"TERMINO",N);
		
				ftime(&fin);
				tiempo=(double)(fin.time-inicio.time);

				printf("%d\t%.0f s\t\t%s\t\t(4) FIN TX LISTA CLIENTES\n",accion,tiempo,clientes[j].direccion);
				accion=accion+1;
				}
			if(strcmp(temporal3,"SLA")==0)
				{
				temporal4=strtok(NULL,":");
				cli=atoi(temporal4);
				cli=cli-1;

				ftime(&fin);
				tiempo=(double)(fin.time-inicio.time);
				
				printf("%d\t%.0f s\t\t%s\t\t(5) INICIO TX LST. ARCHIVO CLI: %s\n",accion,tiempo,clientes[j].direccion,clientes[cli].direccion);
				accion=accion+1;
								
				for (k=0;k<clientes[cli].cant_archivos;k++)
					{
					send(new_fd[j],clientes[cli].lista_archivos[k].archivo,N,0);
					}
				/*INDICA FIN DE LA LISTA*/
				write(new_fd[j],"TERMINO",N);
		
				ftime(&fin);
				tiempo=(double)(fin.time-inicio.time);
				
				printf("%d\t%.0f s\t\t%s\t\t(5) FIN TX LST. ARCHIVO CLI: %s\n",accion,tiempo,clientes[j].direccion,clientes[cli].direccion);
				accion=accion+1;				
				}
			if(strcmp(temporal3,"SBA")==0)
				{			
				temporal4=strtok(NULL,":");
				temporal5=strtok(NULL,":");

				cli=atoi(temporal4);
				arch=atoi(temporal5);
				cli=cli-1;
				arch=arch-1;

				ftime(&fin);
				tiempo=(double)(fin.time-inicio.time);
				
				printf("%d\t%.0f s\t\t%s\t\t(6) INICIO TX ARCHIVO CLI:%s ARCH:%s\n",accion,tiempo,clientes[j].direccion,clientes[cli].direccion,clientes[cli].lista_archivos[arch].archivo);
				accion=accion+1;

				/*PERO ANTES ES NECESARIO SABER SI SE ESTA ATENDIENDO OTRA SOLICITUD*/
				
				/**********************************/
				/*** SECCION DE EXCLUSION MUTUA ***/
				pthread_mutex_lock(&protege_solicitud[cli]);

				/*ES NECESARIO ENLAZAR AL OTRO CLIENTE*/
				strcpy(arch_solicitado[cli].archivo,clientes[cli].lista_archivos[arch].archivo);
		
				/*HAY QUE ESPERAR POR EL ENLACE*/
				while(strcmp(arch_solicitado[cli].archivo,"NOHAYNADA")!=0);

				/*ABRIR ARCHIVO*/
				lector=fopen(clientes[cli].lista_archivos[arch].archivo,"r");

				if (lector==NULL)
					{
					ftime(&fin);
					tiempo=(double)(fin.time-inicio.time);
				
					write(new_fd[j],"NOEXISTEARCHIVO",N);
					
					printf("%d\t%.0f s\t\t%s\t\t(6) ERR TX: NO EXISTE ARCHIVO CLI:%s ARCH:%s\n",accion,tiempo,clientes[j].direccion,clientes[cli].direccion,clientes[cli].lista_archivos[arch].archivo);
					accion=accion+1;
					}
				else
					{
					/*ENVIAR NOMBRE DEL ARCHIVO*/
					write(new_fd[j],clientes[cli].lista_archivos[arch].archivo,N);
					
					/*ENVIAR LINEA A LINEA*/
					while(!feof(lector))
						{
						if(fgets(temporal,N,lector)!=NULL)
							{
							sprintf(temporal2,"%s",temporal);
							write(new_fd[j],temporal2,N);
							}
						}
					fclose(lector);
					strcpy(temporal,"TERMINO");
					write(new_fd[j],temporal,N);
				
					strcpy(temporal7, "./");		
					strcat(temporal7,clientes[cli].lista_archivos[arch].archivo);
					sprintf(temporal8,"rm -f %s",temporal7);
			
					system(temporal8);
	
					ftime(&fin);
					tiempo=(double)(fin.time-inicio.time);
				
					printf("%d\t%.0f s\t\t%s\t\t(6) FIN TX ARCHIVO CLI:%s ARCH:%s\n",accion,tiempo,clientes[j].direccion,clientes[cli].direccion,clientes[cli].lista_archivos[arch].archivo);
					accion=accion+1;
					}
				
				pthread_mutex_unlock(&protege_solicitud[cli]);
				/*** FIN SECCION DE EXCLUSION MUTUA ***/
				/**************************************/
				}
			if (strcmp(temporal3,"DES")==0)
				{				
				/*SOLICITUD DE DESCONEXION*/
				
				clientes[j].disponible=0;
				clientes[j].disponible=0;
				clientes[j].disponible=0;
				clientes[j].disponible=0;
				
				flag_conexion=0;

				printf("%d\t%.0f s\t\t%s\t\t(1) FIN CONEXION DEL CLIENTE.\n",accion,tiempo,clientes[j].direccion);
				}
			}
		}

	while(1);

	close(new_fd[j]);//lo cierra
	exit(0);
	}


/**************************************************************************/
void sigchld_handler(int s)

	{
	//ESPERA CUALQUIERA DE LOS HIJOS,NINGUNO EN ESPECIFICO
	//WNOHANG,SILOS HIJOS NO ESTAN LISTOS PARA RESPONDER AL PADRE EL PADRE SE LOS SALTA 
	while(waitpid(-1, NULL, WNOHANG) > 0);
	}
/**************************************************************************/
int main(void)
	{
	int e;
	float flt;
	
	/*OTRAS VARIABLES*/
	/*SOCKFD EL SERVIDOR CREA EL PUERTO PARA SOLO ESCUCHAR AL CLIENTE,ATIENDEN CONEXIONES*/
	int sockfd; 
	

	struct sockaddr_in my_addr;//MY_ADDR DIRECCION Q TIENE EL SERVIDOR
	struct sockaddr_in their_addr;//THEIR_ADDR INFORMACION CLIENTE QUE LLEGA
	socklen_t sin_size; //LARGO ESTRUCTURA ANTERIOR SOCKADDR_IN
	struct sigaction sa;
	int yes=1;

	ftime(&inicio);
		
	/*INICIALIZACION SEMAFOROS*/
	for(e=0;e<MAX_HEBRAS;e++)
		{
		pthread_mutex_init(&protege_solicitud[e],NULL);
		}


	system("clear");

	printf("DETALLE DE OPERACION DEL SERVIDOR:\n");
	printf("==================================\n\n");
	printf("Nº\tTEMP\t\tDIR.IP\t\t\tACCION\n");
	printf("------------------------------------------------------------------------------------\n");
	
	ftime(&fin);

	tiempo=(double)(fin.time-inicio.time);

	printf("%d\t%.0f s\t\tN/A\t\t\t(0) INICIO SERVIDOR\n",accion,tiempo);
	accion=accion+1;	

	
	/*SE CREA SOCKET(FAMILIA,TIPO DEL SOCKET SOCK_STREAM CONEXION VIRTUAL,PROTOCOLO(TCP/UDP OPCION))*/
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		{
		perror("Error en socket");
		exit(1);
		}
	
	/*SETSOCKPT(I SOCKET,NIVEL Q SURGE EFECTO(SOCKET,PROTOCOLO),SETEA VALR ADDR,OPCION VERDADERO)*/
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
		{
		perror("Error en setsockopt");
		exit(1);
		}
	
	my_addr.sin_family = AF_INET;		 //FAMILIA DEL SOCKET INTERNET  
	my_addr.sin_port = htons(MIPUERTO);	 //DEFINE PUERTO POR DONDE ESCUCHA,VALOR MAQUINA->VALOR DE RED
	my_addr.sin_addr.s_addr = INADDR_ANY; //IP DE LA MAQUINA LOCAL POR DEFECTO QUE TIENE 
	memset(&(my_addr.sin_zero), '\0', 8); //SETEA EN CERO EL RESTO DE LA ESRUCTURA, EN ESTA TAREA SIEPRE CERO
	
	/*SE ENLAZA EL SOCKET CON EL PUERTO*/
	if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
		{
		perror("Error en bind");
		exit(1);
		}
	
	/*CUANTOS CLIENTES SIMPULTANEOS ESCUCHARA*/
	if (listen(sockfd, BACKLOG) == -1)
		{
		perror("Error en listen");
		exit(1);
		}
	
	/*DEJAR DE LA MISMA FORMA PARA LA TAREA*/
	sa.sa_handler = sigchld_handler;//FORMA DE ATENDER A LS HIJOS DEL PROCESO PADRE 
	sigemptyset(&sa.sa_mask);//SETEA EN CERO,PARA QUE ATIENDA A TODAS 
	sa.sa_flags = SA_RESTART;//SETEA LAS QUE PUEDEN CONTENER ERROR
	if (sigaction(SIGCHLD, &sa, NULL) == -1)
		{
		perror("Error en sigaction");
		exit(1);
		}

	while(num_clientes<=99)
		{
		/*LLENO UNA ESTRUCTURA EN ESTE CASO THEIR_ADDR Y SE LA PASA AL SERVIDOR Y RETORNA EL NEW_FD*/
		sin_size = sizeof(struct sockaddr_in);

		
		/*ESCUCHANDO EL PUERTO EN ESPERA DE CONEXION*/		
		if ((new_fd[num_clientes] = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1)
			{
			perror("Error en accept");
			continue;
			}
		
		/*IMPRIME DE DONDE FUE RECIBIDA LA CONEXION*/
		/*CONVIERTE EL IP DEL CLIENTE CON INET_NTOA*/

		clientes[num_clientes].cli_id=num_clientes;
		strcpy(clientes[num_clientes].direccion,inet_ntoa(their_addr.sin_addr));
		clientes[num_clientes].direccion[15]='\0';
		clientes[num_clientes].disponible=1;
		
		ftime(&fin);
		tiempo=(double)(fin.time-inicio.time);
	
		printf("%d\t%.0f s\t\t%s\t\t(1) SE OBTIENE CONEXION DEL CLIENTE\n",accion,tiempo,clientes[num_clientes].direccion);					
		accion=accion+1;
			
		/*DESPACHANDO HEBRA PARA ATENCION DE CLIENTE*/
		pthread_create(&tid[num_clientes], &atributos, conexion_cliente, (void *) num_clientes);	
		
		num_clientes=num_clientes+1;
		}
	
	ftime(&fin);
	tiempo=(double)(fin.time-inicio.time);

	printf("%d\t%.0f s\t\tN/A\t\t\t(0) MAX. CLIENTES ALCANZADO. FIN SERVIDOR.\n",accion,tiempo);
	printf("------------------------------------------------------------------------------------\n");

	return 0;
	}

