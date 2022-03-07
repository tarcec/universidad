#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <math.h>
#include <sys/timeb.h>

#define PORT 5011 //mismo puerto para escuchar 

#define MAXDATASIZE 100 //tamoño del buffer de mensaje 

/*DECLARACION DE FUNCIONES*/
void *esperar_datos(void *parametro);
void subir(char arch_enviar[]);
int listar_clientes();
int listar_archivos(int i);
void listar();
void bajar();
void menu();
void desconectar();
int validar_clientes(int numero);

/*DEINFICION DE ESTRUCTURA DE LISTADO DE ARCHIVOS*/
#define N 100
#define L 50
#define C 20

typedef struct
	{
	char archivo[C];
	}ARCHIVO;

ARCHIVO lista_archivos[L];

int lista_clientes[N]; 

/*DEFINICION DEL SOCKET*/
int sockfd;

/*DEFINICION DE BUFFER RECEPTOR Y ENVIADOR*/
char recepcion[N];
char envio[N];

/*DEFINICION DE FLAG DE RECEPCION*/
int flg_llego;
int flg_que_llego;

/*DEFINICION DE LAS HEBRAS*/
pthread_t tid1; /*RECIBE LAS COMUNICACIONES*/
pthread_t tid2; /*TRANSFIERE ARCHIVOS*/

pthread_mutex_t protege_socket; /*SEMAFORO DEL SOCKET*/
pthread_mutex_t protege_subida; /*SEMAFORO DE LA SUBIDA DE ARCHIVOS, SOLO UNA A LA VEZ*/

char temporal[N];

/*DEFINICION DE CABEZAL*/
FILE *lector;

char dir_servidor[C];

struct timeb inicio,fin,intermedio;

double tiempo;

int IDENTIFICADOR;

/*******************************************************************************************************/
int main(int argc, char *argv[])
	{
	/*** PRUEBAS ***/

	char *archivo="lista.in";
	int numero_archivos;
	char temporal[N];
	int i;

	int flag;

	char opc[2];

	int numbytes;
	char buf[MAXDATASIZE]; //buffer de tamaño de datos de clarado
	struct hostent *he; //donde se guardan los parametros que entrega el servidor
	struct sockaddr_in their_addr; //estructura donde se guardan  los parametros del servidor

	flg_llego=0;

	//debe traer dos argumentos
	if (argc != 2)
		{
		fprintf(stderr,"uso: cliente hostname\n");
		exit(1);
		}
	//retorno del tipo hostent y se lo asigna a he, se almacen ael he la ip del servidor, argv[1] nombre de la maquina
	if ((he=gethostbyname(argv[1])) == NULL)
		{
		perror("Error en gethostbyname");
		exit(1);
		}

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		{
		perror("Error en socket");
		exit(1);
		}

	/*** PRUEBAS ***/
	lector=fopen(archivo,"r+");
	if (lector==NULL)
		{
		printf("*** NO SE ENCUENTRA ARCHIVO LISTA.IN. PROGRAMA TERMINADO ***\n");
		return(0);
		}	

	fgets(temporal,80,lector);
	numero_archivos=atoi(temporal);

	for(i=0;i<numero_archivos;i++)
		{
		if(fgets(temporal,100,lector)!=NULL)
			{
			strcpy(lista_archivos[i].archivo,temporal);
			}
		}

	fclose(lector);

	//llenado de la estructura del servidor
	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons(PORT);
	their_addr.sin_addr = *((struct in_addr *)he->h_addr); //direccion ip del servidor
	memset(&(their_addr.sin_zero), '\0', 8);

	//creacion de una conexion virtual
	if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1)
		{
		perror("Error al conectar");
		exit(1);
		}

	strcpy(dir_servidor,inet_ntoa(their_addr.sin_addr));

	/**************** 1 ENVIO DE LISTA DE ARCHIVOS ************************/
	for (i=0;i<numero_archivos;i++)
		{
		//if (send(sockfd,lista_archivos[i].archivo,strlen(lista_archivos[i].archivo),0)==-1)
		if (write(sockfd,lista_archivos[i].archivo,N)==-1)
			{
			perror("ERROR ENVIANDO LISTA DE ARCHIVOS");
			}
		printf("i= %d ; LINEA = %s",i,lista_archivos[i].archivo);
		}

	write(sockfd,"TERMINO",N);
	
	/*LECTURA IDENTIFICADOR*/
	read(sockfd,temporal,N);
	IDENTIFICADOR=atoi(temporal);

	/*** INICIO DEL TIMER ***/
	ftime(&inicio);

	/*** 2 DETERMINAR QUE ACCION SE DEBE SEGUIR ***/

	/*INICIALIZACION ENVIO Y RECEPCION*/
	
	/*MENU PRINCIPAL*/
	menu();

	/*
	PUEDEN SUCEDER DOS COSAS
	- EL CLIENTE QUIERE BAJAR UN ARCHIVO O VER UN LISTADO DE ARCHIVOS DE OTRO CLIENTE
	- EL CLIENTE TIENE QUE TRANSFERIR ALGO AL SERVIDOR (IMPLICA UNA HEBRA ESPECIAL) */

	/* INICIAR HEBRA QUE PERMITE ESTAR ATENTA A TRANSFERENCIAS*/
	pthread_create(&tid1,NULL,esperar_datos,NULL);
	pthread_mutex_init(&protege_socket,NULL);

	/*ELEGIR UNA OPCION, CUALQUIERA DEBE BLOQUEAR A LA HEBRA CUANDO SE EJECUTE*/
	flag=0;
	do
		{		
		menu();
		scanf("%c",opc);

		switch(opc[0])
			{
			case '1':
				{
				listar();
				break;
				}
			case '2':
				{
				bajar();
				break;
				}
			case '3':
				{
				flag=0;
				break;
				}
			case '4':
				{
				desconectar();
				flag=1;
				break;
				}
			default:
				{
				flag=0;
				}
			}
		}
	while(flag==0);

	/*CIERRE DE LA CONEXION CON EL SERVIDOR*/
	close(sockfd);

	return 0;
	}

