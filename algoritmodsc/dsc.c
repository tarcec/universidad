#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

void dsc();
int procMinimizacion(int nTarea);
void nuevoCluster(int nTarea);
void ordenaXprioridad(int vector[]);
int numHijos(int nTarea);
void dsrw(int nTarea);
void gantt();
void ultimaTarea(int nCluster);
void bottomLevel();

int totalTareas;
int totalClusters;

float **comunicacion;
float **commOrig;
float *blevels;

typedef struct
	{
	int idTarea;
	float tmpTarea;
	float tLevelTarea;
	int clusterTarea;
	int prgTarea;
	int bloqCluster;
	int nFinal;
	}tipoTarea;

typedef struct
	{
	int idCluster;
	float tUltTarea;
	int totalTarAsig;
	int *tareaAsig;
	int bloqueado;
	}tipoCluster;

tipoTarea *tareas;
tipoCluster *clusters;


/*******************************************************/
int main()
	{
	/*LECTURA DE LOS DATOS*/

	FILE *ArchivoEnt;
	int i,j;
	int nHijos;
	int hijo;
	float costoCom;

	if ((ArchivoEnt=fopen("dag.txt","r+")) != NULL)
		{
		fscanf(ArchivoEnt,"%i",&totalTareas);

		/* SE ASIGNA MEMORIA A LAS ESTRUCTURAS Y MATRICES*/
		tareas=(tipoTarea *)calloc(totalTareas,sizeof(tipoTarea));
		clusters=(tipoCluster *)calloc(totalTareas,sizeof(tipoTarea));
		commOrig=(float **)calloc(totalTareas,sizeof(float *));
		comunicacion=(float **)calloc(totalTareas,sizeof(float *));
		for (i=0;i<totalTareas;i++)
			{
			commOrig[i]=(float *)calloc(totalTareas,sizeof(totalTareas));
			comunicacion[i]=(float *)calloc(totalTareas,sizeof(totalTareas));
			clusters[i].tareaAsig=(int *)calloc(totalTareas,sizeof(totalTareas));
			}

		/* SE INICIALIZAN LOS COSTOS DE COM */
		for (i=0;i<totalTareas;i++)
			{
			for (j=0;j<totalTareas;j++)
				{
				commOrig[i][j]=-1;
				}
			}

		/* LECTURA DE RELACIONES ENTRE NODOS DEL DAG */
		for (i=0;i<totalTareas;i++)
			{
			fscanf(ArchivoEnt,"%i",&tareas[i].idTarea);
			fscanf(ArchivoEnt,"%f",&tareas[i].tmpTarea);
			tareas[i].clusterTarea=-1;
			tareas[i].prgTarea=0;
			fscanf(ArchivoEnt,"%i",&nHijos);
			if (nHijos == 0)
				{
				tareas[i].nFinal=1;
				}
			else
				{
				tareas[i].nFinal=0;
				for (j=0;j<nHijos;j++)
					{
					fscanf(ArchivoEnt,"%i",&hijo);
					fscanf(ArchivoEnt,"%f",&costoCom);
					commOrig[i][hijo]=costoCom;
					}
				}
			clusters[i].tUltTarea=0;
			}

		/* SE COPIAN LAS COMUNICACIONES EN UNA MATRIZ, PARA MANTENER LAS ORIGINALES */
		for (i=0;i<totalTareas;i++)
			{
			for (j=0;j<totalTareas;j++)
				{
				comunicacion[i][j]=commOrig[i][j];
				}
			}
		totalClusters=0;
		fclose(ArchivoEnt);
		dsc();
		gantt();
		for (i=0;i<totalTareas;i++)
			{
			free(commOrig[i]);
			free(comunicacion[i]);
			}
		free(commOrig);
		free(comunicacion);
		free(tareas);	
		}
	return 0;
	}

