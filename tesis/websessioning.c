#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>
#include <ctype.h>
#define N 50

//ESTRUCTURA SITIO
typedef struct
	{
	int id;
	char nombrePag[80];
	}
pagina;
pagina *listaPaginas;
int lenListaPaginas;
int **grafoPaginas;

//ESTRUCTURA ESTADISTICAS
typedef struct
	{
	int *cantSesiones;
	float *logCantSesiones;
	float *logSesiones;
	float *logCantSesionesEst;
	float promX;
	float promY;
	float sumX;
	float sumY;
	float sumXY;
	float sumX2;
	float sumY2;
	float powsumX;
	float powsumY;
	float sumError2;
	float m;
	float b;
	float desError;
	float error;
	float r;
	float sxx;
	float sxy;
	float syy;
	}
estadistica;
estadistica staSesion;
int lenStaSesion;

//ESTRUCTURA REGISTRO
typedef struct
	{
	int timeStamp;
	char ip[14];
	int id;
	char nombrePag[80];
	}
registro;
registro *webLog;
int lenWebLog;

//ESTRUCTURA SOLUCION
typedef struct
	{
	int **sol;
	int *lenSesion;
	int *listaSesionesActivas;
	int lenListaSesionesActivas;
	float fitness;
	estadistica staSesion;
	int lenStaSesion;
	}
solucion;

//ESTRUCTURA POBLACION
typedef struct	
	{
	solucion *conjuntoSoluciones;
	int lenConjuntoSoluciones;
	}
poblacion;
poblacion poblacionActual; //AG
int lenConjuntoSoluciones;
solucion solucionActual; //SA

//EL TAMAÑO DEBIERA SER IGUAL AL NUMERO DE REGISTROS EN EL WEB LOG
int mtp;
int maxLenSesion;
int idFo;

char staSesFName[20];
char initFName[20];
char sesionFName[20];
char performanceFName[20];
char foFName[20];

FILE *archivoStaSes;
FILE *archivoSesion;
FILE *archivoPerformance;
FILE *archivoFo;
FILE *archivoInit;

int counterSesiones;
double sumTiempo;
float sumaFO;

//DEFINICION DE FUNCIONES
int leeChunk(int indx);
void leeEstructura();
int retornaIdPagina(char nombrePag[]);
int existeEnlace(int id1,int id2);
solucion newSolucion();
estadistica newEstadistica();
void freeSolucion(solucion sol);
void freeEstadistica(estadistica staSesion);
void llenarListaRegistros(int listaRegistros[]);
void HeuristicaTiempo(int indxChunk);
void SimulatedAnnealing(float tempIni,float tempFin,float factEnfriamiento,int maxIter,int numCicloInterno,int indxChunk);
void SimulatedAnnealingB(float tempIni,float tempFin,float factEnfriamiento,float fnumCicloInterno,int indxChunk,int indxIntentos,long int seed);
void imprimeSesion(solucion sol,int indxChunk);
float funcionObjetivo(solucion sol);
float foMaximo(solucion sol);
estadistica regresionSesion(estadistica staSesion,int lenStaSesion);
solucion generaSolucionInicial(solucion sol);
solucion copiarSolucion(solucion origen,solucion retorno);
int estaEnSesionModificada(int indxSesion,int listaSesionesModificadas[],int lenListaSesionesModificadas);
solucion generaVecino(solucion solOrig,solucion sol,int *ptrFlgOpt);
solucion actualizarStaSesion(solucion sol);
void totalizarStaSesion(solucion sol);
solucion reasignaRegistro(solucion sol,int registroEscogido,int *ptrSes);
solucion eliminarSesion(solucion sol,int indxSesion);
int estaRepetido(solucion sol,int indxRegistro,int listaSesionesModificadas[],int lenListaSesionesModificadas);
int operEstaDescartada(int operDescartadas[],int lenOperDescartadas,int idOperacion);
int *initOperDescartadas(int operDescartadas[]);
int buscarSesionVacia(solucion sol);
int fit(solucion sol,int registro,int *ptrSes);
void estadisticasSA(solucion sol,int indxChunk);

