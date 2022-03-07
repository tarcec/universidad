#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
/*QUITAR SI SE COMPILA EN LINUX*/
#include <conio.h>


/*ESTRUCTURA ARBOL*/
typedef struct ARBOL
	{
	int valor;
	struct ARBOL *izq;
	struct ARBOL *der;
	}
arbol;


int ES_VACIO(arbol *arbolEntrada);
arbol *ANULAR_ABO(arbol *arbolEntrada);
arbol *crearNodo(int elementoEntrada);
arbol *INSERTAR(arbol *arbolEntrada,int elementoEntrada);
void imprimeArbol(arbol *arbolEntrada,int nivelEntrada);
int validar(char valida[]);

/*******************************************************/
main()
	{
	int num;
	char txtValidar[7];
	char txtOpcion[1];
	int flag,flag1;
	arbol *abo;

	/*INICIALIZAR ARBOL*/
	abo=(arbol*) malloc(sizeof(arbol));
	abo=NULL;

	flag1=0;
	do
		{
		/*PEDIR AL USUARIO QUE INSERTE ELEMENTOS*/
		flag=0;
		do
			{
			printf("\nINGRESE UN NUMERO PARA INSERTAR: ");
			scanf("%7s",txtValidar);

			if (validar(txtValidar)!=-1)
				{
				num=atoi(txtValidar);
				flag=1;
				}
			}
		while (flag==0);

		/*INSERTAR EN EL ARBOL*/
		abo=INSERTAR(abo,num);


		fflush(stdin);
		/*INSERTAR OTRO NUMERO?*/
		printf("\nDESEA INSERTAR OTRO NUMERO [s/n]?");
		scanf("%c",txtOpcion);

		switch(txtOpcion[0])
			{
			case 's':
				{
				flag1=0;
				break;
				}
			case 'S':
				{
				flag1=0;
				break;
				}
			case 'N':
				{
				flag1=1;
				break;
				}
			case 'n':
				{
				flag1=1;
				break;
				}
			}
		}
	while(flag1==0);



	/*MOSTRAR ARBOL*/
	printf("\nCONTENIDO DEL ARBOL");
	printf("\n===================");

	imprimeArbol(abo,1);


	/*LIMPIAR ARBOL*/
	abo=ANULAR_ABO(abo);

	/*QUITAR SI SE COMPILA EN LINUX*/
	printf("\nPRESIONE UNA TECLA PARA SALIR\n");
	getch();

	}
/********************************************************/
int ES_VACIO(arbol *arbolEntrada)
	{
	if (arbolEntrada==NULL)
		{	/*retorna 1 si el arbol esta vacio*/
		return (1);
		}
	else
		{	/*retorna 0 si el arbol tiene elementos*/
		return (0);
		}
	}

/**************************************************************************/
arbol *ANULAR_ABO(arbol *arbolEntrada)
	{
	arbol *tmp;

	if (ES_VACIO(arbolEntrada->izq)==0) /* el arbol izquierdo NO esta vacio */
		{
		tmp = ANULAR_ABO(arbolEntrada->izq);
		}
	if (ES_VACIO(arbolEntrada->der)==0) /* el arbol izquierdo NO esta vacio */
		{
		tmp = ANULAR_ABO(arbolEntrada->der);
		}
	if ((ES_VACIO(arbolEntrada->izq)==1) && (ES_VACIO(arbolEntrada->der)==1)) /* el arbol ESTA vacio*/
		{
		free(arbolEntrada);
		tmp=NULL;
		}
	return(tmp);
	}
/************************************************************************/
arbol *crearNodo(int elementoEntrada)
	{
	arbol *aux;
	aux=(arbol *)malloc(sizeof(arbol));
	aux->izq=NULL;
	aux->der=NULL;
	aux->valor=elementoEntrada;
	return(aux);
	}
/***********************************************************************/
arbol *INSERTAR(arbol *arbolEntrada,int elementoEntrada)
	{
	int i=0;
	arbol *tmp=arbolEntrada;

	while (i==0)
		{
		if (ES_VACIO(tmp)==1) /* si el arbol esta vacio */
			{
			arbolEntrada=crearNodo(elementoEntrada);
			i=1;
			}
		else
			{
			if (elementoEntrada <= tmp->valor)
				{
				if (ES_VACIO(tmp->izq)==1) /* el arbol izquierdo esta vacio */
					{
					tmp->izq = crearNodo(elementoEntrada);
					i=1;
					}
				else
					{
					tmp=tmp->izq;
					}
				}
			else
				{
				if (ES_VACIO(tmp->der)==1) /* el arbol derecho esta vacio */
					{
					tmp->der = crearNodo(elementoEntrada);
					i=1;
					}
				else
					{
					tmp=tmp->der;
					}
				}
			}
		}
	tmp=arbolEntrada;
	return(tmp);

	}
/**********************************************************/
void imprimeArbol(arbol *arbolEntrada,int nivelEntrada)
	{
	int i=0,nivel=0;
	printf("\n\t\t\033");

	for (i=0;i<nivelEntrada;i++)
		{
		printf("  ");
		}
	printf("->%d ",arbolEntrada->valor);

	nivel=nivelEntrada + 1;


	if (ES_VACIO(arbolEntrada->izq)==0) /* el arbol izquierdo NO esta vacio */
		{
		imprimeArbol(arbolEntrada->izq, nivel);
		}

	if (ES_VACIO(arbolEntrada->der)==0) /* el arbol derecho NO esta vacio */
		{
		imprimeArbol(arbolEntrada->der, nivel);
		}
	}
/***********************************************************/

int validar(char valida[])
	{
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