/*********************************************************/
void bottomLevel()
	{
	
	float **Grafo;

	int i,j,k;
	float Maximo;

	Grafo=(float **)calloc(totalTareas,sizeof(float *));
	blevels=(float *)calloc(totalTareas,sizeof(float));
	for (i=0;i<totalTareas;i++)
		{
		Grafo[i]=(float *)calloc(totalTareas,sizeof(totalTareas));
		}

	/* INICIALIZO EL GRAFO CON COSTOS + TIEMPO TAREA */
	for (i=0;i<totalTareas;i++)
		{
		for (j=0;j<totalTareas;j++)
			{
			if (i==j)
				{
				Grafo[i][j]=0;
				}
			else if (comunicacion[i][j]==-1)
				{
				Grafo[i][j]=comunicacion[i][j];
				}
			else
				{
				Grafo[i][j]=comunicacion[i][j]+tareas[i].tmpTarea;
				if (tareas[j].nFinal ==1)
					{
					Grafo[i][j]=Grafo[i][j]+tareas[j].tmpTarea;	
					}				
				}
			}
		}

	/* APLICO FLOYD PARA CALCULAR TODOS LOS BLEVELS */
	for (k=0;k<totalTareas;k++)
		{
		for (i=0;i<totalTareas;i++)
			{
			for (j=0;j<totalTareas;j++)
				{
				if (j != i ) /* Se salta las diagonales */
					{
					/* Condicion para cambiar los valores de la matriz */	
					if ((Grafo[i][j] < Grafo[i][k] + Grafo[k][j]) && (Grafo[i][k] > 0) && (Grafo[k][j] > 0))
						{
						Grafo[i][j] = Grafo[i][k] + Grafo[k][j];
						}
					}
				}
			}
		}

	/* GUARDO LOS BLEVELS EN UN ARREGLO */
	for (i=0;i<totalTareas;i++)
		{
		if (tareas[i].nFinal==1)
			{
			blevels[i]=tareas[i].tmpTarea;
			}
		else
			{
			Maximo=Grafo[i][0];
			for (j=1;j<totalTareas;j++)
				{
				if (Maximo < Grafo[i][j])
					{
					Maximo = Grafo[i][j];
					blevels[i]=Maximo;
					}
				}
			}
		}	
	}		

/***********************************************************/