/************************************************************************************************************/
int main(int argc, char** argv)
    {
    int chrOpc;
    int i;
    int indxChunk;
    time_t semilla;
    int maxIter;
    int intentos;
    long int seed;
    int indxIntentos;
    float fmaxIter;
    float tempIni;
    float tempFin;
    float factEnfriamiento;
    int numCicloInterno;
    float fnumCicloInterno;

    system("clear");

    printf("\n");
    printf("SESIONIZACION:\n");
    printf("==============\n");
    printf("\n");

    //LEE ESTRUCTURA DEL SITIO
    leeEstructura();
    indxChunk=0;
    counterSesiones=0;
	//INICIALIZANDO ESTRUCTURA DE
            
   

    while((chrOpc=getopt(argc,argv,"hsb"))!=-1)
        {
        switch(chrOpc)
            {
            case 'h':
                //********** HEURISTICA DE TIEMPO **********
                 if(argc-optind<1)
                    {
                    printf("*** FALTAN ARGUMENTOS PARA EJECUTAR HEURISTICA DE TIEMPO (mtp) ***\n");
                    printf("\a\a");
                    exit(0);
                    }
                else
                    {
                    //LECTURA DE PARAMETROS
                    mtp=atoi(argv[optind++]);

                    printf(" - Parámetros de la Heurística de Tiempo:\n\n");
                    printf("\tMTP\t\t\t=\t%d\n\n",mtp);

                    strcpy(staSesFName,"staSesHE.out");
                    strcpy(sesionFName,"sesionHE.out");
                    strcpy(performanceFName,"performanceHE.out");

                    maxLenSesion=-1;
                    indxChunk=1;
                    sumTiempo=0.0;
                    if(leeChunk(indxChunk))
                        {
                        //MAXLENSESION SERA LEIDO EN LOS CHUNKS
                        staSesion=newEstadistica();
                        lenStaSesion=maxLenSesion + 1;
                        HeuristicaTiempo(indxChunk);
                        
                        free(webLog);
                        staSesion=regresionSesion(staSesion,lenStaSesion);
                        
                         //TAMAÑO SESION vs NUMERO SESIONES
                        archivoStaSes=fopen(staSesFName,"w");
                        for(i=1;i<lenStaSesion;i++)
                            fprintf(archivoStaSes,"%d,%d,%.8f,%.8f\n",i,staSesion.cantSesiones[i],staSesion.logSesiones[i],staSesion.logCantSesiones[i]);

                        fprintf(archivoStaSes,"m,%.8f\n",staSesion.m);
                        fprintf(archivoStaSes,"b,%.8f\n",staSesion.b);
                        fprintf(archivoStaSes,"r2,%.8f\n",staSesion.r);
                        fprintf(archivoStaSes,"error,%.8f\n",staSesion.error);
                        fclose(archivoStaSes);

                        freeEstadistica(staSesion);
                        printf("\t\t****** TIEMPO TOTAL = %.8f s *****\n\n",sumTiempo);
                        printf("\t\t********** PROCESO FINALIZADO **********\n\n");
                        printf("\a\a\a");
                        }
                    else
                        {
                        printf("*** WEBLOG (chunk1.in) NO ENCONTRADO ***\n");
                        printf("\a\a");
                        exit(0);
                        }
                    }
                break;
            case 's':
                //********** SIMULATED ANNEALING **********
               if(argc-optind<7)
                    {
                    printf("*** FALTAN ARGUMENTOS PARA EJECUTAR SA (tempIni,tempFin,Kte.factEnfr,Kte.numCiclo,mtp,maxLenSesion,idFo,intentos) ***\n");
                    printf("\a\a");
                    exit(0);
                    }
                else
                    {
					//LECTURA DE PARAMETROS
                    tempIni=atof(argv[optind++]);
                    tempFin=atof(argv[optind++]);
                    factEnfriamiento=atof(argv[optind++]);
                    fnumCicloInterno=atof(argv[optind++]);
                    mtp=atoi(argv[optind++]);
                    maxLenSesion=atoi(argv[optind++]);
                    idFo=atoi(argv[optind++]);
                    intentos=atoi(argv[optind++]);
                    printf(" - Parámetros de Simulated Annealing:\n\n");
                    printf("\tKte. Temp.Inicial\t=\t%.8f\n",tempIni);
                    printf("\tTemp.Final\t\t=\t%.8f\n",tempFin);
                    printf("\tFact.Enfriamiento\t=\t%.8f\n",factEnfriamiento);
                    printf("\tKte. Cicl. Internos\t=\t%.2f\n",fnumCicloInterno);
                    printf("\tMTP\t\t\t=\t%d\n",mtp);
                    printf("\tTam.Max.Sesion\t\t=\t%d\n",maxLenSesion);
                    printf("\tId.Func.Obj\t\t=\t%d\n",idFo);
                    printf("\tMax.Inentos\t\t=\t%d\n\n",intentos);

                    char sstaSesFName[20];
                    char ssesionFName[20];
                    char sperformanceFName[20];
                    char sfoFName[20];
                    char sinitFName[20];

                    strcpy(sinitFName,"initSA.out");
                    strcpy(sstaSesFName,"staSesSA.out");
                    strcpy(ssesionFName,"sesionSA.out");
                    strcpy(sperformanceFName,"performanceSA.out");
                    strcpy(sfoFName,"foSA");

                    sprintf(performanceFName,"%s",sperformanceFName);
                    archivoPerformance=fopen(performanceFName,"a");
                    fprintf(archivoPerformance,"INTENTO,CHUNK,SEMILLA,TIEMPO,ITER,ITER-A-MAX,RANGO-ITER,T°INI,T°FIN,T°OPTIMO,FOINI,FOSOLMEJOR,FOINC,R,E\n");
                    fclose(archivoPerformance);
                    
                    //APLICAR HEURISTICA
                    for(indxIntentos=1;indxIntentos<=intentos;indxIntentos++)
                        {
                         //TIEMPO
                        semilla=time(0);
                        seed=semilla;
                        srand(seed);
                       
                        staSesion=newEstadistica();
                        lenStaSesion=maxLenSesion + 1;

                        sprintf(initFName,"%d_%s",indxIntentos,sinitFName);
                        sprintf(staSesFName,"%d_%s",indxIntentos,sstaSesFName);
                        sprintf(sesionFName,"%d_%s",indxIntentos,ssesionFName);
                        sprintf(foFName,"%d_%s",indxIntentos,sfoFName);

                        //ENCABEZADO
                        indxChunk=1;
                        sumaFO=0.0;
                        sumTiempo=0.0;
                        while(leeChunk(indxChunk))
                            {
                            SimulatedAnnealingB(tempIni,tempFin,factEnfriamiento,fnumCicloInterno,indxChunk,indxIntentos,seed);

                             //CREAR ARCHIVO DE SALIDA
                            indxChunk=indxChunk + 1;
                            free(webLog);
                            }
                        //CALIDAD DE LA SESIONIZACION
                        staSesion=regresionSesion(staSesion,lenStaSesion);

                        //TAMAÑO SESION vs NUMERO SESIONES
                        archivoStaSes=fopen(staSesFName,"w");
                        for(i=1;i<lenStaSesion;i++)
                            fprintf(archivoStaSes,"%d,%d,%.8f,%.8f\n",i,staSesion.cantSesiones[i],staSesion.logSesiones[i],staSesion.logCantSesiones[i]);

                        fprintf(archivoStaSes,"m,%.8f\n",staSesion.m);
                        fprintf(archivoStaSes,"b,%.8f\n",staSesion.b);
                        fprintf(archivoStaSes,"r2,%.8f\n",staSesion.r);
                        fprintf(archivoStaSes,"error,%.8f\n",staSesion.error);
                        fprintf(archivoStaSes,"sumaFO,%.8f\n",sumaFO);
                        fprintf(archivoStaSes,"sumTiempo,%.8f\n",sumTiempo);
                        fclose(archivoStaSes);

                        freeEstadistica(staSesion);
                        printf("\t\t****** TIEMPO TOTAL = %.8f s *****\n\n",sumTiempo);
                        printf("\t\t********** PROCESO FINALIZADO **********\n\n");
                       }
                    }
                break;
             case 'b':
                //********** SIMULATED ANNEALING B **********
                if(argc-optind<9)
                    {
                    printf("*** FALTAN ARGUMENTOS PARA EJECUTAR SA-B (tempIni,tempFin,Kte.factEnfr,Kte.numCiclo,mtp,maxLenSesion,idFo,intentos,seed) ***\n");
                    printf("\a\a");
                    exit(0);
                    }
                else
                    {
                    //LECTURA DE PARAMETROS
                    tempIni=atof(argv[optind++]);
                    tempFin=atof(argv[optind++]);
                    factEnfriamiento=atof(argv[optind++]);
                    fnumCicloInterno=atof(argv[optind++]);
                    //fmaxIter=atof(argv[optind++]);
                    mtp=atoi(argv[optind++]);
                    maxLenSesion=atoi(argv[optind++]);
                    idFo=atoi(argv[optind++]);
                    intentos=atoi(argv[optind++]);
                    seed=atoi(argv[optind++]);
                    printf(" - Parámetros de Simulated Annealing-B:\n\n");
                    printf("\tKte. Temp.Inicial\t=\t%.8f\n",tempIni);
                    printf("\tTemp.Final\t\t=\t%.8f\n",tempFin);
                    printf("\tFact.Enfriamiento\t=\t%.8f\n",factEnfriamiento);
                    printf("\tKte. Cicl. Internos\t=\t%.2f\n",fnumCicloInterno);
                    printf("\tMTP\t\t\t=\t%d\n",mtp);
                    printf("\tTam.Max.Sesion\t\t=\t%d\n",maxLenSesion);
                    printf("\tId.Func.Obj\t\t=\t%d\n",idFo);
                    printf("\tMax.Inentos\t\t=\t%d\n",intentos);
                    printf("\tSemilla\t\t=\t%ld\n\n",seed);

                    //POR CONSOLA
                    srand(seed);

                    char sstaSesFName[20];
                    char ssesionFName[20];
                    char sperformanceFName[20];
                    char sfoFName[20];
                    char sinitFName[20];

                    strcpy(sinitFName,"initSA.out");
                    strcpy(sstaSesFName,"staSesSA.out");
                    strcpy(ssesionFName,"sesionSA.out");
                    strcpy(sperformanceFName,"performanceSA.out");
                    strcpy(sfoFName,"foSA");

                    sprintf(performanceFName,"%s",sperformanceFName);
                    archivoPerformance=fopen(performanceFName,"a");
                    fprintf(archivoPerformance,"INTENTO,CHUNK,SEMILLA,TIEMPO,ITER,ITER-A-MAX,RANGO-ITER,T°INI,T°FIN,T°OPTIMO,FOINI,FOSOLMEJOR,FOINC,R,E\n");
                    fclose(archivoPerformance);

                    //APLICAR HEURISTICA
                    for(indxIntentos=1;indxIntentos<=intentos;indxIntentos++)
                        {
                        staSesion=newEstadistica();
                        lenStaSesion=maxLenSesion + 1;

                        sprintf(initFName,"%d_%s",indxIntentos,sinitFName);
                        sprintf(staSesFName,"%d_%s",indxIntentos,sstaSesFName);
                        sprintf(sesionFName,"%d_%s",indxIntentos,ssesionFName);
                        sprintf(foFName,"%d_%s",indxIntentos,sfoFName);

                        //ENCABEZADO
                        indxChunk=1;
                        sumaFO=0.0;
                        sumTiempo=0.0;
                        while(leeChunk(indxChunk))
                            {
                            SimulatedAnnealingB(tempIni,tempFin,factEnfriamiento,fnumCicloInterno,indxChunk,indxIntentos,seed);
                            
                            //CREAR ARCHIVO DE SALIDA
                            indxChunk=indxChunk + 1;
                            free(webLog);
                            }
                        //CALIDAD DE LA SESIONIZACION
                        staSesion=regresionSesion(staSesion,lenStaSesion);

                        //TAMAÑO SESION vs NUMERO SESIONES
                        archivoStaSes=fopen(staSesFName,"w");
                        for(i=1;i<lenStaSesion;i++)
                            fprintf(archivoStaSes,"%d,%d,%.8f,%.8f\n",i,staSesion.cantSesiones[i],staSesion.logSesiones[i],staSesion.logCantSesiones[i]);

                        fprintf(archivoStaSes,"m,%.8f\n",staSesion.m);
                        fprintf(archivoStaSes,"b,%.8f\n",staSesion.b);
                        fprintf(archivoStaSes,"r2,%.8f\n",staSesion.r);
                        fprintf(archivoStaSes,"error,%.8f\n",staSesion.error);
                        fprintf(archivoStaSes,"sumaFO,%.8f\n",sumaFO);
                        fprintf(archivoStaSes,"sumTiempo,%.8f\n",sumTiempo);
                        fclose(archivoStaSes);

                        freeEstadistica(staSesion);
                        printf("\t\t****** TIEMPO TOTAL = %.8f s *****\n\n",sumTiempo);
                        printf("\t\t********** PROCESO FINALIZADO **********\n\n");
                        printf("\a\a\a");
                       }
                    }
                break;           
            }
        }

    //BORRADO DE ESTRUCTURAS
    free(listaPaginas);
    for(i=0;i<lenListaPaginas;i++)
        free(grafoPaginas[i]);
    free(grafoPaginas);
    return(0);
    }

