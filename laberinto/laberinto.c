#include <stdio.h>
#include <stdlib.h>
#include <time.h>
//#include <unistd.h>
#include <string.h>

int **grafo;
int **nums;
int dimGrafo;

typedef struct
	{
	int fila;
	int columna;
	}stack;

void laberinto(int **A,int f,int c) 
	{
	int i,j,a;

	//se cambia por el anterior... no funcionaba en ciertos casos
	//siempre tiene que ir antes de generar numeros aleatoreos
	srand(time(0));

	for(i=0;i<f;i++)
		{
		for(j=0;j<c;j++)
			{
			//genera 0 o 1
			a=rand()%2;
			A[i][j]=a;
			}
		}
	A[0][0]=1;
	A[f-1][c-1]=1;
	}
void generaGrafo(int **A,int f,int c)
	{
	int i;
	int j;
	int sum=0;
	int vert;
	int vert2;

	nums=calloc(f,sizeof(int *));

	for(i=0;i<f;i++)
		{
		nums[i]=calloc(c,sizeof(int ));
		}

	//matriz de ids
	for(i=0;i<f;i++)
		{
		for(j=0;j<c;j++)
			{
			if(A[i][j]==1)
				{
				nums[i][j]=sum;
				sum=sum+1;
				}
			else
				{
				nums[i][j]=-1;
				}
			}
		}

	dimGrafo=sum;

	grafo=calloc(dimGrafo+1,sizeof(int *));

	for(i=0;i<dimGrafo;i++)
		{
		grafo[i]=calloc(dimGrafo+1,sizeof(int ));
		}

	//llenar grafo
	//sum
	for(i=0;i<dimGrafo;i++)
		{
		for(j=0;j<dimGrafo;j++)
			{
			grafo[i][j]=0;
			}
		}

	//relaciones entre los vertices
	for(i=0;i<f;i++)
		{
		for(j=0;j<c;j++)
			{
			if(A[i][j]==1)
				{
				vert=nums[i][j];
				//arriba
				if(i-1>=0)
					{
					if(A[i-1][j]==1)
						{
						vert2=nums[i-1][j];
						grafo[vert][vert2]=1;
						}
					}

				//abajo
				if(i+1<f)
					{
					if(A[i+1][j]==1)
						{
						vert2=nums[i+1][j];
						grafo[vert][vert2]=1;
						}
					}

				//derecha
				if(j+1<c)
					{
					if(A[i][j+1]==1)
						{
						vert2=nums[i][j+1];
						grafo[vert][vert2]=1;
						}
					}

				//izquierda
				 if(j-1>=0)
					{
					if(A[i][j-1]==1)
						{
						vert2=nums[i][j-1];
						grafo[vert][vert2]=1;
						}
					}
				}
			}
		}
	}   

void imprime(int **A,int f,int c)
	{
	int i,j;

	for(i=0;i<f;i++)
		{
		for(j=0;j<c;j++)
			{
			//imprime
			printf("%d ",A[i][j]);
			}
		printf("\n");
		}
	}

void leer(int **A,int f,int c)
	{
	int i,j;

	int v;
	for(i=0;i<f;i++)
		{
		for(j=0;j<c;j++)
			{
			scanf("%d",&v);
			A[i][j]=v;
			}
		}
	}

void muestra(int **A,int f,int c)
	{
	int sum=0;
	int i,j;	
	
   //imprime laberinto
	printf("**** laberinto generado: ****\n");

	for(i=0;i<f;i++)
		{
		for(j=0;j<c;j++)
			{
			printf("%d ",A[i][j]);
			}
		printf("\n");
		}

	printf("\n");
	
	printf("**** matriz de adyacencia: *****\n");
	
	for(i=0;i<dimGrafo;i++)
		{
		for(j=0;j<dimGrafo;j++)
			{
			printf("%d ",grafo[i][j]);
			}	
		printf("\n");
		}
	printf("\n");

	printf("**** datos del grafo: ****\n\n");
	printf("nodo\tvecinos\n");
	printf("------------------------------------------------------------\n");

	for(i=0;i<dimGrafo;i++)
		{
		printf("%d\t",i);
		for(j=0;j<dimGrafo;j++)
			{
			if(grafo[i][j]==1)
				{
				printf("%d ",j);
				}
			}	
		printf("\n");
		}
	printf("\n");
	}