void dsc()
	{
	/*PRECONDICIONES PARA MATRIZ DE ADYACENCIA
	  =======================================

	(X,Y) --> La flecha va de X y termina en Y (apunta a Y)
	*/

	int i,j,ValidaLibre,ContLibre,ContPLibre,Conectado,Flag=0;
	int cantProgramados;
	float tLevelMaximo;

	int maxLibre;
	int maxPLibre;

	float prioLibre;
	float prioPLibre;

	int *NLibres;
	int *NPLibres;
	float *Tlevels;
	float *TPlevels;

	bottomLevel();
	
	maxLibre=0;
	prioLibre=blevels[0];	
	
	/*CANTIDAD DE PROGRAMADOS INICIAL IGUAL AL
	N§ DE NODOS*/
	cantProgramados=totalTareas;

	/* MEMORIA PARA NLIBRES Y NPLIBRES*/
	
	NLibres=(int *)calloc(totalTareas,sizeof(int));
	NPLibres=(int *)calloc(totalTareas,sizeof(int));
	Tlevels=(float *)calloc(totalTareas,sizeof(float));	
	TPlevels=(float *)calloc(totalTareas,sizeof(float));

	/* INICIA CICLO DE PARALELIZACION*/
	while (cantProgramados!=-1)
		{

		if (Flag==0)
			{
				
			/* CALCULO INICIAL DE NODO CON MAS PRIORIDAD */
			for (i=1;i<totalTareas;i++)
				{
				if (prioLibre < blevels[i])
					{
					prioLibre=blevels[i];
					maxLibre=i;
					}
				}
			maxPLibre=-1;
			prioPLibre=-1;
			Flag++;
			}
		else
			{			
			/* CALCULO DE NODOS LIBRE Y PARCIALMENTE LIBRES */
			ContLibre=0;
			ContPLibre=0;
					
			for (i=0;i<totalTareas;i++)
				{
				if (tareas[i].prgTarea!=1)
				{
				Conectado=0;
				ValidaLibre=0;
				tLevelMaximo=0;
				for (j=0;j<totalTareas;j++)
					{
					if ((comunicacion[j][i] != -1) && (tareas[i].prgTarea != 1))
						{
						Conectado++;
						if (tareas[j].prgTarea == 1)
							{
							if (tLevelMaximo < tareas[j].tLevelTarea + comunicacion[j][i] + tareas[j].tmpTarea)
								{
								tLevelMaximo = tareas[j].tLevelTarea + comunicacion[j][i] + tareas[j].tmpTarea;
								}
							ValidaLibre++;
							}
						}
					}
				if((Conectado==0))
					{
					NLibres[ContLibre]=i;
					Tlevels[ContLibre]=tLevelMaximo;
					ContLibre++;
					if (tareas[i].bloqCluster != -1)
						{
						clusters[tareas[i].bloqCluster].bloqueado=0;
						tareas[i].bloqCluster = -1;
						}
					}				
				if ((ValidaLibre==Conectado)&&Conectado!=0)
					{
					NLibres[ContLibre]=i;
					Tlevels[ContLibre]=tLevelMaximo;
					ContLibre++;
					if (tareas[i].bloqCluster != -1)
						{
						clusters[tareas[i].bloqCluster].bloqueado=0;
						tareas[i].bloqCluster = -1;
						}
					}
				else if ((ValidaLibre > 0 )&& (ValidaLibre < Conectado))
					{
					NPLibres[ContPLibre]=i;
					TPlevels[ContPLibre]=tLevelMaximo;
					ContPLibre++;
					}			
				}
			}

			/* CALCULO DE PRIORIDADES PARA LIBRES Y PARCIALES*/
			prioLibre=-1;
			prioPLibre=-1;
			maxLibre=-1;
			maxPLibre=-1;
			
			for (i=0;i<ContLibre;i++)
				{
				if (prioLibre < Tlevels[i] + blevels[NLibres[i]])
					{
					prioLibre = Tlevels[i] + blevels[NLibres[i]];
					maxLibre=NLibres[i];	
					}
				}	
			
			for (i=0;i<ContPLibre;i++)
				{
				if (prioPLibre < TPlevels[i] + blevels[NPLibres[i]])
					{
					prioPLibre = TPlevels[i] + blevels[NPLibres[i]];
					maxPLibre=NPLibres[i];	
					}
				}
			}	
		
		/*******************************************************************************/
		if (prioLibre>=prioPLibre)
			{
			/*LLAMAR AL PROCESO DE MINIMIZACION*/
			if(procMinimizacion(maxLibre)==0)
				{
				nuevoCluster(maxLibre);
				}
			}
		else
			{
			/*LLAMAR AL PROCESO DE MINIMIZACION
			BAJO DSRW*/
			dsrw(maxPLibre);
			if(procMinimizacion(maxLibre)==0)
				{
				nuevoCluster(maxLibre);
				}
			}
		cantProgramados=cantProgramados-1;	
		}
	free(NLibres);
	free(NPLibres);
	free(Tlevels);
	free(TPlevels);

	}