/************************************************************************************************************/
void leeEstructura()
    {
    int i,j;
    int a,b;
    int lenPath;
    char *path="paths.in";
    char *website="website.in";
    FILE *archivoEntrada;

    printf(" - Leyendo Listado de Páginas (%s)",website);
    archivoEntrada=fopen(website,"r");
    if(archivoEntrada!=NULL)
        {
        //LECTURA DEL LISTADO DE PAGINAS
        fscanf(archivoEntrada,"%d",&lenListaPaginas);
        listaPaginas=(pagina *)calloc(lenListaPaginas,sizeof(pagina));
        grafoPaginas=(int **)calloc(lenListaPaginas,sizeof(int *));

        for(i=0;i<lenListaPaginas;i++)
            {
            fscanf(archivoEntrada,"%d %s",&listaPaginas[i].id,listaPaginas[i].nombrePag);
            grafoPaginas[i]=(int *)calloc(lenListaPaginas,sizeof(int));
            }
        fclose(archivoEntrada);
        printf("\t\t\t\tOK!\n");
        //INICIALIZAR GRAFO
        for(i=0;i<lenListaPaginas;i++)
            for(j=0;j<lenListaPaginas;j++)
                grafoPaginas[i][j]=0;

        //LECTURA DE LOS PATHS
        printf(" - Leyendo Estructura del Sitio (%s)",path);
        archivoEntrada=fopen(path,"r");
        if(archivoEntrada!=NULL)
            {
            fscanf(archivoEntrada,"%d",&lenPath);
            for(i=0;i<lenPath;i++)
                {
                fscanf(archivoEntrada,"%d %d",&a,&b);
                grafoPaginas[a][b]=1;
                }
            fclose(archivoEntrada);
            printf("\t\t\t\tOK!\n");
            }
        else
            {
             printf("\n*** ERROR AL LEER %s. ARCHIVO NO EXISTE ***\n",path);
             exit(0);
            }
        }
    else
        {
        printf("\n*** ERROR AL LEER %s. ARCHIVO NO EXISTE ***\n",website);
        exit(0);
        }
    
    }
/************************************************************************************************************/
int leeChunk(int index)
	{
	FILE *archivoEntrada;

	int i;
	int totReg;	
	char nombreChunk[80];
	char strTmp[80];
	
	strcpy(strTmp,"chunk");
	sprintf(nombreChunk,"%s%d.in",strTmp,index);
	archivoEntrada=fopen(nombreChunk,"r+");
	if(archivoEntrada!=NULL)
		{
		printf(" - Leyendo %s",nombreChunk);
		//IR PIDIENDO MEMORIA
		fscanf(archivoEntrada,"%d",&totReg);
		webLog=(registro*)calloc(totReg,sizeof(registro));	

		for(i=0;i<totReg;i++)
			{
			fscanf(archivoEntrada,"%s %d %s %d",webLog[i].ip,&webLog[i].id,webLog[i].nombrePag,&webLog[i].timeStamp);
			}
		lenWebLog=totReg;
		if(maxLenSesion==-1)
			{
			fscanf(archivoEntrada,"%d",&maxLenSesion);
			if(maxLenSesion==0)
				maxLenSesion=lenWebLog;
			}
		fclose(archivoEntrada);
		printf("\t\t\t\t\t\t\tOK!\n");
		return 1;
		}
	else
		{
		return 0;
		}
	}
/************************************************************************************************************/
int retornaIdPagina(char nombrePag[])
	{
	int i;
	for(i=0;i<lenListaPaginas;i++)
		if(strcmp(nombrePag,listaPaginas[i].nombrePag)==1)
			break;
	
	return listaPaginas[i].id;
	}
/************************************************************************************************************/
int existeEnlace(int id1,int id2)
	{
	//EXISTE UN ENLACE ENTRE PAGS?
	if(grafoPaginas[id1][id2]==1)
		return 1;
	else
		return 0;
	}
/************************************************************************************************************/
int mismaIp(char ip1[],char ip2[])
    {
    //TIENEN MISMA IP
    if(strcmp(ip1,ip2)==0)
        return 1;
    else
        return 0;
    }
/************************************************************************************************************/
int enMtp(int tmp1,int tmp2)
    {
    //DENTRO MTP
    if((tmp2-tmp1<=mtp)&&(0<=tmp2-tmp1))
        return 1;
    else
        return 0;
    }
/************************************************************************************************************/
solucion newSolucion()
    {
    solucion nvaSolucion;
    int **sol;
    int *lenSesion;
    int *listaSesionesActivas;
    int i,j;

    sol=(int **)calloc(maxLenSesion,sizeof(int *));
    lenSesion=(int *)calloc(lenWebLog,sizeof(int));
    listaSesionesActivas=(int *)calloc(lenWebLog,sizeof(int));

    for(i=0;i<maxLenSesion;i++)
        sol[i]=(int *)calloc(lenWebLog,sizeof(int));
    
    nvaSolucion.sol=sol;
    nvaSolucion.lenSesion=lenSesion;
    nvaSolucion.listaSesionesActivas=listaSesionesActivas;
    nvaSolucion.lenListaSesionesActivas=0;
    nvaSolucion.fitness=-1;

    nvaSolucion.staSesion=newEstadistica();
    nvaSolucion.lenStaSesion=maxLenSesion+1;
   
    return nvaSolucion;
    }
/************************************************************************************************************/
estadistica newEstadistica()
    {
    estadistica staSesion;
    int i;

    staSesion.cantSesiones=(int *)calloc(maxLenSesion+1,sizeof(int));
    staSesion.logCantSesiones=(float *)calloc(maxLenSesion+1,sizeof(float));
    staSesion.logCantSesionesEst=(float *)calloc(maxLenSesion+1,sizeof(float));
    staSesion.logSesiones=(float *)calloc(maxLenSesion+1,sizeof(float));
    lenStaSesion=maxLenSesion+1;

    for(i=0;i<lenStaSesion;i++)
        {
        staSesion.cantSesiones[i]=0;
        staSesion.logCantSesiones[i]=0;
        staSesion.logCantSesionesEst[i]=0;
        if(i!=0)
            staSesion.logSesiones[i]=log(i);
        }
    staSesion.powsumX=0;
    staSesion.powsumY=0;
    staSesion.promX=0;
    staSesion.promY=0;
    staSesion.sumX=0;
    staSesion.sumY=0;
    staSesion.sumX2=0;
    staSesion.sumY2=0;
    staSesion.sumXY=0;
    staSesion.sumError2=0;
    staSesion.m=0;
    staSesion.b=0;
    staSesion.desError=0;
    staSesion.error=0;
    staSesion.r=0;
    staSesion.sxx=0;
    staSesion.sxy=0;
    staSesion.syy=0;

    return staSesion;
    }

/************************************************************************************************************/
void freeSolucion(solucion sol)
    {
    int i;
    free(sol.lenSesion);
    free(sol.listaSesionesActivas);
    for(i=0;i<maxLenSesion;i++)
        free(sol.sol[i]);
    free(sol.sol);
    freeEstadistica(sol.staSesion);
    }
/************************************************************************************************************/
void freeEstadistica(estadistica staSesion)
    {
    free(staSesion.cantSesiones);
    free(staSesion.logCantSesiones);
    free(staSesion.logCantSesionesEst);
    free(staSesion.logSesiones);
    }
/************************************************************************************************************/
void llenarListaRegistros(int listaRegistros[])
    {
    int i;
    for(i=0;i<lenWebLog;i++)
        listaRegistros[i]=i;
    }

/************************************************************************************************************/
void HeuristicaTiempo(int indxChunk)
    {
    solucion sol;
    int contLenSesion;
    int i;
    int flagPrimerReg;
    int indxSesion;
    int countPage;
    int idReg;
    int lenListaSesionesActivas;
    int lenSesion;
    struct timeval inicioMicro,finMicro;
    double tiempo;
    char strChunk[20];
    
    sol=newSolucion();
    contLenSesion=0;

    //APLICAR CRITERIO ENLACE PAGINA
    sol.lenListaSesionesActivas=0;
    flagPrimerReg=1;
    indxSesion=0;
    countPage=0;
    strcpy(strChunk,"chunk");
    sprintf(strChunk,"%s%d",strChunk,indxChunk);

    printf(" - Aplicando Heurística de Tiempo:\n\n");
    gettimeofday(&inicioMicro,NULL);
    for(i=0;i<lenWebLog;i++)
        {
        if(flagPrimerReg==1)
            {
            //PRIMER REGISTRO DE LA NUEVA SESION
            sol.sol[countPage][indxSesion]=i;
            sol.lenSesion[indxSesion]=1;

            flagPrimerReg=0;
            countPage++;
            }
        else
            {
            if ( (mismaIp(webLog[i-1].ip,webLog[i].ip))
                 && (enMtp(webLog[i-1].timeStamp,webLog[i].timeStamp)) )
                {
                //AGREGANDO A SESION EXISTENTE
                sol.sol[countPage][indxSesion]=i;
                sol.lenSesion[indxSesion]++;

                countPage++;
                }
           else
                {
                //NUEVA SESION
                lenListaSesionesActivas=sol.lenListaSesionesActivas;
                sol.listaSesionesActivas[lenListaSesionesActivas]=indxSesion;
                sol.lenListaSesionesActivas++;

				lenSesion=sol.lenSesion[indxSesion];
                countPage=0;
                indxSesion++;
                flagPrimerReg=1;
                i=i-1;
                }
            }
	
		}
		
    //CASO ULTIMO REGISTRO
	if(flagPrimerReg==0)
		{
		lenListaSesionesActivas=sol.lenListaSesionesActivas;
		sol.listaSesionesActivas[lenListaSesionesActivas]=indxSesion;
		sol.lenListaSesionesActivas++;

		lenSesion=sol.lenSesion[indxSesion];
		}
    
    sol=actualizarStaSesion(sol);
    gettimeofday(&finMicro,NULL);
    tiempo=(double)(finMicro.tv_sec-inicioMicro.tv_sec)+0.000001*(finMicro.tv_usec-inicioMicro.tv_usec);
    printf("\n  Tiempo de Resolución %s = %.8f s",strChunk,tiempo);
    printf("\t\t\t\tOK!\n\n",strChunk,tiempo);
    sumTiempo=sumTiempo + tiempo;

    archivoPerformance=fopen(performanceFName,"a");
    fprintf(archivoPerformance,"%s,%.8f",strChunk,tiempo);
    for(i=1;i<sol.lenStaSesion;i++)
        fprintf(archivoPerformance,",%d",sol.staSesion.cantSesiones[i]);

    fprintf(archivoPerformance,"\n");
    fclose(archivoPerformance);

    totalizarStaSesion(sol);

    //GENERAR SESION
    estadisticasSA(sol,indxChunk);

    freeSolucion(sol);
    }
