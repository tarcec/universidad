#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int tablero[8][8];
char *arch_entrada="d:\\knight.in";
char *arch_salida="d:\\knight.out";
FILE *lector,*escritor;

int minim;

void insertar_tablero(char coord1,int coord2,int modo);
int insertar_num(int i,int j,int acum);
void limpiar_tablero();
void minimo();
/*******************************************************/
main()
	{
	char valor1;
	char valor2[1];
	char valor3;
	char valor4[1];
	char basura;
	int numero1;
	int numero2;

	lector=fopen(arch_entrada,"r");
	escritor=fopen(arch_salida,"w");

	/*LECTURA Y PROCESADOS DE CASOS HASTA EOF*/
	while (!feof(lector))
		{
		minim=0;

		/*LIMPIAR LA MATRIZ*/
		limpiar_tablero();

		/*LECTURA DE LA CASILLA DE ORIGEN*/
		valor1=fgetc(lector);

		/*TRUNCADO DE CASO,CUANDO EN VEZ DE EOF SE TIENE
		UNA LINEA VACIA*/
		if ( (valor1=='a')||(valor1=='b')||(valor1=='c')||(valor1=='d')||(valor1=='e')||(valor1=='f')||(valor1=='g')||(valor1=='h'))
			{
			valor2[0]=fgetc(lector);
			numero1=atoi(valor2);
			insertar_tablero(valor1,numero1,0);

			/*LECTURA DEL ESPACIO*/
			basura=fgetc(lector);

			/*LECTURA DE LA CASILLA DE DESTINO*/
			valor3=fgetc(lector);
			valor4[0]=fgetc(lector);
			numero2=atoi(valor4);
			insertar_tablero(valor3,numero2,-2);

			/*PROCESAR EL CASO*/
			if ( (valor1==valor3)&&(numero1==numero2) )
				{
				minim=0;
				}
			else
				{
				minimo();
				}

			/*ESCRIBIR RESULTADO*/
			fprintf(escritor,"To get from %c%c to %c%c takes %i knight moves.\n",valor1,valor2[0],valor3,valor4[0],minim);

			/*LEER SALTO LINEA*/
			basura=fgetc(lector);
			}
		}
	fclose(lector);
	fclose(escritor);
	}
/*******************************************************/
void limpiar_tablero()
	{
	int i;
	int j;

	for (i=0;i<=7;i++)
		{
		for (j=0;j<=7;j++)
			{
			tablero[i][j]=-1;
			}
		}
	}
/*******************************************************/
void insertar_tablero(char coord1,int coord2,int modo)
	{
	switch (coord1)
		{
		case 'a':
			{
			tablero[8-coord2][0]=modo;
			break;
			}
		case 'b':
			{
			tablero[8-coord2][1]=modo;
			break;
			}
		case 'c':
			{
			tablero[8-coord2][2]=modo;
			break;
			}
		case 'd':
			{
			tablero[8-coord2][3]=modo;
			break;
			}
		case 'e':
			{
			tablero[8-coord2][4]=modo;
			break;
			}
		case 'f':
			{
			tablero[8-coord2][5]=modo;
			break;
			}
		case 'g':
			{
			tablero[8-coord2][6]=modo;
			break;
			}
		case 'h':
			{
			tablero[8-coord2][7]=modo;
			break;
			}
		}
	}
/*******************************************************/
void minimo()
	{
	int i;
	int j;
	int numero;
	int flag;

	numero=0;
	flag=0;

	do
		{
		for (i=0;i<=7;i++)
			{
			for (j=0;j<=7;j++)
				{
				flag=0;

				if (tablero[i][j]==numero)
					{
					/*JUGAR*/

					/*POSIBILIDAD 1*/
					if ( ((i-2)>=0) && ((j-1)>=0) )
						{
						flag=insertar_num(i-2,j-1,numero);
						if (flag==1)
							{
							break;
							}
						}

					/*POSIBILIDAD 2*/
					if ( ((i-2)>=0) && ((j+1)<=7) )
						{
						flag=insertar_num(i-2,j+1,numero);
						if (flag==1)
							{
							break;
							}
						}

					/*POSIBILIDAD 3*/
					if ( ((i-1)>=0) && ((j-2)>=0) )
						{
						flag=insertar_num(i-1,j-2,numero);
						if (flag==1)
							{
							break;
							}
						}

					/*POSIBILIDAD 4*/
					if ( ((i-1)>=0) && ((j+2)<=7) )
						{
						flag=insertar_num(i-1,j+2,numero);
						if (flag==1)
							{
							break;
							}
						}

					/*POSIBILIDAD 5*/
					if ( ((i+1)<=7) && ((j-2)>=0) )
						{
						flag=insertar_num(i+1,j-2,numero);
						if (flag==1)
							{
							break;
							}
						}

					/*POSIBILIDAD 6*/
					if ( ((i+1)<=7) && ((j+2)<=7) )
						{
						flag=insertar_num(i+1,j+2,numero);
						if (flag==1)
							{
							break;
							}
						}

					/*POSIBILIDAD 7*/
					if ( ((i+2)<=7) && ((j-1)>=0) )
						{
						flag=insertar_num(i+2,j-1,numero);
						if (flag==1)
							{
							break;
							}
						}

					/*POSIBILIDAD 8*/
					if ( ((i+2)<=7) && ((j+1)<=7) )
						{
						flag=insertar_num(i+2,j+1,numero);
						if (flag==1)
							{
							break;
							}
						}
					}
				if (flag==1)
					{
					break;
					}
				}
			if (flag==1)
				{
				break;
				}
			}
			numero=numero + 1;
		}
	while(flag==0);
	}

/*******************************************************/
int insertar_num(int i,int j,int acum)
	{
	int retorno;

	if (tablero[i][j]==-2)
		{
		minim=acum + 1;
		retorno=1;
		}
	else
		{
		if (tablero[i][j]==-1)
			{
			tablero[i][j]=acum + 1;
			}
		retorno=0;
		}
	return(retorno);
	}