/*******************************************************/
int procMinimizacion(int nTarea)
	{
	int i,j,k,l;
	float tmp;
	int *listaPred;
	float *listaPrio;
	float *listaTLevel;
	//int *listaSched;
	int totTareas;
	int h;
	float suma;
	int ptoMinimo;
	int clusterAnt;
	int tareaAct;
	float maxTLevel;
	float tLevelAct;
	float tareaTLMax;
	int m;
	int L;

	j=0;
	
	/* ASIGNACION DE MEMORIA A ARREGLOS */
	listaPred=(int *)calloc(totalTareas,sizeof(int));
	listaPrio=(float *)calloc(totalTareas,sizeof(float));
	listaTLevel=(float *)calloc(totalTareas,sizeof(float));
	//listaSched=(int *)calloc(totalTareas,sizeof(int)); 	
		
	/*-------------------------------------------------------------------------------*/
	/****************************/
	/*		PASO N§1		  */
	/****************************/
	L=totalTareas-1;

	/*SORTING DE LAS PRIORIDADES*/

	for(i=0;i<=L;i++)
		{
		/*SE DEJA FIJA LA COLUMNA PARA SABER CUALES
		SON LOS ANTESESORES*/
		if (comunicacion[i][nTarea]!=-1)
			{
			/*ACA EXISTE UN ENLACE...LLENAR SOLAMENTE*/
			listaPred[j]=i;
			listaPrio[j]=tareas[i].tLevelTarea+tareas[i].tmpTarea+comunicacion[i][nTarea];
			j=j+1;
			}
		}
	j=j-1;

	/*AHORA ORDENARLOS...BUBBLESORT MEJORABLE!!!*/
	for(i=0;i<=j;i++)
		{
		for(k=i+1;k<=j;k++)
			{
			if(listaPrio[i]<listaPrio[k])
				{
				tmp=listaPrio[k];
				listaPrio[k]=listaPrio[i];
				listaPrio[i]=tmp;

				tmp=listaPred[k];
				listaPred[k]=listaPred[i];
				listaPred[i]=tmp;
				}
			}
		}

	/*VIENDO SI LA MINIMIZACION ES POSIBLE CON EL DE
	PRIORIDAD MAS ALTA*/

	/*FORMA DE CALCULARLO
	tUltTarea= tiempo en que termina la ultima tarea del cluster
	listaPrio[0]= 	es la suma del tiempo de inicio del predecesor mas el
			tiempo de ejecucion de la tarea y su costo de
			comunicacion.

			= tLevelTarea + tmpTarea + tmpComm*/

	/*CONDICION PARA LAS TAREAS INICIALES DEL GRAFO QUE VAN A SER
	PLANIFICADAS*/
	if (j==-1)
		{
		return(0);
		}

	/*CONDICION DE BLOCKEO DE DRSW*/
	if (clusters[tareas[listaPred[0]].clusterTarea].bloqueado==1)
		{
		return(0);
		}

	if (clusters[tareas[listaPred[0]].clusterTarea].tUltTarea<listaPrio[0])
		{
		/*VERDADERO :  SE PROCEDE CON LA MINIMIZACION*/

		/*-------------------------------------------------------------------------------*/
		/****************************/
		/*		PASO N§2		  */
		/****************************/

		/*BUSCAR H DE MANERA DE QUE NINGUN OTRO PREDECESOR
		TENGA OTRO HIJO ADEMAS DE nTarea*/

		h=0;
		for(k=1;k<=j;k++)
			{
			/*!!!!VERIFICAR ESTA CONDICION !!!!!*/

			if(tareas[listaPred[k]].clusterTarea==tareas[listaPred[0]].clusterTarea)
				{
				/*LA TAREA ESTA EN EL CLUSTER DE LA TAREA
				CON MAYOR PRIORIDAD*/
				h=h+1;
				}
			else
				{
				/*LA TAREA NO ESTA EN EL CLUSTER DE LA TAREA
				CON MAYOR PRIORIDAD*/
				if (masHijos(listaPred[k])==0)					{
					h=h+1;
					}
				else
					{
					break;
					}
				}
			}
			
		/*-------------------------------------------------------------------------------*/	
		/****************************/
		/*		PASO N§3		  */
		/****************************/
		/* HAY QUE BUSCAR EL PUNTO OPTIMO*/
		/* H SIEMPRE TERMINA CON EL N§ EXACTO DE PREDECESORES
		   QUE CUMPLEN CON LA CONDICION*/

		ptoMinimo=0;
		for(i=0;i<h;i++)
			{
			/*SUMATORIA*/
			suma=0;
			for (k=0;k<=i;k++)
				{
				suma=tareas[listaPred[i]].tmpTarea+suma;
				}

			/*APLICANDO EL CRITERIO*/
			if (suma<=comunicacion[listaPred[i+1]][nTarea])
				{
				ptoMinimo=ptoMinimo+1;
				}
			}
		/*PTOMINIMO SIEMPRE TERMINA CON UNO MAS*/
	
		/*-------------------------------------------------------------------------------*/
		/****************************/
		/*		PASO N§4		  */
		/****************************/
		/*UNIFICAR TODOS LOS CLUSTERES EN UNO SOLO*/

		/*UFFF!!!*/

		/*CASO j=0,1*/
		if ((ptoMinimo==0)||(j==0)||(j==1))
			{
			/*HACIENDO CERO EL TIEMPO DE COMUNICACION*/
			comunicacion[listaPred[0]][nTarea]=0;

			/*VIENDO TLEVEL MAXIMO DE LA TAREA*/
			maxTLevel=clusters[tareas[listaPred[0]].clusterTarea].tUltTarea;
			for(l=0;l<=L;l++)
				{
				if(comunicacion[l][nTarea]!=-1)
					{
					tLevelAct=tareas[l].tLevelTarea+tareas[l].tmpTarea+comunicacion[l][nTarea];
					if (tLevelAct>=maxTLevel)
						{
						maxTLevel=tLevelAct;
						}
					}
				}
			/*TLEVEL DE LA TAREA*/
			tareas[nTarea].tLevelTarea=maxTLevel;

			/*ANADIENDO LA NUEVA TAREA AL CLUSTER*/
			totTareas=clusters[tareas[listaPred[0]].clusterTarea].totalTarAsig;
			clusters[tareas[listaPred[0]].clusterTarea].tareaAsig[totTareas]=nTarea;
			clusters[tareas[listaPred[0]].clusterTarea].totalTarAsig=clusters[tareas[listaPred[0]].clusterTarea].totalTarAsig+1;

			/*ACTUALIZANDO INFO DE LA TAREA*/
			tareas[nTarea].clusterTarea=clusters[tareas[listaPred[0]].clusterTarea].idCluster;
			tareas[nTarea].prgTarea=1;
			tareas[nTarea].bloqCluster=-1;

			/*CALCULO DE ULTTAREA*/
			ultimaTarea(tareas[listaPred[0]].clusterTarea);

			return(1);
			}

		/*CASO j>0*/

		/*** ACTUALIZACION DEL CLUSTER DE LAS TAREAS ***/
		for (i=1;i<=ptoMinimo;i++)
			{
			clusterAnt=tareas[listaPred[i]].clusterTarea;
			tareaAct=listaPred[i];

			/*RESTAURACION DE TIEMPOS DE COMUNICACION*/
			/*BUSQUEDA DE LOS ANTESESORES*/
			maxTLevel=0;
			for(l=0;l<=L;l++)
				{
				/*SI FUE MINIMIZADO*/
				if (comunicacion[l][tareaAct]==0)
					{
					/*SI EL PREDECESOR DEL PREDECESOR COMPARTEN
					CLUSTERS*/
					if(tareas[l].clusterTarea==clusterAnt)
						{
						/*RESTAURAR COMM*/
						comunicacion[l][tareaAct]=commOrig[l][tareaAct];

						}
					}
				}

			/*ACTUALIZAR TULUTAREA*/

			totTareas=clusters[clusterAnt].totalTarAsig-1;
			clusters[clusterAnt].tUltTarea=clusters[clusterAnt].tUltTarea-tareas[tareaAct].tmpTarea;

			/*ELIMINACION DEL REGISTRO EN EL CLUSTER ANTERIOR*/
			clusters[clusterAnt].totalTarAsig=clusters[clusterAnt].totalTarAsig-1;
			}
		/*** PRIMERA ACTUALIZACION DE TLEVELS CON NODOS PREDECESORES*/
		for(i=0;i<=ptoMinimo;i++)
			{
			tareaAct=listaPred[i];
			if (i==0)
				{
				maxTLevel=tareas[listaPred[0]].tLevelTarea;
				}
			else
				{
				maxTLevel=0;
				}

			/*VER TODOS SUS ANTECESORES*/
			for(l=0;l<=L;l++)
				{
				if(comunicacion[l][tareaAct]!=-1)
					{
					tLevelAct=tareas[l].tLevelTarea+tareas[l].tmpTarea+comunicacion[l][tareaAct];
					if (tLevelAct>=maxTLevel)
						{
						maxTLevel=tLevelAct;
						}
					}
				}
			tareas[tareaAct].tLevelTarea=maxTLevel;
			}

		totTareas=clusters[tareas[listaPred[0]].clusterTarea].totalTarAsig;

		/*** HACER CERO LAS ARISTAS QUE CORRESPONDAN ***/
		for(i=0;i<=ptoMinimo;i++)
			{
			comunicacion[listaPred[i]][nTarea]=0;
			}

		/*** ORDENAMIENTO DE LOS PREDECESORES, SEGUN SU TLEVEL ***/
		totTareas=clusters[tareas[listaPred[0]].clusterTarea].totalTarAsig;

		/*LLENADO DE listaTLevel CON INFORMACION DEL CLUSTER*/
		for (k=0;k<totTareas;k++)
			{
			listaTLevel[k]=tareas[clusters[tareas[listaPred[0]].clusterTarea].tareaAsig[k]].tLevelTarea;
			}
		m=0;
		/*LLENADO DE listaTLevel CON INFORMACION DE LAS NUEVAS TAREAS
		Y DEL CLUSTER*/

		for (i=0;i<=ptoMinimo;i++)
			{
			if(tareas[listaPred[0]].clusterTarea!=tareas[listaPred[i]].clusterTarea)
				{
				listaTLevel[m+totTareas]=tareas[listaPred[i]].tLevelTarea;
				clusters[tareas[listaPred[0]].clusterTarea].tareaAsig[m+totTareas]=listaPred[i];
				m=m+1;
				}
			/*CAMBIO DE CLUSTER*/
			tareas[listaPred[i]].clusterTarea=tareas[listaPred[0]].clusterTarea;
			}
		/*MAXTLEVEL PARA ULTIMO NODO*/
		maxTLevel=0;
		for(l=0;l<=L;l++)
			{
			if(comunicacion[l][nTarea]!=-1)
				{
				tLevelAct=tareas[l].tLevelTarea+tareas[l].tmpTarea+comunicacion[l][nTarea];
				if(tLevelAct>=maxTLevel)
					{
					maxTLevel=tLevelAct;
					}
				}
			}
		tareas[nTarea].tLevelTarea=maxTLevel;
		listaTLevel[totTareas+m]=tareas[nTarea].tLevelTarea;

		clusters[tareas[listaPred[0]].clusterTarea].totalTarAsig=m+totTareas;
		totTareas=clusters[tareas[listaPred[0]].clusterTarea].totalTarAsig;

		/*INSERTAR EL nTAREA EN EL CLUSTER*/
		clusters[tareas[listaPred[0]].clusterTarea].tareaAsig[totTareas]=nTarea;
		totTareas=totTareas+1;
		clusters[tareas[listaPred[0]].clusterTarea].totalTarAsig=totTareas;

		/*BURBUJA NUEVAMENTE*/
		for(i=0;i<totTareas;i++)
			{
			for(k=i+1;k<totTareas;k++)
				{
				if(listaTLevel[i]>listaTLevel[k])
					{
					tmp=clusters[tareas[listaPred[0]].clusterTarea].tareaAsig[k];
					clusters[tareas[listaPred[0]].clusterTarea].tareaAsig[k]=clusters[tareas[listaPred[0]].clusterTarea].tareaAsig[i];
					clusters[tareas[listaPred[0]].clusterTarea].tareaAsig[i]=tmp;

					tmp=listaTLevel[k];
					listaTLevel[k]=listaTLevel[i];
					listaTLevel[i]=tmp;
					}
				}
			}

		/*** SEGUNDA ACTUALIZACION DE LOS TLEVELS*/
		ultimaTarea(tareas[listaPred[0]].clusterTarea);

		/*EN ESTE PUNTO SABEMOS EN QUE ORDEN VAN LAS TAREAS
		Y EN QUE MINUTO*/

		/*MANEJO DE LOS DATOS DE nTAREA*/
		tareas[nTarea].tLevelTarea=clusters[tareas[listaPred[0]].clusterTarea].tUltTarea-tareas[nTarea].tmpTarea;
		tareas[nTarea].clusterTarea=tareas[listaPred[0]].clusterTarea;
		tareas[nTarea].prgTarea=1;
		tareas[nTarea].bloqCluster=-1;

		/*LA TAREA SE AGREGO FINALMENTE A UN CLUSTER*/
		return(1);
		}
	else
		{
		/*FALSO : LA TAREA QUEDA EN UN CLUSTER UNITARIO*/
		return(0);
		}
	}