/************************************************************************************************************/
void SimulatedAnnealing(float tempIni,float tempFin,float factEnfriamiento,int numCicloInterno,int maxIter,int indxChunk)
    {
    float tempAct;
    int flgOpt;
    int i,j;
    int countIter;
    float random;
    float delta;
    float boltsman;
    solucion sol;
    solucion solVecina;
    solucion solMejor;
    float foSol;
    float foSolVecina;
    float foSolMejor;
    float foMax;
    struct timeval inicioMicro,finMicro;
    double tiempo;
    char strChunk[20];
    char strFile[20];

    strcpy(strChunk,"chunk");
    srand(time(0));

    sol=newSolucion();
    solMejor=newSolucion();
    solVecina=newSolucion();

    sol=generaSolucionInicial(sol);
    solMejor=copiarSolucion(sol,solMejor);

    foSolMejor=funcionObjetivo(solMejor);
    tempAct=tempIni;
    
    sprintf(strFile,"%s%d.out",foFName,indxChunk);
    sprintf(strChunk,"%s%d",strChunk,indxChunk);
    archivoFo=fopen(strFile,"w");
    fprintf(archivoFo,"CAMB,CHUNK,T°ACT,C.I.,E,F.VEC,F.SOL,F.MEJ,R,ERR\n",strChunk,tempAct,i,boltsman,foSolVecina,foSol,foSolMejor,sol.staSesion.r,sol.staSesion.desError);

    //ftime(&inicio);
    gettimeofday(&inicioMicro,NULL);
    printf(" - Aplicando Simulated Annealing:\n\n");

    foMax=(float)foMaximo(sol);
    tempAct=foMax;//averiguar criterio

    countIter=0;

    while(tempAct>tempFin)
        {
        //IGUAL AL TAMAÑO DEL VECINDARIO
        numCicloInterno=solMejor.lenListaSesionesActivas / 2;
        maxIter=3*solMejor.lenListaSesionesActivas;
        for(i=0;i<numCicloInterno;i++)
            {
            solVecina=generaVecino(sol,solVecina,&flgOpt);
            foSol=funcionObjetivo(sol);
            foSolVecina=funcionObjetivo(solVecina);
            delta=(float)foSolVecina-foSol;

            foMax=(float)foMaximo(sol);
            boltsman=exp((float)delta/((float)(foMax*tempAct)));//SE MODIFICO
            
            if ((delta==0.0)|| ((delta>-0.001) && (delta<0.001)))
				{
				//***** MISMO VALOR FO, PERO DISTINTA COMBINACION *****
				//INFORMACION SOBRE ITERACIONES EN ARCHIVO
				fprintf(archivoFo,"d=,%s,%.8f,%d,%.8f,%.8f,%.8f,%.8f,%.8f,%.8f",strChunk,tempAct,i,boltsman,foSolVecina,foSol,foSolMejor,sol.staSesion.r,sol.staSesion.desError);
				for(j=1;j<sol.lenStaSesion;j++)
					fprintf(archivoFo,",%d",sol.staSesion.cantSesiones[j]);
				fprintf(archivoFo,"\n");

				countIter++;
				}
            else
				{
				if(delta>0)
					{
					countIter++;
					//***** ES MEJOR *****
					sol=copiarSolucion(solVecina,sol);
					if(foSolVecina>foSolMejor)
						{
						//***** EL MEJOR DE TODOS *****
						solMejor=copiarSolucion(solVecina,solMejor);
						foSolMejor=foSolVecina;
						countIter=0;
						}
					//INFORMACION SOBRE ITERACIONES EN ARCHIVO
					fprintf(archivoFo,"+,%s,%.8f,%d,%.8f,%.8f,%.8f,%.8f,%.8f,%.8f",strChunk,tempAct,i,boltsman,foSolVecina,foSol,foSolMejor,sol.staSesion.r,sol.staSesion.desError);
					for(j=1;j<sol.lenStaSesion;j++)
						fprintf(archivoFo,",%d",sol.staSesion.cantSesiones[j]);
					fprintf(archivoFo,"\n");

				   }
				else
					{
					//***** ELIGIENDO A UNO PEOR *****
					countIter++;
					random=rand()/(float)RAND_MAX;
					if(random<boltsman)
						{
						//***** SE ELIGIO A UNO PEOR *****
						sol=copiarSolucion(solVecina,sol);

						//INFORMACION SOBRE ITERACIONES EN ARCHIVO
						fprintf(archivoFo,"-,%s,%.8f,%d,%.8f,%.8f,%.8f,%.8f,%.8f,%.8f",strChunk,tempAct,i,boltsman,foSolVecina,foSol,foSolMejor,sol.staSesion.r,sol.staSesion.desError);
						for(j=1;j<sol.lenStaSesion;j++)
							fprintf(archivoFo,",%d",sol.staSesion.cantSesiones[j]);

						fprintf(archivoFo,"\n");

						}
					else
						{
						//***** NO FUE ELEGIDO EL PEOR *****
						//INFORMACION SOBRE ITERACIONES EN ARCHIVO
						fprintf(archivoFo,"=,%s,%.8f,%d,%.8f,%.8f,%.8f,%.8f,%.8f,%.8f",strChunk,tempAct,i,boltsman,foSolVecina,foSol,foSolMejor,sol.staSesion.r,sol.staSesion.desError);
						for(j=1;j<sol.lenStaSesion;j++)
							fprintf(archivoFo,",%d",sol.staSesion.cantSesiones[j]);
						fprintf(archivoFo,"\n");

						}
					}
				}
            }
        tempAct=factEnfriamiento*tempAct;
        if(countIter>maxIter)
            {
            printf("\n - Condicion de Término Anticipado Alcanzada para %s\n",strChunk);
            //MAXIMO NUMERO DE ITERACIONES SIN MEJORA
            break;
            }
        }
    gettimeofday(&finMicro,NULL);
    fclose(archivoFo);

    tiempo=(double)(finMicro.tv_sec-inicioMicro.tv_sec)+0.000001*(finMicro.tv_usec-inicioMicro.tv_usec);
    printf("\n  Tiempo de Resolución %s = %.8f s",strChunk,tiempo);
    printf("\t\t\t\tOK!\n\n",strChunk,tiempo);
    sumTiempo=sumTiempo + tiempo;

    //TIEMPO EN ENCONTRAR SOLUCION
    archivoPerformance=fopen(performanceFName,"a");
    fprintf(archivoPerformance,"%s,%.8f,%.8f,%.8f,%.8f",strChunk,tiempo,foSolMejor,solMejor.staSesion.r,solMejor.staSesion.desError);
    for(i=1;i<solMejor.lenStaSesion;i++)
        fprintf(archivoPerformance,",%d",solMejor.staSesion.cantSesiones[i]);
    fprintf(archivoPerformance,"\n");
    fclose(archivoPerformance);

    sumaFO=(float)sumaFO + foSolMejor;
    
    totalizarStaSesion(solMejor);

    //GENERAR SESION*
    estadisticasSA(solMejor,indxChunk);
    
    freeSolucion(sol);
    freeSolucion(solMejor);
    freeSolucion(solVecina);
    }
