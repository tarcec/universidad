#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

struct Manos{
	char Cartas[2];
};


int Mazo[13];
int m,j,r;
struct Manos *Players;
pthread_t *Jugadores;
pthread_mutex_t *protege_mazo;
pthread_mutex_t protege_archivo;

FILE *lector;
char *arch="log.out";

/******************************/
char *convierte(int num)
	{
	switch(num)
		{
		case 11:
			{
			return("J");
			break;
			}
		case 12:
			{
			return("Q");
			break;
			}
		case 13:
			{
			return("K");
			break;
			}
		case 1:
			{
			return("As");
			break;
			}
		}
	}
/********************************/
void Inicializar()
	{
	int i;

	for (i=0;i<j+1;i++)
		{
		pthread_mutex_lock(&protege_mazo[i]);
		Players[i].Cartas[0]=0;
		pthread_mutex_unlock(&protege_mazo[i]);
		}

	for (i=0;i<j+1;i++)
		{
		pthread_mutex_lock(&protege_mazo[i+(j+1)]);
		Players[i].Cartas[1]=0;
		pthread_mutex_unlock(&protege_mazo[i+(j+1)]);
		}
	}

/************************************/
void Repartir(int param,int numJuego)
	{
	int k;
	int num,flag;
	char *carta;

	for (k=0;k<j+1;k++)
		{
		do {
			flag=0;
			num=rand()%13;
			pthread_mutex_lock(&protege_mazo[k+(j+1)*param]);

			if (Mazo[num] != 0)
				{
				Mazo[num]--;
				Players[k].Cartas[param]=num+1;
				flag=1;
				}

			pthread_mutex_unlock(&protege_mazo[k+(j+1)*param]);
		}while(flag==0);

		/*ESCRIBIR EN EL ARCHIVO*/
		pthread_mutex_lock(&protege_archivo);
		lector=fopen(arch,"a+");
		if (k==j)
			{
			if ( (num+1>=2) && (num+1<=10) )
				{
				if (param==0)
					{
					fprintf(lector,"CROUPIER : JUEGO %i, REPARTE PRIMERA CARTA = %i A CROUPIER\n",numJuego,num+1);
					}
				else
					{
					fprintf(lector,"CROUPIER : JUEGO %i, REPARTE SEGUNDA CARTA = %i A CROUPIER\n",numJuego,num+1);
					}
				}
			else
				{
				carta=convierte(num+1);
				if (param==0)
					{
					fprintf(lector,"CROUPIER : JUEGO %i, REPARTE PRIMERA CARTA = %s A CROUPIER\n",numJuego,carta);
					}
				else
					{
					fprintf(lector,"CROUPIER : JUEGO %i, REPARTE SEGUNDA CARTA = %s A CROUPIER\n",numJuego,carta);
					}
				}
			}
		else
			{
			if ( (num+1>=2) && (num+1<=10) )
				{
				if (param==0)
					{
					fprintf(lector,"CROUPIER : JUEGO %i, REPARTE PRIMERA CARTA = %i A JUGADOR = %i\n",numJuego,num+1,k);
					}
				else
					{
					fprintf(lector,"CROUPIER : JUEGO %i, REPARTE SEGUNDA CARTA = %i A JUGADOR = %i\n",numJuego,num+1,k);
					}
				}
			else
				{
				carta=convierte(num+1);
				if (param==0)
					{
					fprintf(lector,"CROUPIER : JUEGO %i, REPARTE PRIMERA CARTA = %s A JUGADOR = %i\n",numJuego,carta,k);
					}
				else
					{
					fprintf(lector,"CROUPIER : JUEGO %i, REPARTE SEGUNDA CARTA = %s A JUGADOR = %i\n",numJuego,carta,k);
					}
				}
			}
		fclose(lector);
		pthread_mutex_unlock(&protege_archivo);
		}
	}
/*************************************************/
/********* Funcion de cada hebra jugador *********/