/******************************************************/
void nuevoCluster(int nTarea)
	{
	float maxTLevel;
	float tLevelAct;
	int l;

	/*LLENADO EN TAREAS*/
	tareas[nTarea].clusterTarea=totalClusters;

	/*BUSQUEDA DEL MAXIMO TLEVEL*/
	maxTLevel=0;
	for(l=0;l<=totalTareas-1;l++)
		{
		if(comunicacion[l][nTarea]!=-1)
			{
			tLevelAct=tareas[l].tLevelTarea+tareas[l].tmpTarea+comunicacion[l][nTarea];
			if (tLevelAct>=maxTLevel)
				{
				maxTLevel=tLevelAct;
				}
			}
		}

	tareas[nTarea].tLevelTarea=maxTLevel;
	tareas[nTarea].prgTarea=1;
	tareas[nTarea].bloqCluster=-1;

	/*LLENADO EN CLUSTERS*/
	clusters[totalClusters].idCluster=totalClusters;
	clusters[totalClusters].tUltTarea=tareas[nTarea].tLevelTarea+tareas[nTarea].tmpTarea;
	clusters[totalClusters].totalTarAsig=1;
	clusters[totalClusters].tareaAsig[0]=nTarea;
	clusters[totalClusters].bloqueado=0;

	totalClusters=totalClusters+1;
	}