/************************************************************************************************************/
void SimulatedAnnealingB(float tempIni,float tempFin,float factEnfriamiento,float fkteCicloInterno,int indxChunk,int indxIntentos,long int seed)
    {
    float tempAct;
    int flgOpt;
    int i,j;
    int countIter;
    int numCicloInterno;
    int maxIter;
    int iteraciones;
    int iterFoMax;
    int rangoIteraciones;
    float random;
    float delta;
    float boltsman;
    float tempCamb;
    solucion sol;
    solucion solVecina;
    solucion solMejor;
    float foIni;
    float foSol;
    float foSolVecina;
    float foSolMejor;
    float foMax;
    struct timeval inicioMicro,finMicro;
    time_t semilla;
    double tiempo;
    char strChunk[20];
    char strFile[20];

    strcpy(strChunk,"chunk");
        

    sol=newSolucion();
    solMejor=newSolucion();
    solVecina=newSolucion();

    sol=generaSolucionInicial(sol);
    solMejor=copiarSolucion(sol,solMejor);

    foSolMejor=funcionObjetivo(solMejor);
    foIni=foSolMejor;

    sprintf(strFile,"%s%d.out",foFName,indxChunk);
    sprintf(strChunk,"%s%d",strChunk,indxChunk);
    archivoFo=fopen(strFile,"w");
    fprintf(archivoFo,"CAMB,CHUNK,T°ACT,C.I.,E,F.VEC,F.SOL,F.MEJ,R,ERR\n");

    //ESTADISTICAS DE LA SOLUCION INICIAL
    fprintf(archivoFo,"1=,%s,%.8f,%d,%.8f,%.8f,%.8f,%.8f,%.8f,%.8f",strChunk,tempAct,i,boltsman,foSolMejor,foSolMejor,foSolMejor,sol.staSesion.r,sol.staSesion.desError);
    for(j=1;j<sol.lenStaSesion;j++)
        fprintf(archivoFo,",%d",sol.staSesion.cantSesiones[j]);
    fprintf(archivoFo,"\n");

    //PERFORMANCE DE LA SOLUCION INICIAL
    archivoInit=fopen(initFName,"a");
    fprintf(archivoInit,"%s,%.8f,%.8f,%.8f",strChunk,foSolMejor,solMejor.staSesion.r,solMejor.staSesion.desError);
    for(i=1;i<solMejor.lenStaSesion;i++)
        fprintf(archivoInit,",%d",solMejor.staSesion.cantSesiones[i]);
    fprintf(archivoInit,"\n");
    fclose(archivoInit);

    //ftime(&inicio);
    gettimeofday(&inicioMicro,NULL);
    printf(" - Aplicando Simulated Annealing:\n\n");

    foMax=(float)foMaximo(sol);
    tempIni=(float)foMax/tempIni;//averiguar criterio
    tempAct=tempIni;
    countIter=0;
    iteraciones=0;
    iterFoMax;
    rangoIteraciones=0;

    while(tempAct>tempFin)
        {
        //IGUAL AL TAMAÑO DEL VECINDARIO
        numCicloInterno=fkteCicloInterno* solMejor.lenListaSesionesActivas;
        for(i=0;i<numCicloInterno;i++)
            {
            solVecina=generaVecino(sol,solVecina,&flgOpt);
            foSol=funcionObjetivo(sol);
            foSolVecina=funcionObjetivo(solVecina);
            delta=(float)foSolVecina-foSol;

            boltsman=exp((float)delta/((float)(tempAct)));

            if ((delta==0.0)|| ((delta>-0.001) && (delta<0.001)))
				{
				//***** MISMO VALOR FO, PERO DISTINTA COMBINACION *****
				//INFORMACION SOBRE ITERACIONES EN ARCHIVO
				fprintf(archivoFo,"d=,%s,%.8f,%d,%.8f,%.8f,%.8f,%.8f,%.8f,%.8f",strChunk,tempAct,i,boltsman,foSolVecina,foSol,foSolMejor,sol.staSesion.r,sol.staSesion.desError);
				for(j=1;j<solVecina.lenStaSesion;j++)
					fprintf(archivoFo,",%d",solVecina.staSesion.cantSesiones[j]);
				for(j=1;j<sol.lenStaSesion;j++)
					fprintf(archivoFo,",%d",sol.staSesion.cantSesiones[j]);
				fprintf(archivoFo,"\n");

				countIter++;
				}
            else
				{
				if(delta>0)
					{
					//FORZAR DESENSO DE TEMPERATURA
					i=numCicloInterno;

					countIter=0;
					if(foSolVecina>foSolMejor)
						{
						//***** EL MEJOR DE TODOS *****
						solMejor=copiarSolucion(solVecina,solMejor);
						foSolMejor=foSolVecina;
						tempCamb=tempAct;
						if(iteraciones-iterFoMax>rangoIteraciones)
							rangoIteraciones=iteraciones-iterFoMax;

						iterFoMax=iteraciones;
						}
					//INFORMACION SOBRE ITERACIONES EN ARCHIVO
					fprintf(archivoFo,"+,%s,%.8f,%d,%.8f,%.8f,%.8f,%.8f,%.8f,%.8f",strChunk,tempAct,i,boltsman,foSolVecina,foSol,foSolMejor,sol.staSesion.r,sol.staSesion.desError);
					for(j=1;j<solVecina.lenStaSesion;j++)
						fprintf(archivoFo,",%d",solVecina.staSesion.cantSesiones[j]);
					for(j=1;j<sol.lenStaSesion;j++)
						fprintf(archivoFo,",%d",sol.staSesion.cantSesiones[j]);
					fprintf(archivoFo,"\n");

					//***** ES MEJOR *****
					sol=copiarSolucion(solVecina,sol);
				   }
				else
					{
					//***** ELIGIENDO A UNO PEOR *****
					random=(float) rand()/(float)RAND_MAX;
					if(random<boltsman)
						{
						//***** SE ELIGIO A UNO PEOR *****
						countIter=0;
					   
						//INFORMACION SOBRE ITERACIONES EN ARCHIVO
						fprintf(archivoFo,"-,%s,%.8f,%d,%.8f,%.8f,%.8f,%.8f,%.8f,%.8f",strChunk,tempAct,i,boltsman,foSolVecina,foSol,foSolMejor,sol.staSesion.r,sol.staSesion.desError);
						for(j=1;j<solVecina.lenStaSesion;j++)
							fprintf(archivoFo,",%d",solVecina.staSesion.cantSesiones[j]);
						for(j=1;j<sol.lenStaSesion;j++)
							fprintf(archivoFo,",%d",sol.staSesion.cantSesiones[j]);

						fprintf(archivoFo,"\n");

						sol=copiarSolucion(solVecina,sol);
						}
					else
						{
						//***** NO FUE ELEGIDO EL PEOR *****
						countIter++;
						//INFORMACION SOBRE ITERACIONES EN ARCHIVO
						fprintf(archivoFo,"=,%s,%.8f,%d,%.8f,%.8f,%.8f,%.8f,%.8f,%.8f",strChunk,tempAct,i,boltsman,foSolVecina,foSol,foSolMejor,sol.staSesion.r,sol.staSesion.desError);
						for(j=1;j<solVecina.lenStaSesion;j++)
							fprintf(archivoFo,",%d",solVecina.staSesion.cantSesiones[j]);
						for(j=1;j<sol.lenStaSesion;j++)
							fprintf(archivoFo,",%d",sol.staSesion.cantSesiones[j]);
						fprintf(archivoFo,"\n");
						}
					}
				}
				iteraciones++;
            }
        tempAct=factEnfriamiento*tempAct;
        }
    //ftime(&fin);
    gettimeofday(&finMicro,NULL);
    fclose(archivoFo);

    tiempo=(double)(finMicro.tv_sec-inicioMicro.tv_sec)+0.000001*(finMicro.tv_usec-inicioMicro.tv_usec);
    printf("\n  Tiempo de Resolución %s = %.8f s",strChunk,tiempo);
    printf("\t\t\t\tOK!\n\n",strChunk,tiempo);
    sumTiempo=sumTiempo + tiempo;

    //TIEMPO EN ENCONTRAR SOLUCION
    archivoPerformance=fopen(performanceFName,"a");
    fprintf(archivoPerformance,"%d,%s,%ld,%.8f,%d,%d,%d,%.8f,%.8f,%.8f,%.8f,%.8f,%.8f,%.8f,%.8f",indxIntentos,strChunk,seed,tiempo,iteraciones,iterFoMax,rangoIteraciones,tempIni,tempFin,tempCamb,foIni,foSolMejor,foSolMejor-foIni,solMejor.staSesion.r,solMejor.staSesion.desError);
 
    for(i=1;i<solMejor.lenStaSesion;i++)
        fprintf(archivoPerformance,",%d",solMejor.staSesion.cantSesiones[i]);
    fprintf(archivoPerformance,"\n");
    fclose(archivoPerformance);

    sumaFO=(float)sumaFO + foSolMejor;
    totalizarStaSesion(solMejor);

    //GENERAR SESION*
    estadisticasSA(solMejor,indxChunk);

    freeSolucion(sol);
    freeSolucion(solMejor);
    freeSolucion(solVecina);
    }
/************************************************************************************************************/
void imprimeSesion(solucion sol,int indxChunk)
    {
    int indx;
    int cont;
    int j;
    int i;
    FILE *archivoSalida;
    char strChunk[80];

    sprintf(strChunk,"salida%d.out",indxChunk);
    archivoSalida=fopen(strChunk,"a");

    cont=0;
    for(i=0;i<lenWebLog;i++)
        {
        if(sol.lenSesion[i]!=0)
            {
            fprintf(archivoSalida,"%d,%d:",i,sol.lenSesion[i]);
            cont++;
            for(j=0;j<sol.lenSesion[i];j++)
                {
                fprintf(archivoSalida,"%d ",sol.sol[j][i]);
                }
            fprintf(archivoSalida,"\n");
            }
        }
    fprintf(archivoSalida,"==========\n");
    for(i=0;i<sol.lenStaSesion;i++)
        fprintf(archivoSalida,"%d %d\n",i,sol.staSesion.cantSesiones[i]);
    fprintf(archivoSalida,"****************************************\n");
    fclose(archivoSalida);
    }