/********************************************************************************************************/
void *esperar_datos(void *parametro)
	{
	char *tmp,*resto,temporal[N],*temporal2,temporal3[N];
	char recepcion[N],envio[N];	

	int i;
	int j;

	while(1)
		{
		
		/**********************************/
		/*** SECCION DE EXCLUSION MUTUA ***/
		pthread_mutex_lock(&protege_socket);	
	
		/*ESCHUCHAR DENTRO DEL SOCKET ESPERANDO SOLICITUD*/
		read(sockfd,recepcion,N);

		/*LLEGO LA SEÑAL*/
		strcpy(temporal,recepcion);
		resto=strtok(temporal,":");
		temporal2=strtok(NULL,":");
		
		/*SUBIR EL ARCHIVO*/
		if (strcmp(resto,"SSA")==0)
			{
			subir(temporal2);
			menu();
			}
		else
			{
			strcpy(envio,"NDA:");
			write(sockfd,envio,N);
			}

		pthread_mutex_unlock(&protege_socket);
		/*** FIN SECCION DE EXCLUSION MUTUA ***/
		/**************************************/
				
		for (i=0;i<1000;i++)
			{
			j=j/110001;
			}
		}
	}

/********************************************************************************************************/
int listar_clientes()
	{
	char temporal[N];
	char *resto;
	char *tmp;
	char *temporal2;	
	char temporal3[N];
	char temporal4[N];
	char *temporal5;
	char *temporal6;	

	int flag;
	int i;
	int valor;
			
	/*ENVIAR SEÑAL AL SERVIDOR QUE SOLICITE LA LISTA DE CLIENTES*/
	
	printf("LISTA DE CLIENTES DISPONIBLES:\n");
	printf("------------------------------\n\n");
	
	/**********************************/
	/*** SECCION DE EXCLUSION MUTUA ***/
	pthread_mutex_lock(&protege_socket);	
	
	/*EVIANDO SOLICITUD*/
	/*CEÑIRSE AL CICLO ESTABLECIDO*/

	/*LEER EL CANAL*/
	read(sockfd,recepcion,N);
	
	/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
	/*PUEDEN OCURRIR PROBLEMAS ACA... PUEDE SER QUE EN ESE INSTANTE SE HAYA ENVIADO UNA SOLICITUD
	DE PARTE DEL SERVIDOR PARA SUBIR UN ARCHIVO... EN CASO DE OCURRIR TAL COSA TOMAR COMO PRIORITARIA
	LA SOLICITUD Y SUSPENDER LA ACCION ACTUAL*/
	/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

	/*LLEGO LA SEÑAL*/
	strcpy(temporal,recepcion);
	resto=strtok(temporal,":");
	temporal2=strtok(NULL,":");
			
	/*SUBIR EL ARCHIVO EN CASO DE SOLICITUD DE SUBIR ARCHIVO*/
	if (strcmp(resto,"SSA")==0)
		{
		subir(temporal2);
		pthread_mutex_unlock(&protege_socket);
		/*** FIN SECCION DE EXCLUSION MUTUA ***/
		/**************************************/
		
		i=-1;
		}
	
	/*EN CASO DE NO EXISTIR TAL SOLICITUD*/	
	else
		{
		/*CONSTRUIR Y ENVIAR SOLICITUD DE LISTA DE CLIENTES*/
		strcpy(temporal,"SLC:");
		write(sockfd,temporal,N);

		flag=0;
		i=0;
		do
			{
			/*ESPERAR QUE ESTA LLEGUE Y PROCESARLA*/

			read(sockfd,temporal,N);
				
			if(strcmp(temporal,"TERMINO")==0)
				{
				flag=1;
				}		
			else
				{
				i=i+1;
				
				strcpy(temporal4,temporal);
				temporal5=strtok(temporal4,":");	
				temporal6=strtok(NULL,":");
				valor=atoi(temporal6);
				
				if (valor==1)
					{								
					printf("%i %s\n",i,temporal5);			
					/*ARREGLO DE CLIENTES*/
					lista_clientes[i-1]=1;		
					}
				else
					{
					lista_clientes[i-1]=0;
					}
				}		
			}
		while (flag==0);
		pthread_mutex_unlock(&protege_socket);
		/*** FIN SECCION DE EXCLUSION MUTUA ***/
		/**************************************/
		}

	return(i);
	}