/******************************************************/
int masHijos(int nTarea)
	{
	int i;
	int cont;

	cont=0;
	for (i=0;i<=totalTareas-1;i++)
		{
		if (comunicacion[nTarea][i]!=-1)
			{
			cont=cont+1;
			}
		if (cont>1)
			{
			return(1);
			}
		}
	return(0);
	}
/******************************************************/
void dsrw(int nTarea)
	{
	int i;
	int j;
	int k;
	int tmp;
	int *listaPred;
	float *listaPrio;

	listaPred=(int *)calloc(totalTareas,sizeof(int));
	listaPrio=(float *)calloc(totalTareas,sizeof(float));

	/*VERIFICAR CON EL PREDECESOR DE MAYOR PRIORIDAD*/

	/*SORTING DE LAS PRIORIDADES*/
	j=0;
	for(i=0;i<=totalTareas-1;i++)
		{
		/*SE DEJA FIJA LA COLUMNA PARA SABER CUALES
		SON LOS ANTESESORES*/
		if (comunicacion[i][nTarea]!=-1)
			{
			/*OTRA CONDICION DEBE ESTAR PROGRAMADO**/
			if(tareas[i].prgTarea==1)
				{
				/*ACA EXISTE UN ENLACE...LLENAR SOLAMENTE*/
				listaPred[j]=i;
				listaPrio[j]=tareas[i].tLevelTarea+tareas[i].tmpTarea+comunicacion[i][nTarea];
				j=j+1;
				}
			}
		}
	j=j-1;

	/*AHORA ORDENARLOS...BUBBLESORT MEJORABLE!!!*/
	for(i=0;i<=j;i++)
		{
		for(k=i+1;k<=j;k++)
			{
			if(listaPrio[i]<listaPrio[k])
				{
				tmp=listaPrio[k];
				listaPrio[k]=listaPrio[i];
				listaPrio[i]=tmp;

				tmp=listaPred[k];
				listaPred[k]=listaPred[i];
				listaPred[i]=tmp;
				}
			}
		}
	/*SE TIENE EL PRIMERO.... VER ANEXION*/
	if(clusters[tareas[listaPred[0]].clusterTarea].tUltTarea<listaPrio[0])
		{
		/*VERDADERO, SE DEBE RESTRINGIR EL CLUSTER*/
		clusters[tareas[listaPred[0]].clusterTarea].bloqueado=1;
		tareas[nTarea].bloqCluster=clusters[tareas[listaPred[0]].clusterTarea].idCluster;
		//return(1);
		}
	else
		{
		//return(0);
		}

	}