void *Funcion(void *Parametro)
	{
	int NJugador = (int)Parametro;
	int jugElec;

	int *marcado;
	int k,l,n;
	int flag1,flag2;
	int contJugador;
	int puntaje=0;

	char *carta;

	marcado=(int *)malloc((j+1)*sizeof(int));

	for(n=0;n<r;n++)
		{
		for (k=0;k<2;k++)
			{
			contJugador=0;
			for(l=0;l<j+1;l++)
				{
				marcado[l]=0;
				}
			do
				{
				flag1=0;
					do
					{
					flag2=0;
					jugElec=rand()%(j+1);

					if (marcado[jugElec]==0)
						{
						pthread_mutex_lock(&protege_mazo[jugElec+(j+1)*k]);

						if (Players[jugElec].Cartas[k] != 0)
							{
								/*EN EL CASO DE ESTAR REVIZANDO AL JUGADOR*/
								if (jugElec==NJugador)
									{
									/*IMPRIMIR A ARCHIVO DE LOG*/
									pthread_mutex_lock(&protege_archivo);
									lector=fopen(arch,"a+");
									if (NJugador==j)
										{
										if ( (Players[jugElec].Cartas[k]>=2) && (Players[jugElec].Cartas[k]<=10) )
											{
											if (k==0)
												{
												fprintf(lector,"JUGADOR CROUPIER : JUEGO %i, RECIBE PRIMERA CARTA = %i\n",n,Players[jugElec].Cartas[k]);
												}
											else
												{
												fprintf(lector,"JUGADOR CROUPIER : JUEGO %i, RECIBE SEGUNDA CARTA = %i\n",n,Players[jugElec].Cartas[k]);
												}
											}
										else
											{
											carta=convierte(Players[jugElec].Cartas[k]);
											if (k==0)
												{
												fprintf(lector,"JUGADOR CROUPIER : JUEGO %i, RECIBE PRIMERA CARTA = %s\n",n,carta);
												}
											else
												{
												fprintf(lector,"JUGADOR CROUPIER : JUEGO %i, RECIBE SEGUNDA CARTA = %s\n",n,carta);
												}
											}
										}
									else
										{
										if ( (Players[jugElec].Cartas[k]>=2) && (Players[jugElec].Cartas[k]<=10) )
											{
											if (k==0)
												{
												fprintf(lector,"JUGADOR %i : JUEGO %i, RECIBE PRIMERA CARTA = %i DE CROUPIER\n",NJugador,n,Players[jugElec].Cartas[k]);
												}
											else
												{
												fprintf(lector,"JUGADOR %i : JUEGO %i, RECIBE SEGUNDA CARTA = %i DE CROUPIER\n",NJugador,n,Players[jugElec].Cartas[k]);
												}
											}
										else
											{
											carta=convierte(Players[jugElec].Cartas[k]);
											if (k==0)
												{
												fprintf(lector,"JUGADOR %i : JUEGO %i, RECIBE PRIMERA CARTA = %s DE CROUPIER\n",NJugador,n,carta);
												}
											else
												{
												fprintf(lector,"JUGADOR %i : JUEGO %i, RECIBE SEGUNDA CARTA = %s DE CROUPIER\n",NJugador,n,carta);
												}
											}
										}
									fclose(lector);
									pthread_mutex_unlock(&protege_archivo);
									}
								marcado[jugElec]=1;
								contJugador++;
								flag2=1;

								if (Players[jugElec].Cartas[k] == 1 )
									{
									puntaje--;
									}

								else if (Players[jugElec].Cartas[k] >=2 && Players[jugElec].Cartas[k] <=6 )
									{
									puntaje++;
									}
								else if(Players[jugElec].Cartas[k] >=10 && Players[jugElec].Cartas[k] <=13)
									{
									puntaje--;
									}
							if (NJugador!=j)
								{
								printf("%i : %i\n",NJugador,puntaje);
								}
							}
						pthread_mutex_unlock(&protege_mazo[jugElec+(j+1)*k]);
						}
					}
				while(flag2==0);
				if (contJugador==j+1)
					{
					flag1=1;
					}
				}
			while(flag1==0);
			}
		}
	pthread_exit(NULL);
	}
/***********************************************/
/****** Hebra principal : Croupier ************/
int main (int argc, char **argv)
	{	
	char *PParametro = NULL;
	char *SParametro = NULL;
	char *TParametro = NULL;

	int indice;
	int Valida;
	int i,l,p;
	srand(time(0));

	opterr = 0;

	if (fopen(arch,"w+")!=NULL)
		{
		system("rm log.out");
		}

	while ((Valida = getopt (argc, argv, "m:j:r:")) != -1)

			switch (Valida)
			{
				case 'm':
						PParametro = optarg;
						m = atoi(PParametro);
				break;

	   	 		case 'j':
						SParametro = optarg;
						j = atoi(SParametro);
					break;

				case 'r':
						TParametro = optarg;
						r = atoi(TParametro);
					break;

				case '?':
						if (isprint (optopt))
						{
							   		fprintf (stderr, "Argumento no valido `-%c'.\n", optopt);
						}
						else
						{
									fprintf (stderr, "Caracter no valido `\\x%x'.\n", optopt);
						}
						return 1;
				default:
						abort ();
			}

	if ((52*m)<=(j+1)*2)
		{
		printf("NO EXISTEN SUFICIENTES CARTAS PARA JUGAR\n");
		exit(0);
		}

	for (indice = optind; indice < argc; indice++)
		{
		printf ("Argumento no valido : %s\n", argv[indice]);
		}
	/*****************************************************************/
	////////// Creamos los j jugadores y empezamos el juego //////////

	pthread_mutex_init(&protege_archivo,NULL);

	Players = (struct Manos *)malloc((j+1)*sizeof(struct Manos)); //Aqui se guardan las manos
	Jugadores = (pthread_t *)malloc(j*sizeof(pthread_t)); //Hebras de los jugadores
	protege_mazo=(pthread_mutex_t *)malloc(2*(j+1)*sizeof(pthread_mutex_t));

	for (i=0;i<2*(j+1);i++)
		{
		pthread_mutex_init(&protege_mazo[i],NULL);
		}

	Inicializar();

	for (i=0;i<j+1;i++)
		{
		pthread_create(&Jugadores[i],NULL,Funcion,(void *)i);
		}

	/*REPARTIR CARTAS*/
	for (i=0;i<r;i++)
		{
		Inicializar();
		for (p=0;p<13;p++)
			{
			Mazo[p]=4*m;
			}
		for (l=0;l<2;l++)
			{
			Repartir(l,i);
			}
		}

	pthread_exit(NULL);
	return 0;
	}