/********************************************************************************************************/
int listar_archivos(int i)
	{
	/* i =  id del cliente dentro del servidor*/
	char temporal[N];
	char *resto;
	char *tmp;
	char *temporal2;
	char temporal3[N];
	char temporal4[N];

	int j;
	int flag;	

	printf("\n");
	printf("ARCHIVOS DEL CLIENTE:\n");
	printf("---------------------\n");
		
	/*LISTAR ARCHIVOS DISPONIBLES*/	
	
	/**********************************/
	/*** SECCION DE EXCLUSION MUTUA ***/
	pthread_mutex_lock(&protege_socket);

	/*CEÑIRSE AL CICLO ESTABLECIDO*/

	/*LEER EL CANAL*/
	read(sockfd,recepcion,N);
	
	/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
	/*PUEDEN OCURRIR PROBLEMAS ACA... PUEDE SER QUE EN ESE INSTANTE SE HAYA ENVIADO UNA SOLICITUD
	DE PARTE DEL SERVIDOR PARA SUBIR UN ARCHIVO... EN CASO DE OCURRIR TAL COSA TOMAR COMO PRIORITARIA
	LA SOLICITUD Y SUSPENDER LA ACCION ACTUAL*/
	/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

	/*LLEGO LA SEÑAL*/
	strcpy(temporal,recepcion);
	resto=strtok(temporal,":");
	temporal2=strtok(NULL,":");
	
	/*SUBIR EL ARCHIVO EN CASO DE SOLICITUD DE SUBIR ARCHIVO*/
	if (strcmp(resto,"SSA")==0)
		{
		subir(temporal2);
		pthread_mutex_unlock(&protege_socket);
		/*** FIN SECCION DE EXCLUSION MUTUA ***/
		/**************************************/
		j=-1;
		}
	
	/*EN CASO DE NO EXISTIR TAL SOLICITUD*/	
	else
		{		
	
		/*EVIANDO SOLICITUD*/
		strcpy(temporal4,"SLA");

		sprintf(temporal4,"%s:%i:",temporal4,i);

		write(sockfd,temporal4,N);

		flag=0;
		j=0;
		do
			{
			/*ESPERAR QUE ESTA LLEGUE Y PROCESARLA*/
			
			printf("\n");
			read(sockfd,temporal4,N);
	
			if(strcmp(temporal4,"TERMINO")==0)
				{
				flag=1;
				}		
			else
				{
				j=j+1;
				printf("%i %s",j,temporal4);
				}
		
			}
		while (flag==0);
		pthread_mutex_unlock(&protege_socket);
		/*** FIN SECCION DE EXCLUSION MUTUA ***/
		/**************************************/
		}

	
	return(j);

	}