/************************************************************************************************************/
void estadisticasSA(solucion sol,int indxChunk)
	{
	int i,j;
	int lenListaSesionesActivas;
	int lenSesion;
	int indxSesion;
	int contSesion;
	int indxRegistro;
	char ip[255]="1.0.0.0";
	int timeStamp;
	int idPag;
	char nombrePag[80]="html";
	char strChunk[20]="chunk";

	sprintf(strChunk,"chunk%d",indxChunk);

	archivoSesion=fopen(sesionFName,"a");
	fprintf(archivoSesion,"CONT.SESION,CHUNK,POS.SESION,INDX.REGISTRO,TIMESTAMP,IP,ID.PAG,NOM.PAG.\n",contSesion,strChunk,i+1,indxRegistro,timeStamp,ip,idPag,nombrePag);
	lenListaSesionesActivas=sol.lenListaSesionesActivas;
	for(j=0;j<lenListaSesionesActivas;j++)
		{
		contSesion=j+counterSesiones;
		indxSesion=sol.listaSesionesActivas[j];
		lenSesion=sol.lenSesion[indxSesion];

		for(i=0;i<lenSesion;i++)
			{
			indxRegistro=sol.sol[i][indxSesion];
			timeStamp=webLog[indxRegistro].timeStamp;
			strcpy(ip,webLog[indxRegistro].ip);
			idPag=webLog[indxRegistro].id;
			strcpy(nombrePag,webLog[indxRegistro].nombrePag);			

			fprintf(archivoSesion,"%d,%s,%d,%d,%d,%s,%d,%s\n",contSesion,strChunk,i+1,indxRegistro,timeStamp,ip,idPag,nombrePag);
			}
		}
	fclose(archivoSesion);
	counterSesiones=counterSesiones+lenListaSesionesActivas;
	}	
/************************************************************************************************************/
float funcionObjetivo(solucion sol)
    {
    int i;
    int j;
    int indxSesion;
    int lenSesion;
    float CO;
    
    //ES POSIBLE USANDO LENSENSION
    if( (idFo<=5) &&(idFo>=1))
        {
        CO=0.00000000;
        //CASO FUNCIONES
        for(i=0;i<sol.lenListaSesionesActivas;i++)
            {
            indxSesion=sol.listaSesionesActivas[i];
            lenSesion=sol.lenSesion[indxSesion];
            for(j=1;j<=lenSesion;j++)
                {
                 switch(idFo)
                    {
                    case 1:
                        //1/sqrt(o)
                        CO=CO+((float)1/(float)sqrt(j));
                        break;
                    case 2:
                        //log(o)
                        CO=(float)((float)CO+(float)log(j));
                        break;
                    case 3:
                        //(3/2)*log(o)+(o-3)^2/12*o
                        CO=CO+((float)3/2)*log(j)+((float)((float)pow(j-3,2)/(12*j)));
                        break;
                    case 4:
                        //o
                        CO=CO+j;
                        break;
                    case 5:
                        //o^2
                        CO=CO+(float)pow(j,2);
                        break;
                    }
                }
            }
        }
    else
        {
        //CASO 6, REGRESION
        sol.staSesion=regresionSesion(sol.staSesion,sol.lenStaSesion);

        CO=(1-sol.staSesion.r)+sol.staSesion.desError;
        }
    return CO;
    }
/************************************************************************************************************/
float foMaximo(solucion sol)
    {
    int i;
    int j;
    int indxSesion;
    int minTamSes;
    int maxTamSes;
    int lenSesion;
    float maxFo;
    float sumaAgrupada;
    float sumaIndividual;

    maxTamSes=-1;
    minTamSes=maxLenSesion + 1;

    //ES POSIBLE USANDO LENSENSION
    for(i=0;i<sol.lenListaSesionesActivas;i++)
		{
		indxSesion=sol.listaSesionesActivas[i];
		lenSesion=sol.lenSesion[indxSesion];
		if(lenSesion<minTamSes)
			minTamSes=lenSesion;
		if(lenSesion>maxTamSes)
			maxTamSes=lenSesion;
		}
    switch(idFo)
        {
        case 1:
            //DECRECIENTE
            if(maxTamSes==1)
                maxTamSes++;

            sumaAgrupada=0.0;
            for(i=1;i<=maxTamSes;i++)
                sumaAgrupada=sumaAgrupada+(float)1/sqrt(i);

            sumaIndividual=maxTamSes;
            maxFo=sumaIndividual-sumaAgrupada;

            break;
        case 2:
           //CRECIENTE
            if(maxTamSes==1)
                maxTamSes++;
            
            sumaAgrupada=0.0;
            for(i=1;i<=maxTamSes;i++)
                sumaAgrupada=sumaAgrupada+(float)log(i);

            sumaIndividual=maxTamSes*(float)log(1);
            maxFo=sumaAgrupada-sumaIndividual;
            break;
        case 3:
            //CRECIENTE
            if(maxTamSes==1)
                maxTamSes++;

            sumaAgrupada=0.0;
            for(i=1;i<=maxTamSes;i++)
                sumaAgrupada=sumaAgrupada+((float)3/2)*log(i)+((float)((float)pow(i-3,2)/(12*i)));

            sumaIndividual=maxTamSes*((float)3/2)*log(1)+((float)((float)pow(1-3,2)/(12*1)));
            maxFo=sumaAgrupada-sumaIndividual;
            break;
        case 4:
            //CRECIENTE
            if(maxTamSes==1)
                maxTamSes++;

            sumaAgrupada=0.0;
            for(i=1;i<=maxTamSes;i++)
                sumaAgrupada=sumaAgrupada+i;

            sumaIndividual=maxTamSes*1;
            maxFo=sumaAgrupada-sumaIndividual;

            break;
        case 5:
            if(maxTamSes==1)
                maxTamSes++;

            //CRECIENTE
            sumaAgrupada=0.0;
            for(i=1;i<=maxTamSes;i++)
                sumaAgrupada=sumaAgrupada+pow(i,2);

            sumaIndividual=maxTamSes*pow(1,2);
            maxFo=sumaAgrupada-sumaIndividual;
            break;
        }

    return maxFo;
    }

/************************************************************************************************************/
estadistica regresionSesion(estadistica staSesion,int lenStaSesion)
    {
    //CASO 6, REGRESION
    int n;
    int j;

    staSesion.powsumX=0;
    staSesion.powsumY=0;
    staSesion.promX=0;
    staSesion.promY=0;
    staSesion.sumX=0;
    staSesion.sumY=0;
    staSesion.sumX2=0;
    staSesion.sumY2=0;
    staSesion.sumXY=0;
    staSesion.sumError2=0;
    staSesion.m=0;
    staSesion.b=0;
    staSesion.desError=0;
    staSesion.error=0;
    staSesion.r=0;
    staSesion.sxx=0;
    staSesion.sxy=0;
    staSesion.syy=0;

    for(j=1;j<lenStaSesion;j++)
        if(staSesion.cantSesiones[j]!=0)
            staSesion.logCantSesiones[j]=log(staSesion.cantSesiones[j]);

    n=0;
    for(j=1;j<lenStaSesion;j++)
        {
        if(staSesion.cantSesiones[j]!=0)
            {
            staSesion.sumX=staSesion.sumX + staSesion.logSesiones[j];
            staSesion.sumY=staSesion.sumY + staSesion.logCantSesiones[j];
            staSesion.sumX2=staSesion.sumX2 + pow(staSesion.logSesiones[j],2);
            staSesion.sumY2=staSesion.sumY2 + pow(staSesion.logCantSesiones[j],2);
            staSesion.sumXY= staSesion.sumXY + staSesion.logCantSesiones[j]*staSesion.logSesiones[j];
            n++;
            }
        }
    staSesion.promX=(float) staSesion.sumX/n;
    staSesion.promY=(float) staSesion.sumY/n;
    staSesion.powsumX=pow(staSesion.sumX,2);
    staSesion.powsumY=pow(staSesion.sumY,2);

    staSesion.sxx=staSesion.sumX2 - (float)staSesion.powsumX/n;
    staSesion.syy=staSesion.sumY2 - (float)staSesion.powsumY/n;
    staSesion.sxy=staSesion.sumXY - (float)(staSesion.sumX*staSesion.sumY)/n;

    staSesion.m=(float)staSesion.sxy/staSesion.sxx;
    staSesion.b=staSesion.promY - staSesion.m*staSesion.promX;

    staSesion.desError=staSesion.syy - staSesion.m*staSesion.sxy;
    staSesion.r=1-(float)staSesion.desError/staSesion.syy;
    staSesion.error=(float)sqrt((float)staSesion.desError/(n-2));

    return staSesion;
    }

