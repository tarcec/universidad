#include <stdio.h>
#include <stdlib.h>
#define N 10


void lectura();
void dijsktra();
void consulta(int i,int j);
void inicializar();
void imprime();

typedef struct
	{
	long int costo;
	int nodo;
	int direcc;/*0 SI BAJA EL RIO;1 SI SUBE*/
	int final;
	}
	MINIMO;

char *arch_entrada="aqcuapia.in";
char *arch_salida="aqcuapia.out";

MINIMO matMinimos[N][N];
int matEnlaces[N][N];

int totCiudades;
int totRios;
long totSecciones;
long int totConsultas;

FILE *lector;
FILE *escritor;
/********************************************************************/
int main()
	{
	int i;
	int j;
	int k;
	int flag=0;

	lector=fopen(arch_entrada,"r");
	escritor=fopen(arch_salida,"w");

	if (lector==NULL)
		{
		printf("\nERROR : ARCHIVO 'aqcuapia.in' NO ENCONTRADO. FIN DEL PROGRAMA.");
		return 0;
		}

	/*LECTURA Y PROCESAMIENTO DEL ARCHIVO DE ENTRADA*/

	while (flag==0)
		{
		/*PRIMERA LINEA*/
		fscanf(lector,"%d",&totCiudades);
		fscanf(lector,"%d",&totRios);
		fscanf(lector,"%d",&totSecciones);
		fscanf(lector,"%d",&totConsultas);

		if ((totCiudades==0)&&(totRios==0)&&(totSecciones==0)&&(totConsultas==0))
			{
			/*FIN DE ARCHIVO*/
			flag=1;
			}
		else
			{
			inicializar();

			/*RESTO*/
			lectura();

			/*CALCULO DE DISTANCIAS*/
			dijsktra();

			/*imprime();*/

			fprintf(escritor,"\n");
			/*PROCESAMIENTO DE CONSULTAS*/
			for(i=1;i<=totConsultas;i++)
				{
				fscanf(lector,"%d",&j);
				fscanf(lector,"%d",&k);

				/*HACE LA CONSULTA*/
				consulta(j,k);
				}
			}
		}
	fclose(lector);
	fclose(escritor);
	}

/********************************************************************/
void lectura()
	{
	int basura;
	int i;

	int j;
	int k;

	/*SEGUNDA LINEA*/
	for(i=1;i<=totRios;i++)
		{
		fscanf(lector,"%d",&basura);
		}

	/*PROCESAMIENTO DE ENLACES*/
	for(i=1;i<=totSecciones;i++)
		{
		fscanf(lector,"%d",&j);
		fscanf(lector,"%d",&k);
		matEnlaces[j][k]=1;
		}
	}
/********************************************************************/
void dijsktra()
	{
	int i;
	int j;
	int nodoAct;
	int nodoMin;
	int nodoMinTmp;
	long int costoAcum;
	long int costoMin;

	int direccion;

	/*PARA CADA NODO*/
	for (nodoAct=1;nodoAct<=totCiudades;nodoAct++)
		{
		nodoMin=nodoAct;
		costoAcum=0;

		/*NO SE CONSIDERA EL NODO ACTUAL*/
		matMinimos[nodoAct][nodoAct].final=1;

		/*COMIENZA EL DIJSKTRA*/
		for(j=1;j<totCiudades;j++)
			{
			costoMin=1000000;

			/*BUSCAR SUCESORES*/
			for (i=1;i<=totCiudades;i++)
				{
				if (matEnlaces[nodoMin][i]==1)
					{
					/*SI NO ESTA FINALIZADA*/
					if(matMinimos[nodoAct][i].final==0)
						{
						/*HAY ENLACE VER MINIMO*/
						if (matEnlaces[nodoMin][i]+costoAcum<=matMinimos[nodoAct][i].costo)
							{
							matMinimos[nodoAct][i].costo=matEnlaces[nodoMin][i]+costoAcum;
							matMinimos[nodoAct][i].nodo=nodoMin;
							matMinimos[nodoAct][i].direcc=0;
							}
						}
					}
				}

			/*BUSCAR ANTECESORES*/
			for (i=1;i<=totCiudades;i++)
				{
				if (matEnlaces[i][nodoMin]==1)
					{
					/*SI NO ESTA FINALIZADA*/
					if(matMinimos[nodoAct][i].final==0)
						{
						/*HAY ENLACE VER MINIMO*/
						if (matEnlaces[i][nodoMin]+costoAcum<=matMinimos[nodoAct][i].costo)
							{
							matMinimos[nodoAct][i].costo=matEnlaces[i][nodoMin]+costoAcum;
							matMinimos[nodoAct][i].nodo=nodoMin;
							matMinimos[nodoAct][i].direcc=1;
							}
						}
					}
				}
			/*BUSCAR EL MINIMO PARA LA PROXIMA ITERACION*/
			for(i=1;i<=totCiudades;i++)
				{
				if (matMinimos[nodoAct][i].costo<=costoMin)
					{
					if (matMinimos[nodoAct][i].final==0)
						{
						costoMin=matMinimos[nodoAct][i].costo;
						nodoMinTmp=i;
						}
					}
				}

			/*FINALIZANDO ITERACION*/
			nodoMin=nodoMinTmp;
			costoAcum=costoMin;
			matMinimos[nodoAct][nodoMin].final=1;
			}
		}

	}
/********************************************************************/
void consulta(int i,int j)
	{
	/*DEF DE CABEZALES*/

	int nodoAct=j;
	int cambio=-1;

	int direccion=matMinimos[i][nodoAct].direcc;
	int direccionAct;

	if (i==j)
		{
		/*IMPRIME 0*/
		fprintf(escritor,"0\n");
		}
	else if(matMinimos[i][nodoAct].costo==1000000)
		{
		/*NO ES POSIBLE LLEGAR A ESA CIUDAD*/
		fprintf(escritor,"-1\n");
		}
	else
		{

		/*HACER EL RECORRIDO*/
		while(nodoAct!=i)
			{
			direccionAct=matMinimos[i][nodoAct].direcc;
			if ((direccionAct!=direccion)&&(cambio==-1))
				{
				cambio=nodoAct;
				}
			direccion=direccionAct;
			nodoAct=matMinimos[i][nodoAct].nodo;
			}

		/*CONDICIONES DE ESCRITURA*/
		if(cambio==-1)
			{
			/*NO SE CAMBIO LA RUTA*/
			fprintf(escritor,"0\n");
			}
		else
			{
			/*SE CAMBIO LA RUTA*/
			fprintf(escritor,"%d\n",cambio);
			}
		}
	}
/********************************************************************/
void inicializar()
	{

	int i;
	int j;
	/*INICIALIZAR*/
	for (i=0;i<=totCiudades;i++)
		{
		for (j=0;j<=totCiudades;j++)
			{
			matEnlaces[i][j]=-1;
			matMinimos[i][j].costo=1000000;
			matMinimos[i][j].nodo=-1;
			matMinimos[i][j].direcc=-1;
			matMinimos[i][j].final=0;
			}
		}
	}
/********************************************************************/
void imprime()
	{
	int i,j;
	for (i=1;i<=totCiudades;i++)
		{
		for (j=1;j<=totCiudades;j++)
			{
			printf("(%d ",matMinimos[i][j].costo);
			printf("%d ",matMinimos[i][j].nodo);
			printf("%d)",matMinimos[i][j].direcc);

			}
		printf("\n");
		}
	printf("==========================\n");
	}