/********************************************************************************************************/
void subir(char arch_enviar[])
	{
	/*HEBRA QUE TRANSFIERE INFORMACION AL SERVIDOR,SE INICIA CUANDO LLEGA LA SEÑAL*/
		
	char temporal[N];
	char temporal2[N];

	/*INFORMAR AL USUARIO*/
	system("clear");
	printf("SOLICITUD DE TRANSFERENCIA DE ARCHIVO:\n");
	printf("======================================\n\n");
	printf("EL SERVIDOR CON DIRECCION IP : %s.\n",dir_servidor);
	printf("HA SOLICITADO EL ARCHIVO	 : %s.\n\n",arch_enviar);
	printf("SU OPERACION HA SIDO INTERRUMPIDA.\n");
	printf("ESPERE MIENTRAS SE HACE LA TRANSFERENCIA...\n");
	
	/*ABRIR ARCHIVO*/
	lector=fopen(arch_enviar,"r");

	if (lector==NULL)
		{
		printf("*** NO SE ENCUENTRA ARCHIVO SOLICITADO ***\n");
		printf("*** LA TRANSFERENCIA HA FALLADO !!!! ***\n");
		write(sockfd,"NOEXISTEARCHIVO",N);
		}
	else
		{
		write(sockfd,"EXISTEARCHIVO",N);
		while(!feof(lector))
			{
			/*CONSTRUIR EL MENSAJE*/
			if(fgets(temporal,N,lector)!=NULL)
				{
				/*ENVIAR LA LINEA*/	
				write(sockfd,temporal,N);
				}
			}

		fclose(lector);	

		write(sockfd,"TERMINO",N);

		printf("\n*** LA TRANSFERENCIA HA FINALIZADO. ***");
		}

	system("sleep 3");
	
	}
/********************************************************************************************************/
void listar()
	{
	int num_clientes;
	int num_archivo;
	char opc_str[L];
	char temporal[N];
	int opc;
	int flag;

	printf("LISTAR ARCHIVOS DE UN CLIENTE:\n");
	printf("==============================\n\n");
	
	num_clientes=listar_clientes();
	
	/*VIENDO SI HAY INTERRUPCION DEL PROCESO*/
	if (num_clientes!=-1)
		{	
	
		flag=0;
		do
			{
			/*SELECCIONAR UN CLIENTE*/
			printf("\nELIJA UN CLIENTE : ");		

			scanf("%s",opc_str);
			if(validar(opc_str)!=-1)
				{
				opc=atoi(opc_str);
				if ( (opc>=1) && (opc<=num_clientes) && (validar_clientes(opc)==1) )
					{
					flag=1;
					}
				else
					{
					flag=0;
					}
				}
			}
		while(flag==0);

		printf("\n");
			
		/*LISTAR LOS ARCHIVOS*/
		num_archivo=listar_archivos(opc);
	
		/*FIN*/
		system("sleep 5");
		}
	
	}