/************************************************************************************************************/
solucion generaSolucionInicial(solucion sol)
    {
    /************ ESTE CODIGO GENERA SOLUCION PARA SA ************/

    int i;
    int flagPrimerReg;
    int indxSesion;
    int countPage;
    int idReg;
    int lenListaSesionesActivas;
    int lenSesion;

    //APLICAR CRITERIO ENLACE PAGINA
    sol.lenListaSesionesActivas=0;
    flagPrimerReg=1;
    indxSesion=0;
    countPage=0;

    for(i=0;i<lenWebLog;i++)
        {
        if(flagPrimerReg==1)
            {
            //PRIMER REGISTRO DE LA NUEVA SESION
            sol.sol[countPage][indxSesion]=i;
            sol.lenSesion[indxSesion]=1;

            flagPrimerReg=0;
            countPage++;
            }
        else
            {
            if ( (existeEnlace(webLog[i-1].id,webLog[i].id))
                && (mismaIp(webLog[i-1].ip,webLog[i].ip))
                 && (enMtp(webLog[i-1].timeStamp,webLog[i].timeStamp))
                    && (countPage<=maxLenSesion-1) )
                {
                //AGREGANDO A SESION EXISTENTE
                sol.sol[countPage][indxSesion]=i;
                sol.lenSesion[indxSesion]++;

                countPage++;
                }
           else
                {
                //NUEVA SESION
                lenListaSesionesActivas=sol.lenListaSesionesActivas;
                sol.listaSesionesActivas[lenListaSesionesActivas]=indxSesion;
                sol.lenListaSesionesActivas++;

		lenSesion=sol.lenSesion[indxSesion];

                countPage=0;
                indxSesion++;
                flagPrimerReg=1;
                i=i-1;
                }
            }
	
		}
    //CASO ULTIMO REGISTRO
	if(flagPrimerReg==0)
		{
		lenListaSesionesActivas=sol.lenListaSesionesActivas;
		sol.listaSesionesActivas[lenListaSesionesActivas]=indxSesion;
		sol.lenListaSesionesActivas++;

		lenSesion=sol.lenSesion[indxSesion];
		}
    sol=actualizarStaSesion(sol);
    return sol;
    }
/************************************************************************************************************/
solucion copiarSolucion(solucion origen,solucion retorno)
	{
	//N ES IGUAL A LA CANTIDAD DE REGISTROS EN EL WEB LOG
	int i;
	int j;	
	int indxSesion;
	int lenSesion;
	int lenListaSesionesActivas;	
        
	//COPIANDO SOL
	for(j=0;j<origen.lenListaSesionesActivas;j++)
		{
		indxSesion=origen.listaSesionesActivas[j];
		lenSesion=origen.lenSesion[indxSesion];
		for(i=0;i<lenSesion;i++)
			retorno.sol[i][indxSesion]=origen.sol[i][indxSesion];
		
		retorno.listaSesionesActivas[j]=origen.listaSesionesActivas[j];
		}
	//lenSesion
	for(i=0;i<lenWebLog;i++)
		retorno.lenSesion[i]=origen.lenSesion[i];

	//sta
	for(i=0;i<origen.lenStaSesion;i++)
		{
		retorno.staSesion.cantSesiones[i]=origen.staSesion.cantSesiones[i];
		retorno.staSesion.logCantSesiones[i]=origen.staSesion.logCantSesiones[i];
		retorno.staSesion.logCantSesionesEst[i]=origen.staSesion.logCantSesionesEst[i];
		}
		
	retorno.staSesion.m=origen.staSesion.m;
	retorno.staSesion.powsumX=origen.staSesion.powsumX;
	retorno.staSesion.powsumY=origen.staSesion.powsumY;
	retorno.staSesion.promX=origen.staSesion.promX;
	retorno.staSesion.promY=origen.staSesion.promY;
	retorno.staSesion.r=origen.staSesion.r;
	retorno.staSesion.sumError2=origen.staSesion.sumError2;
	retorno.staSesion.sumX=origen.staSesion.sumX;
	retorno.staSesion.sumX2=origen.staSesion.sumX2;
	retorno.staSesion.sumXY=origen.staSesion.sumXY;
	retorno.staSesion.sumY=origen.staSesion.sumY;
	retorno.staSesion.sumY2=origen.staSesion.sumY2;
	retorno.staSesion.sxx=origen.staSesion.sxx;
	retorno.staSesion.sxy=origen.staSesion.sxy;
	retorno.staSesion.syy=origen.staSesion.syy;

	retorno.lenListaSesionesActivas=origen.lenListaSesionesActivas;
	retorno.lenStaSesion=origen.lenStaSesion;

	return retorno;

	}

/************************************************************************************************************/
int estaRepetido(solucion sol,int indxRegistro,int listaSesionesModificadas[],int lenListaSesionesModificadas)
	{
	int i;
	int j;
	
	int retorno;
	int indxSesion;
	int lenSesion;

	retorno=-1;

	for(j=0;j<sol.lenListaSesionesActivas;j++)
		{
		indxSesion=sol.listaSesionesActivas[j];
		lenSesion=sol.lenSesion[indxSesion];
		if(estaEnSesionModificada(indxSesion,listaSesionesModificadas,lenListaSesionesModificadas)==0)
			{		
			for(i=0;i<lenSesion;i++)
				{
				if(indxRegistro==sol.sol[i][indxSesion])
					{
					retorno=indxSesion;
					break;
					}
				}
			}
		}
	return retorno;
	}
/************************************************************************************************************/
int estaEnSesionModificada(int indxSesion,int listaSesionesModificadas[],int lenListaSesionesModificadas)
	{
	int i;
	int retorno;

	retorno=0;	
	for(i=0;i<lenListaSesionesModificadas;i++)
		{
		if(indxSesion==listaSesionesModificadas[i])
			{			
			retorno=1;
			break;	
			}
		}
	return retorno;
	}
/************************************************************************************************************/
solucion generaVecino(solucion solOrig,solucion sol,int *ptrFlgOpt)
	{
	int i;
	int indxListaSesiones;
	int *listaSesionesActivas;
	int lenListaSesionesActivas;
	int indxSesion;
	int indxSesionChng;
	int indxRegistro;
	int lenSesion;
	int registroEscogido;
	int flgCambio;

	//solucion sol;
	*ptrFlgOpt=0;
	sol=copiarSolucion(solOrig,sol);

	listaSesionesActivas=(int*)calloc(lenWebLog,sizeof(int));
	lenListaSesionesActivas=sol.lenListaSesionesActivas;
	for(i=0;i<lenListaSesionesActivas;i++)
		listaSesionesActivas[i]=sol.listaSesionesActivas[i];

	flgCambio=0;
	while(flgCambio==0)
		{
		//ESCOGER UNA SESION
		indxListaSesiones=rand()%lenListaSesionesActivas;
		indxSesion=listaSesionesActivas[indxListaSesiones];

		//INACTIVARLA
		lenSesion=sol.lenSesion[indxSesion];
		sol=eliminarSesion(sol,indxSesion);

		//ELIMINANDO ENLACES INDIVIDUALES Y ASIGNANDO A LISTA DE SELECCION
		int *listaRegInactivos;
		int *listaRegistros;
		int lenListaRegInactivos;
		int lenListaRegistros;
		int indxListaRegInactivos;

	   
		listaRegInactivos=(int*)calloc(lenSesion,sizeof(int));
		listaRegistros=(int*)calloc(lenSesion,sizeof(int));
		lenListaRegInactivos=0;
		for(i=0;i<lenSesion;i++)
			{
			indxRegistro=sol.sol[i][indxSesion];
			listaRegInactivos[i]=indxRegistro;
			listaRegistros[i]=indxRegistro;
			lenListaRegInactivos=lenListaRegInactivos+1;
			}
		lenListaRegistros=lenListaRegInactivos;

		//ESCOGER UN REGISTRO DE LA LISTA AL AZAR
		while(lenListaRegInactivos>0)
			{
			indxListaRegInactivos=rand()%lenListaRegInactivos;
			registroEscogido=listaRegInactivos[indxListaRegInactivos];

			sol=reasignaRegistro(sol,registroEscogido,&indxSesionChng);
			for(i=indxListaRegInactivos;i<lenListaRegInactivos-1;i++)
				listaRegInactivos[i]=listaRegInactivos[i+1];

			lenListaRegInactivos=lenListaRegInactivos-1;
			}

		//COMPROBAR SI LLEGAMOS A LO MISMO
		if(lenListaRegistros==sol.lenSesion[indxSesionChng])
			{
			 for(i=0;i<lenListaRegistros;i++)
				{
				if(listaRegistros[i]!=sol.sol[i][indxSesionChng])
					{
					flgCambio=1;
					break;
					}
				}
			}
		else
		flgCambio=1;

		//NO HAY CAMBIO.. QUITAR ESA SESION DE LAS POSIBILIDADES
		if(flgCambio==0)
			{
			for(i=indxListaSesiones;i<lenListaSesionesActivas-1;i++)
				listaSesionesActivas[i]=listaSesionesActivas[i+1];
			lenListaSesionesActivas--;
			}
		//IMPORTANTE POR QUE INDICA SESIONIZACION OPTIMA
		if(lenListaSesionesActivas==0)
			{
			//printf("VACIO!");
			flgCambio=1;
			*ptrFlgOpt=1;
			}
		free(listaRegistros);
		free(listaRegInactivos);
		}	
	free(listaSesionesActivas);
	sol=actualizarStaSesion(sol);

	return sol;
	}
/************************************************************************************************************/
solucion actualizarStaSesion(solucion sol)
    {
    int i;
    int lenSesion;
    int indxSesion;

    for(i=0;i<sol.lenStaSesion;i++)
        sol.staSesion.cantSesiones[i]=0;

    for(i=0;i<sol.lenListaSesionesActivas;i++)
        {
        indxSesion=sol.listaSesionesActivas[i];
        lenSesion=sol.lenSesion[indxSesion];
        sol.staSesion.cantSesiones[lenSesion]++;
        }
    return sol;
    }