void resuelve(int **mat,int f,int c,int y,int z)
	{
	stack *stk;
	int **vis;

	int nstk;	

	int i;
	int j;
	int num;
	int fa;
	int ca;
	int fs;
	int not;
	int sig;
	
	int cant;
	int *val;

	int v1,v2;
  
	nstk=0;
	
	not=0;

	//pide memoria
	stk=(stack *)calloc(f*c,sizeof(stack));
	
	//pedimos memoria fila
	vis=calloc(f,sizeof(int *));
	val=calloc(f*c,sizeof(int *));

	//pedimos memoria col
	for(i=0;i<f;i++)
		{
		vis[i]=calloc(c,sizeof(int ));
		}

	//llena visitado
	for(i=0;i<f;i++)
		{
		//val[i]=0;
		for(j=0;j<c;j++)
			{
			vis[i][j]=0;
			}
		}

	printf("\n");
	printf("+++ solucion +++\n");

	cant=1;

	if(mat[0][0]==0)
		{
		//NO TIENE SOLUCION
		not=1;
		}
	else
		{
		stk[0].fila=y;
		stk[0].columna=z;
		vis[y][z]=1;

		//partiendo de 1,1
		//Indicador de solucion
		sig=1;
		while(sig==1)
			{
			//Idicador que debemos retroceder en una posicion en el stack
			fs=0;

			//ver posibilidades en pos
			fa=stk[nstk].fila;
			ca=stk[nstk].columna;

			val[nstk]=1;

			if( (fa==f-1)&&(ca==c-1) )
				{
				//hay solucion
				printf("HAY SOLUCION!!!\n");
				for(i=0;i<=nstk;i++)
					{
					if(val[i]==1)
						{
						printf("(%d,%d)\n",stk[i].fila,stk[i].columna);
						}
					}
					printf("\n");
					printf("\n");

					//recorrido en el grafo
					printf("Recorrido en el grafo\n");
					for(i=0;i<=nstk;i++)
						{
						if(val[i]==1)
							{
										v1=stk[i].fila;
										v2=stk[i].columna;
							printf("%d ",nums[v1][v2]);
							}
						}
				printf("\n");

				sig=0;
				}
			else
				{
				//revision de movimiento
				//arriba
				if(fa-1>=0)
					{
					//existe una posibilidad arriba
					if(mat[fa-1][ca]==1)
						{
						if(vis[fa-1][ca]==0)
							{
							//a¤adir al stack
							nstk=nstk+1;
							stk[nstk].fila=fa-1;
							stk[nstk].columna=ca;
							val[nstk]=0;
							vis[fa-1][ca]=1;

							fs=1;
							}
						}
					}
				//abajo
				if(fa+1<f)
					{
					//existe posibilidad abajo
					if(mat[fa+1][ca]==1)
						{
						if (vis[fa+1][ca]==0)
							{
							//a¤adir al stack
							nstk=nstk+1;
							stk[nstk].fila=fa+1;
							stk[nstk].columna=ca;
							val[nstk]=0;
							vis[fa+1][ca]=1;
							fs=1;
							}
						}
					}
				//derecha
				if(ca+1<c)
					{
					//existe posibilidad derecha
					if(mat[fa][ca+1]==1)
						{
						if (vis[fa][ca+1]==0)
							{
							//a¤adir al stack
							nstk=nstk+1;
							stk[nstk].fila=fa;
							stk[nstk].columna=ca+1;
							val[nstk]=0;
							vis[fa][ca+1]=1;
							fs=1;
							}
						}
					}
				//izquierda
				if(ca-1>=0)
					{
					//existe posibilidad izquierda
					if(mat[fa][ca-1]==1)
						{
						if (vis[fa][ca-1]==0)
							{
							//a¤adir al stack
							nstk=nstk+1;
							stk[nstk].fila=fa;
							stk[nstk].columna=ca-1;
							val[nstk]=0;
							vis[fa][ca-1]=1;
							fs=1;
							}
						}
					}
				//si fs=0 el caso no tiene solucion
				if(fs==0)
					{					
					val[nstk]=0;		
				
					nstk=nstk-1;										
					//si vaciamos el stack... no hay solucion
					if (nstk<0)
						{
						//no sol
						not=1;
						sig=0;
						}
					}	
				}
			}
		}
	if(not==1)
		{
		printf("sin solucion!!!\n");
		}
	
	}

int main(int argc,char **argv)
	{
 	int f,c,z,y,**A,i,j;
	char *o;
	char *bar;
	int flg;

	if(argc==4)
		{
		f=atoi(argv[1]);
		c=atoi(argv[2]);
		o=argv[3];

		//crea la matriz dinamica
		A=calloc(f,sizeof(int *));

		for(i=0;i<f;i++)
			{
			A[i]=calloc(c,sizeof(int ));
			}

		if(strcmp(o,"-G")==0)
			{
			//genera matriz
			laberinto(A,f,c);
			//imprime
			imprime(A,f,c);
			}
		else if(strcmp(o,"-R")==0)
			{
			//lee matriz y resuelve
			leer(A,f,c);
			generaGrafo(A,f,c);
			muestra(A,f,c);

			flg=0;

			while(1)
				{
				//fila de partida
				scanf("%d",&y);
				
				//columna de partida
				scanf("%d",&z);	
				
				printf("\nfila de partida :%d",y);
		
				printf("\ncolumna de partida:%d",z);				

				if ( (A[y][z]==1) || (y>=0) || (z>=0) || (y<f) || (z<c) )
					{
					resuelve(A,f,c,y,z);
					return(0);
					}
				else
					{
					printf("*** ingrese una posicion valida ***\n");
					}
				}
			}
		else if(strcmp(o,"-E")==0)
			{
			//genera matriz y resuelve
			laberinto(A,f,c);
			generaGrafo(A,f,c);
			muestra(A,f,c);

			while(1)
				{
				printf("\ningrese la fila de partida:");
				scanf("%d",&y);
				printf("\ningrese la columna de partida:");
				scanf("%d",&z);

			   if(A[y][z]==1)
					{
					resuelve(A,f,c,y,z);
					return(0);
					}
				else
					{
					printf("*** ingrese una posicion valida ***\n");
					}
				}			
			}
		else
			{
			printf("parametros insuficientes");
			}
		}
	else
		{
		printf("parametros insuficientes");
		}
	return(0);
	}