/********************************************************************************************************/
void bajar()
	{
	int num_clientes;
	int num_archivo;
	char opc_str[L];
	char temporal[N],*temporal2,temporal3[N];
	char cliente[N];
	char archivo[N];	
	char *resto;
	char *tmp;

	int opc1;
	int opc2;
	int flag;

	system("clear");
	
	printf("BAJAR ARCHIVOS DE UN CLIENTE:\n");
	printf("=============================\n\n");
	
	num_clientes=listar_clientes();
	
	/*VIENDO SI HAY INTERRUPCION DEL PROCESO*/
	if (num_clientes!=-1)
		{		
		flag=0;
		do
			{	
			/*SELECCIONAR UN CLIENTE*/
			printf("\nELIJA UN CLIENTE : ");

			scanf("%s",opc_str);
			if(validar(opc_str)!=-1)
				{
				opc1=atoi(opc_str);
				if ( (opc1>=1) && (opc1<=num_clientes) && (validar_clientes(opc1)==1) )
					{
					flag=1;
					}
				else
					{
					flag=0;
					}
				}
			}
		while(flag==0);

		printf("\n");
			
		/*LISTAR LOS ARCHIVOS*/
		num_archivo=listar_archivos(opc1);
	
		if (num_archivo!=-1)
			{
			flag=0;
			do
				{
				/*SELECCIONAR UN ARCHIVO*/
				printf("\nELIJA UN ARCHIVO : ");
		
				scanf("%s",opc_str);
				if(validar(opc_str)!=-1)
					{
					opc2=atoi(opc_str);
			
					if ( (opc2>=1) && (opc2<=num_archivo) )
						{
						flag=1;
						}
					else
						{
						flag=0;
						}
					}
				}
			while(flag==0);
			
		
			/**********************************/
			/*** SECCION DE EXCLUSION MUTUA ***/
			pthread_mutex_lock(&protege_socket);		

			/*LEER NOMBRE DEL ARCHIVO*/
			read(sockfd,temporal,N);

			/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
			/*PUEDEN OCURRIR PROBLEMAS ACA... PUEDE SER QUE EN ESE INSTANTE SE HAYA ENVIADO UNA SOLICITUD
			DE PARTE DEL SERVIDOR PARA SUBIR UN ARCHIVO... EN CASO DE OCURRIR TAL COSA TOMAR COMO PRIORITARIA
			LA SOLICITUD Y SUSPENDER LA ACCION ACTUAL*/
			/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

			/*LLEGO LA SEÑAL*/
			strcpy(temporal,recepcion);
			resto=strtok(temporal,":");
			temporal2=strtok(NULL,":");
					
			/*SUBIR EL ARCHIVO EN CASO DE SOLICITUD DE SUBIR ARCHIVO*/
			if (strcmp(resto,"SSA")==0)
				{
				subir(temporal2);
				pthread_mutex_unlock(&protege_socket);
				/*** FIN SECCION DE EXCLUSION MUTUA ***/
				/**************************************/
				}
			else
				{
				ftime(&intermedio);

				/*ENVIAR SOLICITUD DE ENVIO DE ARCHIVO*/
				sprintf(temporal,"SBA:%d:%d:",opc1,opc2);

				write(sockfd,temporal,N);
		
				printf("*** ESPERANDO RESPUESTA DEL SERVIDOR. ***\n");
				
				/*BAJANDO EL ARCHIVO*/		
				read(sockfd,temporal,N);

				if(strcmp(temporal,"NOEXISTEARCHIVO")==0)
					{
					printf("*** NO SE ENCUENTRA ARCHIVO SOLICITADO ***\n");
					printf("*** LA TRANSFERENCIA HA FALLADO !!!! ***\n");
					system("sleep 2");
					}				
				else
					{
					lector=fopen(temporal,"w");

					printf("*** RECIBIENDO ARCHIVO : %s ***\n",temporal);
		
					flag=0;
					do
						{
						/*LEER DE SERVIDOR*/
						read(sockfd,temporal,N);
	
						if (strcmp(temporal,"TERMINO")==0)
							{
							flag=1;
							}
						else
							{
							/*ESCRIBIR EN EL ARCHIVO*/
							fprintf(lector,"%s",temporal);
							}
						}		
					while(flag==0);

					ftime(&fin);
					tiempo=(double)(fin.time-intermedio.time);

					printf("*** SE HA RECIBIDO EL ARCHIVO CORRECTAMENTE. ***\n");
					printf("*** TIEMPO DEMORADO : %.0f s ***\n",tiempo);
					system("sleep 2");
				
					fclose(lector);
					}

				pthread_mutex_unlock(&protege_socket);
				/*** FIN SECCION DE EXCLUSION MUTUA ***/
				/**************************************/
				}
			}
		}
	}