/************************************************************************************************************/
void totalizarStaSesion(solucion sol)
    {
    int i;
    for(i=0;i<lenStaSesion;i++)
        staSesion.cantSesiones[i]=staSesion.cantSesiones[i]+sol.staSesion.cantSesiones[i];
    }
/************************************************************************************************************/
solucion reasignaRegistro(solucion sol,int registroEscogido,int *ptrSes)
	{
	int i,k;
	int registroTemp;
	int flagFinOperacion;
	int idOperacion;
	int operDescartadas[4];
	int lenOperDescartadas;
	int indxListaSesiones;
	int indxRegistro;
	int indxSesion;
	int lenSesion;
	int posSesion;
	int lenListaSesionesActivas;
	 
	//BASICAMENTE COMO LA MUTACION DEL AG

	/*
	TOMAR UN REGISTRO AL AZAR
	POSIBILIDADES (AL AZAR):
		-FIT AL FINAL DE UNA SESION(0)
		-FIT ENTRE MEDIO DE UNA SESION(1)
		-NUEVA SESION(2)
		-REEMPLAZO (QUE CONSIDERA LA EJECUCION DEL PROCEDIMIENTO ANTERIOR(3)
	*/

	indxRegistro=fit(sol,registroEscogido,&indxSesion);
	if(indxRegistro!=-1)
		{
		//HACER ESPACIO
		lenSesion=sol.lenSesion[indxSesion];
		for(i=lenSesion;i>indxRegistro;i--)
			sol.sol[i][indxSesion]=sol.sol[i-1][indxSesion];
			
		//INSERTAR
		sol.sol[indxRegistro][indxSesion]=registroEscogido;
		sol.lenSesion[indxSesion]++;
		
		//CAMBIAR TAMAÑO DE LA SESION
		lenSesion++;//==sol.lenSesion

		}
	else
		{
		//OSEA CREAR UNA NUEVA SESION UNITARIA!!!!!
		//CHEKEAR SI SE PUEDE ENCAPSULAR EN UNA OPCION
		indxSesion=buscarSesionVacia(sol);
		sol.sol[0][indxSesion]=registroEscogido;
		sol.lenSesion[indxSesion]=1;

		lenListaSesionesActivas=sol.lenListaSesionesActivas;			
		sol.listaSesionesActivas[lenListaSesionesActivas]=indxSesion;
		sol.lenListaSesionesActivas=sol.lenListaSesionesActivas + 1;

		}
        *ptrSes=indxSesion;
	return sol;
	}
/************************************************************************************************************/
solucion eliminarSesion(solucion sol,int indxSesion)
	{
	int i;
	int j;
	int lenListaSesionesActivas;
	int lenSesion;


	lenSesion=sol.lenSesion[indxSesion];
	sol.lenSesion[indxSesion]=0;
	lenListaSesionesActivas=sol.lenListaSesionesActivas;
	for(i=0;i<sol.lenListaSesionesActivas;i++)
		if(indxSesion==sol.listaSesionesActivas[i])
			break;
            
	for(j=i;j<sol.lenListaSesionesActivas;j++)
		sol.listaSesionesActivas[j]=sol.listaSesionesActivas[j+1];

	sol.lenListaSesionesActivas=sol.lenListaSesionesActivas-1;

	return sol;
	}

/************************************************************************************************************/
int operEstaDescartada(int operDescartadas[],int lenOperDescartadas,int idOperacion)
	{
	int i;
	int ret;
	ret=0;
	for(i=0;i<lenOperDescartadas;i++)
		{
		if(operDescartadas[i]==idOperacion)
			{
			ret=1;
			break;
			}
		}
	return ret;
	}
/************************************************************************************************************/
int *initOperDescartadas(int operDescartadas[])
	{
	int i;
	for(i=0;i<4;i++)
		operDescartadas[i]=-1;
	return operDescartadas;
	}
/************************************************************************************************************/
int buscarSesionVacia(solucion sol)
    {
    int i;

    i=0;
    for(i=0;i<lenWebLog;i++)
        if(sol.lenSesion[i]==0)
            break;
        
    return i;//INDICE DE LA SESION
    }
/************************************************************************************************************/
int fit(solucion sol,int registro,int *ptrSes)
    {
    //EJ: r1--->r2; RETONRA r2!!!!
    int i,j;
    int indxAct,indxAnt;
    int indxSesion;
    int lenSesion;
    int indxListaPosibles;
    int indxRegistroEscogido;
    int *listaPosibles;
    int *listaSesion;
    int lenListaPosibles;
    int lenListaSesion;

    int id1;
    int id2;
    int id3;
    char ip1[80];
    char ip2[80];
    char ip3[80];
    int tmp1;
    int tmp2;
    int tmp3;

    listaPosibles=(int*)calloc(lenWebLog,sizeof(int));
    listaSesion=(int*)calloc(lenWebLog,sizeof(int));
    lenListaPosibles=0;
    lenListaSesion=0;

    id2=webLog[registro].id;
    tmp2=webLog[registro].timeStamp;

    for(j=0;j<sol.lenListaSesionesActivas;j++)
        {
        indxSesion=sol.listaSesionesActivas[j];
        lenSesion=sol.lenSesion[indxSesion];
        if(lenSesion<=maxLenSesion-1)
            {
            indxAnt=sol.sol[0][indxSesion];
            strcpy(ip1,webLog[indxAnt].ip);
            strcpy(ip2,webLog[registro].ip);
            if(mismaIp(ip1,ip2))
                {
                //CONDICION DE TIEMPO PARA OPTIMIZAR EL PROCESO
                //CASO LENSESION=1
                if(lenSesion==1)
                    {
                    id1=webLog[indxAnt].id;
                    tmp1=webLog[indxAnt].timeStamp;
                    //FIT PRINCIPIO
                    if((existeEnlace(id2,id1))
                        && (enMtp(tmp2,tmp1)) )
                        {
                        listaPosibles[lenListaPosibles]=0;
                        listaSesion[lenListaSesion]=indxSesion;
                        lenListaPosibles++;
                        lenListaSesion++;
                        }
                    if ( (existeEnlace(id1,id2))
                       && (enMtp(tmp1,tmp2)) )
                        {
                        listaPosibles[lenListaPosibles]=1;
                        listaSesion[lenListaSesion]=indxSesion;
                        lenListaPosibles++;
                        lenListaSesion++;
                        }
                    }
                else
                    {
                    //CONDICION OPTIMIZADORA DE TIEMPO
                    indxAnt=sol.sol[0][indxSesion];
                    indxAct=sol.sol[lenSesion-1][indxSesion];
                    tmp1=webLog[indxAnt].timeStamp;
                    tmp3=webLog[indxAct].timeStamp;

                    if( ((tmp1<=tmp2) &&
                            ((tmp2<=tmp3)||(enMtp(tmp3,tmp2)))
                       ) ||
                            (enMtp(tmp2,tmp1)) )

						{
						for(i=1;i<lenSesion;i++)
							{
							indxAnt=sol.sol[i-1][indxSesion];
							indxAct=sol.sol[i][indxSesion];
							id1=webLog[indxAnt].id;
							id3=webLog[indxAct].id;
							tmp1=webLog[indxAnt].timeStamp;
							tmp3=webLog[indxAct].timeStamp;

							//FIT PRINCIPIO
							if ( (i==1)
								&& (existeEnlace(id2,id1))
								&& (enMtp(tmp2,tmp1)) )
								{
								listaPosibles[lenListaPosibles]=i-1;
								listaSesion[lenListaSesion]=indxSesion;
								lenListaPosibles++;
								lenListaSesion++;
								}
							//FIT MEDIO
							if (( (existeEnlace(id1,id2)) && (existeEnlace(id2,id3)) )
							   && ( (enMtp(tmp1,tmp2)) && (enMtp(tmp2,tmp3)) )    )
								{
								listaPosibles[lenListaPosibles]=i;
								listaSesion[lenListaSesion]=indxSesion;
								lenListaPosibles++;
								lenListaSesion++;
								}
							//FIT FINAL
							if ( (i==lenSesion-1)
								&& (existeEnlace(id3,id2))
							   && (enMtp(tmp3,tmp2)) )
								{
								listaPosibles[lenListaPosibles]=i+1;
								listaSesion[lenListaSesion]=indxSesion;
								lenListaPosibles++;
								lenListaSesion++;
								}
							}
						}
                    }
                }
            }
        }
    //MOVIMIENTO DE SESION INDIVIDUAL
    listaPosibles[lenListaPosibles]=-1;
    listaSesion[lenListaSesion]=-1;
    lenListaPosibles++;
    lenListaSesion++;

    if(lenListaPosibles!=0)
        {
        //EXISTEN UNA O MAS POSIBILIDADES
        //ESCOGER AL AZAR
        indxListaPosibles=rand()%lenListaPosibles;
        indxRegistroEscogido=listaPosibles[indxListaPosibles];
        *ptrSes=listaSesion[indxListaPosibles];
        free(listaPosibles);
        free(listaSesion);
        return indxRegistroEscogido;
        }
    else
        {
        free(listaPosibles);
        free(listaSesion);
        return -1;
        }
    }





