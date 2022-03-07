#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#define N 100

FILE *file;

int main(int argc, char *argv[])
	{
	int idProc;
	int totProc;
	int i,j,k,p,m,tmp;

	MPI_Status stat;

	//DIMENSION DE LA MATRIZ
	int f1;
	int c1;
	int f2;
	int c2;	

	int tag;
	int dest;
	int aux;
	int aux2;

	//MATRICES
	int A[N][N];
	int B[N][N];
	int C[N][N];

	//FILAS COLS
	int filaA[N];
	int colB[N];
	int result[N][N];

	//DIVISION DE TRABAJO
	int nAct;
	int n1;
	int n2;
	int ntrabaj;
	int trabaj[N];
	int numFil;

	int temp;
	int base;
	

	//INICIO MPI
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &idProc);
	MPI_Comm_size(MPI_COMM_WORLD, &totProc);

	if(idProc==0)
		{
		/*********************************************************************/
		//CASO PRINCIPAL
		//LECTURA DE ARCHIVO DE ENTRADA
		file=fopen("matriz.in","r");

		//MATRIZ 1
		fscanf(file,"%d",&f1);
		fscanf(file,"%d",&c1);

		for (i=0;i<f1;i++)
			{
			for (j=0;j<c1;j++)
				{
				fscanf(file,"%d",&tmp);
				A[i][j]=tmp;
				}
			}
		//MATRIZ 2
		fscanf(file,"%d",&f2);
		fscanf(file,"%d",&c2);
		
		for (i=0;i<f2;i++)
			{
			for (j=0;j<c2;j++)
				{
				fscanf(file,"%d",&tmp);
				B[i][j]=tmp;
				}
			}


		if (c1==f2)
			{
						
			//ES POSIBLE IMPRIMIR  
			printf("primera matriz\n");
			for(i=0;i<f1;i++)
				{
				for(j=0;j<c1;j++)
					{
					printf("%d ",A[i][j]);
					}
				printf("\n");
				}
			printf("\n");
			printf("segunda matriz\n");
			for(i=0;i<f2;i++)
				{
				for(j=0;j<c2;j++)
					{
					printf("%d ",B[i][j]);
					}
				printf("\n");
				}
			printf("\n");

			//DIVISION DE TRABAJO
			trabaj[0]=f1;			
			ntrabaj=1; 
			
			for(p=0;p<totProc-1;p++)
				{
				nAct=trabaj[0];
				n1=nAct/2;
				n2=nAct-n1;

				trabaj[0]=n1;
				trabaj[ntrabaj]=n2;
				
				ntrabaj=ntrabaj+1;	   

				//ordenar
				for(k=0;k<ntrabaj-1;k++)
					{
					for(m=k+1;m<ntrabaj;m++)
						{
						if(trabaj[k]<trabaj[m])
							{
							temp=trabaj[m];
							trabaj[m]=trabaj[k];
							trabaj[k]=temp;				
							}
						} 
					}
				}

	 
			//FIN DIVISION DE TRABAJO
			printf("*** parte procesador #%d ***\n",idProc);

			//DIFUNDIR DIMENSION MAT 1
			for (i=1;i<totProc;i++)
				{
				MPI_Send(&f1,1,MPI_INT,i,0,MPI_COMM_WORLD);
				MPI_Send(&c1,1,MPI_INT,i,0,MPI_COMM_WORLD);
				}

			//DIFUNDIR DIMENSION MAT 2
			for (i=1;i<totProc;i++)
				{
				MPI_Send(&f2,1,MPI_INT,i,0,MPI_COMM_WORLD);
				MPI_Send(&c2,1,MPI_INT,i,0,MPI_COMM_WORLD);
				}


			//DIFUNDIR DIVISION DE TRABAJO
			for(j=1;j<totProc;j++)
				{
				aux2=trabaj[j];
				MPI_Send(&aux2,1,MPI_INT,j,0,MPI_COMM_WORLD);
				}

			//DIFUNDIR BASE
			base=trabaj[0];
			for(j=1;j<totProc;j++)
				{
				MPI_Send(&base,1,MPI_INT,j,0,MPI_COMM_WORLD);
				base=base+trabaj[j];
				}

			//DIFUNDIR COLUMNAS DE B
			for(j=0;j<c2;j++)
				{
				for (i=0;i<f2;i++)
					{
					colB[i]=B[i][j];
					}
				for (i=1;i<totProc;i++)
					{
					MPI_Send(colB,N,MPI_INT,i,0,MPI_COMM_WORLD);
					}
				
				}
		  
			//DIFUNDIR FILAS DE A
			//USANDO DIVISION DE TRABAJO
			base=trabaj[0];
			for(i=1;i<totProc;i++)
				{
				aux2=trabaj[i];
				
				for(j=0;j<aux2;j++)
					{
					for(k=0;k<c1;k++)
						{
						filaA[k]=A[j+base][k];
						}					
					MPI_Send(filaA,N,MPI_INT,i,0,MPI_COMM_WORLD);	
					}
				base=base+trabaj[i];
				}

			//FILAS DE A PARA PRINCIPAL
			//CONSIDERAR matA
  

			//OPERAR
			for(i=0;i<trabaj[0];i++)
				{
				for(j=0;j<c2;j++)
					{
					result[i][j]=0;
					for (k=0;k<c1;k++)
						{
						result[i][j]=result[i][j]+A[i][k]*B[k][j];
						}
					}
				}
		  

			//TRASPASAR A MATC
			for (i=0;i<trabaj[0];i++)
				{
				for(j=0;j<c2;j++)
					{
					C[i][j]=result[i][j];
					}
				}
			printf("** procesador #%i, resultado de la multiplicacion de la fila # ",idProc);
			for(i=0;i<trabaj[0];i++)
				{
				printf("%i ",i);
				}
			printf(" **\n");

			for(i=0;i<trabaj[0];i++)
				{
				for (j=0;j<c2;j++)
					{
					printf("%d ",result[i][j]);
					}
				printf("\n");
				}

			//RECIBIR RESULTADOS
			base=trabaj[0];
			for(i=1;i<totProc;i++)
				{
				aux2=trabaj[i];

				for(j=0;j<aux2;j++)
					{
					MPI_Recv(filaA,N,MPI_INT,i,0,MPI_COMM_WORLD,&stat);
					for (k=0;k<c2;k++)
						{
						C[base+j][k]=filaA[k];
						}
					}
				base=base+trabaj[i];		 
				}


			printf("*** termina procesador #%d ***\n",idProc);
			//IMPRIMIR MULTIPLICACION
			printf("resultado:\n");
			for(i=0;i<f1;i++)
				{
				for(j=0;j<c2;j++)
					{
					printf("%d ",C[i][j]);
					}
				printf("\n");
				}
			
			}
		else
			{
			printf("no es posible multiplicar\n");
			}		

		}
	else
		{
		/**********************************************************************/
		//CASO CLIENTES
		printf("*** parte procesador #%d ***\n",idProc);

		//RECIBIR DIMENSION MAT 1
		MPI_Recv(&f1,1,MPI_INT,0,0,MPI_COMM_WORLD,&stat);
		MPI_Recv(&c1,1,MPI_INT,0,0,MPI_COMM_WORLD,&stat);

		//RECIBIR DIMENSION MAT 2
		MPI_Recv(&f2,1,MPI_INT,0,0,MPI_COMM_WORLD,&stat);
		MPI_Recv(&c2,1,MPI_INT,0,0,MPI_COMM_WORLD,&stat);

		//RECIBIR DIVISION DE TRABAJO
		MPI_Recv(&numFil,1,MPI_INT,0,0,MPI_COMM_WORLD,&stat);

		//RECIBIR BASE
		MPI_Recv(&base,1,MPI_INT,0,0,MPI_COMM_WORLD,&stat);
		
		if (c1==f2)
			{
			//RECIBIR COLUMNAS DE B
			for (j=0;j<c2;j++)
				{
				MPI_Recv(colB,N,MPI_INT,0,0,MPI_COMM_WORLD,&stat);
				for(i=0;i<f2;i++)
					{
					B[i][j]=colB[i];
					}
				}

			//RECIBIR FILAS DE A
			//FILAS A OPERAR EN MATRIZ A
			for(i=0;i<numFil;i++)
				{
				MPI_Recv(filaA,N,MPI_INT,0,0,MPI_COMM_WORLD,&stat);

				for(j=0;j<c1;j++)
					{
					A[i][j]=filaA[j];
					}
				}

		  //OPERAR
			for(i=0;i<numFil;i++)
				{
				for(j=0;j<c2;j++)
					{
					result[i][j]=0;
					for (k=0;k<c1;k++)
						{
						result[i][j]=result[i][j]+A[i][k]*B[k][j];
						}
					}
				}

			printf("** procesador #%i, resultado de la multiplicacion de la filas # ",idProc);
			for(i=0;i<numFil;i++)
				{
				printf("%i ",base+i);
				}
			printf(" **\n");
			
			for (i=0;i<numFil;i++)
				{
				for (j=0;j<c2;j++)
					{
					printf("%d ",result[i][j]);
					}
				printf("\n");
				}
		   
			//ENVIAR RESULTADOS
			for(i=0;i<numFil;i++)
				{
				for(j=0;j<c2;j++)
					{
					filaA[j]=result[i][j];
					}
				MPI_Send(filaA,N,MPI_INT,0,0,MPI_COMM_WORLD);
				}		
			}

		printf("*** termina procesador #%d ***\n",idProc);
		}
	MPI_Finalize();
	return(0);
	}
	