/********************************************************************************************************/
void menu()
	{
	ftime(&fin);
	tiempo=(double)(fin.time-inicio.time);

	system("clear");

	printf("\nPROGRAMA CLIENTE:\n");
	printf("=================\n\n");
	printf("*** CONECTADO A %s ***\n",dir_servidor);
	printf("*** TIEMPO DE CONEXION %.0f s ***\n\n",tiempo);
	printf("1 VER LISTADO DE ARCHIVOS DE UN CLIENTE CONECTADO AL SERVIDOR.\n");
	printf("2 BAJAR UN ARCHIVO DE UN CLIENTE CONECTADO AL SERVIDOR.\n");
	printf("3 ACTUALIZAR TIEMPO DE CONEXION.\n");
	printf("4 CERRAR CLIENTE.\n\n");
	printf("ELIJA UNA OPCION:\n\n");
	}
/********************************************************************************************************/
void desconectar()
	{
	char temporal[N];
	char *resto;
	char *tmp;
	char *temporal2;	
	char temporal3[N];	

	/**********************************/
	/*** SECCION DE EXCLUSION MUTUA ***/
	pthread_mutex_lock(&protege_socket);	
	
	/*EVIANDO SOLICITUD*/
	/*CEÑIRSE AL CICLO ESTABLECIDO*/

	/*LEER EL CANAL*/
	read(sockfd,recepcion,N);
	
	/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
	/*PUEDEN OCURRIR PROBLEMAS ACA... PUEDE SER QUE EN ESE INSTANTE SE HAYA ENVIADO UNA SOLICITUD
	DE PARTE DEL SERVIDOR PARA SUBIR UN ARCHIVO... EN CASO DE OCURRIR TAL COSA TOMAR COMO PRIORITARIA
	LA SOLICITUD Y SUSPENDER LA ACCION ACTUAL*/
	/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

	/*LLEGO LA SEÑAL*/
	strcpy(temporal,recepcion);
	resto=strtok(temporal,":");
	temporal2=strtok(NULL,":");
		
	/*SUBIR EL ARCHIVO EN CASO DE SOLICITUD DE SUBIR ARCHIVO*/
	if (strcmp(resto,"SSA")==0)
		{
		subir(temporal2);
		pthread_mutex_unlock(&protege_socket);
		/*** FIN SECCION DE EXCLUSION MUTUA ***/
		/**************************************/		
		}
	
	/*EN CASO DE NO EXISTIR TAL SOLICITUD*/	
	else
		{
		/*CONSTRUIR Y ENVIAR SOLICITUD DE LISTA DE CLIENTES*/
		strcpy(temporal,"DES:");
		write(sockfd,temporal,N);

		pthread_mutex_unlock(&protege_socket);
		/*** FIN SECCION DE EXCLUSION MUTUA ***/
		/**************************************/
		}

	}
/********************************************************************************************************/
int validar(char valida[])
	{
	/*DEVOLVER -1 CUANDO HAY ERROR*/
	int i;
	int numero,flg_error,pos;
	char copia[1];

	flg_error=0;
	pos=-1;

	i=0;
	/*REVIZANDO POR SI HAY LETRAS*/
	while (valida[i]!='\0')
		{
		copia[0]=valida[i];
		numero=atoi(copia);
		if ( (numero==0)&&(copia[0]!='0')&&(copia[0]!='-') )
			{
			flg_error=-1;
			}
		i=i+1;
		pos=pos+1;
		}
	/*REVISANDO SI CUMPLE CON LONGITUD*/
	if (valida[0]=='-')
		{
		if (pos>4)
			{
			flg_error=-1;
			}
		}
	else
		{
		if (pos>3)
			{
			flg_error=-1;
			}
		}
	return (flg_error);
	}

/********************************************************************************************************/
int validar_clientes(int numero)
	{
	if ( (lista_clientes[numero-1]==1) && (IDENTIFICADOR!=numero-1) )
		{
		return(1);
		}
	else
		{
		return(0);
		}		
	}