/******************************************************/
void gantt()
	{
	int i;
	int j;
	int k;

	printf("\nRESUMEN:");
	printf("\n=========");

	printf("\nTareas:");

	for(i=0;i<totalTareas;i++)
		{
		printf("\n---------------------------------");
		printf("\nID	 :%i",tareas[i].idTarea);
		printf("\nTIEMPO :%f",tareas[i].tmpTarea);
		printf("\nTLEVEL :%f",tareas[i].tLevelTarea);
		printf("\nCLUSTER:%i",tareas[i].clusterTarea);
		}
	printf("\n***********************************************************************");
	printf("\nClusters:");
	for(i=0;i<totalClusters-1;i++)
		{
		if(clusters[i].totalTarAsig!=0)
			{
			printf("\n---------------------------------");
			printf("\nCLUSTER :%i",clusters[i].idCluster);
			printf("\nTIEMPO\t\t\t\tTAREA");
			printf("\n");		
			for (j=0;j<clusters[i].totalTarAsig;j++)
				{
				printf("\n%f\t\t\t%i",tareas[clusters[i].tareaAsig[j]].tLevelTarea,tareas[clusters[i].tareaAsig[j]].idTarea);
				}
			}
		}
	printf("\n---------------------------------\n");
		}
/******************************************************/
void ultimaTarea(int nCluster)
	{
	int i;
	float suma;
	float tLevelAct;
	float tEjecuion;
	float tInit;

	suma=0;

	/*VIENDO ULTTAREA DE LOS PREDECESORES*/

	tInit=tareas[clusters[nCluster].tareaAsig[0]].tLevelTarea;

	/*CALCULANDO ULTTAREA*/
	suma=tInit+tareas[clusters[nCluster].tareaAsig[0]].tmpTarea;
	for (i=1;i<clusters[nCluster].totalTarAsig;i++)
		{
		if(suma<=tareas[clusters[nCluster].tareaAsig[i]].tLevelTarea)
			{
			suma=tareas[clusters[nCluster].tareaAsig[i]].tLevelTarea+tareas[clusters[nCluster].tareaAsig[i]].tmpTarea;
			}
		else
			{
			tareas[clusters[nCluster].tareaAsig[i]].tLevelTarea=suma;
			suma=suma+tareas[clusters[nCluster].tareaAsig[i]].tmpTarea;
			}
		}
	clusters[nCluster].tUltTarea=suma;
	}
/******************************************************/


