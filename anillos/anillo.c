#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>


FILE *lector;

int n,i,v;

void creacion()
	{
	
	/* i : identificacion interna del proceso*/
	pid_t pid;
	
	int j,k;	
	int process;
	int resultado;
	char resul[80];
	int recibido;
	char recib[80];
	pid_t temporal;
	char temp[80];

	int Enviar[2];
	int Recibir[2];
	int proceso1[2];
	int proceso2[2];

	int EnviarULT[2];
	int RecibirULT[2];
	int proceso1ULT[2];
	int proceso2ULT[2];
	
	int puerta_salida_sum;
	int puerta_salida_rest;
	int puerta_entrada_sum;
	int puerta_entrada_rest;
	int puerta_entrada_pro1;
	int puerta_entrada_pro2;
	int puerta_salida_pro1;
	int puerta_salida_pro2;


	char nombre_archivo[120];	
	char pidtmp[100];	

	int status;

	/*PIPEAR ULTIMOS CANALES*/
	pipe(EnviarULT);	
	pipe(RecibirULT);
	pipe(proceso1ULT);
	pipe(proceso2ULT);	
		

	for (j=0;j<n;j++)
		{
		if (j==n-1)
			{
			/*NO SE CREAN MAS PIPES, ESTAN ARRIBA*/
			
			/*CERRAR ULTIMOS CANALES*/
			close(EnviarULT[0]);
			close(RecibirULT[1]);
			close(proceso1ULT[0]);
			close(proceso2ULT[1]);

			puerta_salida_sum=EnviarULT[1];
			puerta_entrada_rest=RecibirULT[0];
			puerta_salida_pro1=proceso1ULT[1]; 
			puerta_entrada_pro2=proceso2ULT[0];			
			}			
		else	
			{
			/*PIPEANDO CANALES GENERALES*/
			pipe(Enviar);	
			pipe(Recibir);
			pipe(proceso1);
			pipe(proceso2);
		
			pid=fork();
		
			if (pid==0)
				{
				/*ESTAMOS EN EL HIJO*/

				/*CERRAR CANALES DE COMUNICACION*/
				close(Enviar[1]);
				close(Recibir[0]);
				close(proceso1[1]);
				close(proceso2[0]);

				puerta_entrada_sum=Enviar[0];
				puerta_salida_rest=Recibir[1];
				puerta_entrada_pro1=proceso1[0];
				puerta_salida_pro2=proceso2[1];				

				/*SE DEVUELVE A CREAR OTRO HIJO*/				
				}
			else
				{
				/*ESTAMOS EN EL PADRE	
			
				/*SI SE ESTA EN EL PRIMER CANAL
				CERRAR ULTIMOS CANALES PARA CREAR ANILLO*/
				if (j==0)
					{
					close(EnviarULT[1]);
					close(RecibirULT[0]);
					close(proceso1ULT[1]);
					close(proceso2ULT[0]);

					puerta_entrada_sum=EnviarULT[0];
					puerta_salida_rest=RecibirULT[1];
					puerta_entrada_pro1=proceso1ULT[0];
					puerta_salida_pro2=proceso2ULT[1];							
					}				
				/*CERRAR CANALES DE COMUNICACION*/
				close(Enviar[0]);
				close(Recibir[1]);
				close(proceso1[0]);
				close(proceso2[1]);
			
				puerta_salida_sum=Enviar[1];
				puerta_entrada_rest=Recibir[0];
				puerta_salida_pro1=proceso1[1];
				puerta_entrada_pro2=proceso2[0];				

				/*TERMINA CON CREACION DE HIJOS*/
				break;
				}
			}		
		}
	if (j==n)
		{
		j=n-1;
		}	
	
	/*OPERACIONES*/
	/*EVENTUALMENTE TENEMOS QUE ESTAR EN UN CICLo*/
	
	for (k=1;k<=v;k++)
		{	
		/**** CASO INICIAL ****/
		if (j==0)
			{			
			if (k==1)
				{
				/*SUMANDO*/
				temporal = getpid();				
		
				resultado=i+1;
				sprintf(resul,"%d",resultado);
		
				write(puerta_salida_sum,resul,sizeof(resul));
					
				/*RECIBIR Y ENVIAR PID*/
				sprintf(resul,"%d",temporal);
				write(puerta_salida_pro1,resul,sizeof(resul));		
			
				/*ABRIR Y GUARDAR EN LOG*/
						
				strcpy(nombre_archivo,"log_");
				sprintf(temp,"%d.txt",temporal);
				strcat(nombre_archivo,temp);
						
				lector=fopen(nombre_archivo,"a+");
				fprintf(lector,"+ * %d %d\n",i,resultado);
				fclose(lector);
		
				/*RESTANDO*/
				resultado=i-1;
				sprintf(resul,"%d",resultado);

				write(puerta_salida_rest,resul,sizeof(resul));
						
				/*RECIBIR Y ENVIAR PID*/
				sprintf(resul,"%d",temporal);
				write(puerta_salida_pro2,resul,sizeof(resul));		

				/*ABRIR Y GUARDAR EN LOG*/
				temporal = getpid();
				strcpy(nombre_archivo,"log_");
				sprintf(temp,"%d.txt",temporal);
				strcat(nombre_archivo,temp);
				lector=fopen(nombre_archivo,"a+");

				fprintf(lector,"- * %d %d\n",i,resultado);

				fclose(lector);
				}
			/*OTROS CASOS*/
			else	/*k>1*/
				{
				temporal = getpid();
						
				/*CASO SUMA Y RESTA EN EL PRIMER PROCESO*/
						
				/**** SUMANDO ****/

				read(puerta_entrada_sum,recib,sizeof(recib));
				recibido = atoi(recib);
				resultado=recibido+1;
				sprintf(resul,"%d",resultado);

				write(puerta_salida_sum,resul,sizeof(resul));

				/*RECIBIR Y ENVIAR PID*/
				read(puerta_entrada_pro1,pidtmp,sizeof(pidtmp));
				sprintf(resul,"%d",temporal);
				write(puerta_salida_pro1,resul,sizeof(resul));		
												
				/*ABRIR Y GUARDAR EN LOG*/
						
				strcpy(nombre_archivo,"log_");
				sprintf(temp,"%d.txt",temporal);
				strcat(nombre_archivo,temp);
				lector=fopen(nombre_archivo,"a+");
					
				fprintf(lector,"+ %s %d %d\n",pidtmp,recibido,resultado);

				fclose(lector);
					
				/*** RESTANDO ***/

				read(puerta_entrada_rest,recib,sizeof(recib));
				recibido = atoi(recib);
				resultado=recibido-1;
				sprintf(resul,"%d",resultado);
	
				write(puerta_salida_rest,resul,sizeof(resul));
										
				/*RECIBIR Y ENVIAR PID*/
				read(puerta_entrada_pro2,pidtmp,sizeof(pidtmp));
				sprintf(resul,"%d",temporal);
				write(puerta_salida_pro2,resul,sizeof(resul));					

				//*ABRIR Y GUARDAR EN LOG*/
				strcpy(nombre_archivo,"log_");
				sprintf(temp,"%d.txt",temporal);
				strcat(nombre_archivo,temp);
				lector=fopen(nombre_archivo,"a+");
			
				fprintf(lector,"- %s %d %d\n",pidtmp,recibido,resultado);

				fclose(lector);

				if (k==v)
					{
					temporal=getpid();
	
					/*HACER ESCRITURA ADICIONAL*/
					/*SUMANDO*/

					read(puerta_entrada_sum,recib,sizeof(recib));
					recibido = atoi(recib);
					
					/*RECIBIR Y ENVIAR PID*/
					read(puerta_entrada_pro1,pidtmp,sizeof(pidtmp));
						
					/*ABRIR Y GUARDAR EN LOG*/
					strcpy(nombre_archivo,"log_");
					sprintf(temp,"%d.txt",temporal);
					strcat(nombre_archivo,temp);
					lector=fopen(nombre_archivo,"a+");
					
					fprintf(lector,"+ %s %d *\n",pidtmp,recibido);

					fclose(lector);
					
					/*RESTANDO*/
		
					read(puerta_entrada_rest,recib,sizeof(recib));
					recibido = atoi(recib);
					
					/*RECIBIR Y ENVIAR PID*/
					read(puerta_entrada_pro2,pidtmp,sizeof(pidtmp));
								
					//*ABRIR Y GUARDAR EN LOG*/
					strcpy(nombre_archivo,"log_");
					sprintf(temp,"%d.txt",temporal);
					strcat(nombre_archivo,temp);
					lector=fopen(nombre_archivo,"a+");
					
					fprintf(lector,"- %s %d *\n",pidtmp,recibido);

					fclose(lector);
					}
				}					
			}					
		
		else
			{
			if (j==n-1)
				{
				/*ULTIMO CASO*/
				temporal = getpid();
					
				/*SUMANDO*/
				read(puerta_entrada_sum,recib,sizeof(recib));
				recibido = atoi(recib);
				resultado=recibido+1;
				sprintf(resul,"%d",resultado);
				
				write(puerta_salida_sum,resul,sizeof(resul));
	
				/*RECIBIR Y ENVIAR PID*/
				
				read(puerta_entrada_pro1,pidtmp,sizeof(pidtmp));
				sprintf(resul,"%d",temporal);
				write(puerta_salida_pro1,resul,sizeof(resul));				

				/*ABRIR Y GUARDAR EN LOG*/
				strcpy(nombre_archivo,"log_");
				sprintf(temp,"%d.txt",temporal);
				strcat(nombre_archivo,temp);
	
				lector=fopen(nombre_archivo,"a+");
											
				fprintf(lector,"+ %s %d %d\n",pidtmp,recibido,resultado);

				fclose(lector);
						
				/*RESTANDO*/
				read(puerta_entrada_rest,recib,sizeof(recib));
				recibido = atoi(recib);
				resultado=recibido-1;
				sprintf(resul,"%d.txt",resultado);
					
	
				write(puerta_salida_rest,resul,sizeof(resul));
						
				/*RECIBIR Y ENVIAR PID*/
				read(puerta_entrada_pro2,pidtmp,sizeof(pidtmp));
				sprintf(resul,"%d",temporal);
				write(puerta_salida_pro2,resul,sizeof(resul));				
						
				//*ABRIR Y GUARDAR EN LOG*/
				strcpy(nombre_archivo,"log_");
				sprintf(temp,"%d.txt",temporal);
				strcat(nombre_archivo,temp);
				lector=fopen(nombre_archivo,"a+");
				fprintf(lector,"- %s %d %d\n",pidtmp,recibido,resultado);
			
				fclose(lector);
				}
			else
				{
				/**** CASO NORMALES ****/
				temporal=getpid();

				/*SUMANDO*/		

				read(puerta_entrada_sum,recib,sizeof(recib));
					
				recibido = atoi(recib);
				resultado=recibido+1;
				sprintf(resul,"%d",resultado);


				write(puerta_salida_sum,resul,sizeof(resul));

				/*RECIBIR Y ENVIAR PID*/
				read(puerta_entrada_pro1,pidtmp,sizeof(pidtmp));
				sprintf(resul,"%d",temporal);
				write(puerta_salida_pro1,resul,sizeof(resul));	
					
				/*ABRIR Y GUARDAR EN LOG*/
				strcpy(nombre_archivo,"log_");
				sprintf(temp,"%d.txt",temporal);
				strcat(nombre_archivo,temp);
				lector=fopen(nombre_archivo,"a+");
					
				fprintf(lector,"+ %s %d %d\n",pidtmp,recibido,resultado);

				fclose(lector);
					
				/*RESTANDO*/

				read(puerta_entrada_rest,recib,sizeof(recib));
				recibido = atoi(recib);
				resultado=recibido-1;
				sprintf(resul,"%d",resultado);

				write(puerta_salida_rest,resul,sizeof(resul));
						
				/*RECIBIR Y ENVIAR PID*/
				read(puerta_entrada_pro2,pidtmp,sizeof(pidtmp));
				sprintf(resul,"%d",temporal);
				write(puerta_salida_pro2,resul,sizeof(resul));	

				/*ABRIR Y GUARDAR EN LOG*/
				strcpy(nombre_archivo,"log_");
				sprintf(temp,"%d.txt",temporal);
				strcat(nombre_archivo,temp);
				lector=fopen(nombre_archivo,"a+");
					
				fprintf(lector,"- %s %d %d\n",pidtmp,recibido,resultado);
				fclose(lector);
				}
			}
		}
	/*ESPERAR A LOS HIJOS*/
	waitpid(pid,&status,0);					
	}

/***************************************************************************************/
int main (int argc, char **argv)
	{
	char *PParametro = NULL; 
	char *SParametro = NULL;
	char *TParametro = NULL;

	int j,k;
	int indice;
	int Valida;
	 
	opterr = 0;
		
	while ((Valida = getopt (argc, argv, "n:i:v:")) != -1)		
		switch (Valida)
			{
			case 'n':
					PParametro = optarg;
					n = atoi(PParametro);
			break;
	
			case 'i':
					SParametro = optarg;
					i = atoi(SParametro);
				break;
	
			case 'v':
					TParametro = optarg;
					v = atoi(TParametro); 
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
		
	for (indice = optind; indice < argc; indice++)
		{
		printf ("Argumento no valido : %s\n", argv[indice]);
		}
	
	creacion();
		
	
	return 0;
	}

/***************************************************************************************************/
