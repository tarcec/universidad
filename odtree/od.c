#include <stdio.h>
#include <stdlib.h>

/*DEFINICION DE ESTRUCTURA*/
typedef struct OD
	{
	int valor;
	struct OD *ptr1;
	struct OD *ptr2;
	struct OD *ptr3;
	struct OD *ptr4;
	struct OD *ptr5;
	struct OD *ptr6;
	struct OD *ptr7;
	struct OD *ptr8;
	}
od;

void insertar(od *abo);
void parentesco(od *abo);
void lista_hijos(od *abo);
int estado_hijo(od *abo,int hijo);
od *insertar_hijo(od *abo,int hijo,int value);
od *ir_hijo(od *abo,int hijo);
void repetido(od *abo,int num,int nivel,int valAnt);


int encontrado;
int level;
int padre;


/*****************************/
main()
	{
	char txtOpc[2];
	od *abo;
	int flag;

	abo=(od*)malloc(sizeof(od));
	abo=NULL;

	flag=0;

	do
		{
		fflush(stdin);

		printf("\nMENU:");
		printf("\n=====");

		printf("\n1 INSERTAR");
		printf("\n2 PARENTESCO ENTRE NODOS");
		printf("\n3 SALIR");

		printf("\nELIJA UNA OPCION:");
		scanf("%c",txtOpc);

		switch(txtOpc[0])
			{
			case '1':
				{
				insertar(abo);
				break;
				}
			case '2':
				{
				parentesco(abo);
				break;
				}
			case '3':
				{
				flag=1;
				break;
				}
			}
		}
	while(flag==0);

	/*LIMPIAR*/


	}

/*************************************************/
void insertar(od *abo)
	{
	int num;
	int flag=0,flag1;
	int i;
	int hijo;
	int value;

	char txtOpcion[1];

	od *temp=abo;

	do
		{
		fflush(stdin);
		if (temp==NULL)
			{
			printf("\nNO HAY NODOS EN EL ARBOL");
			printf("\nINGRESE UN NUMERO: ");
			scanf("%d",&num);

			/*CREAR EL NODO*/
			temp->ptr1=NULL;
			temp->ptr2=NULL;
			temp->ptr3=NULL;
			temp->ptr4=NULL;
			temp->ptr5=NULL;
			temp->ptr6=NULL;
			temp->ptr7=NULL;
			temp->ptr8=NULL;
			temp->valor=num;

			}
		else
			{

			flag1=0;
			do
				{
				fflush(stdin);

				printf("\nNODO ACTUAL CON VALOR %i",temp->valor);
				printf("\nLISTA DE HIJOS : ");

				lista_hijos(temp);

				printf("\nELIJA UNA OPCION:");
				printf("\n1 IR A UN HIJO");
				printf("\n2 INSERTAR EN UN HIJO NULO");
				printf("\n3 SALIR");

				scanf("%c",txtOpcion);

				switch(txtOpcion[0])
					{
					case '1':
						{
						printf("\nINGRESE EL NUMERO DEL HIJO AL CUAL QUIERE IR: ");
						scanf("%d",&hijo);

						if (estado_hijo(temp,hijo)==1)
							{
							printf("\n*** ELECCION INVALIDA ***");
							}
						else
							{
							temp=ir_hijo(temp,hijo);
							flag1=1;
							}
						break;
						}
					case '2':
						{
						printf("\nINGRESE EL NUMERO DEL HIJO DONDE QUIERE INSERTAR: ");
						scanf("%d",&hijo);


						if (estado_hijo(temp,hijo)!=1)
							{
							printf("\n*** ELECCION INVALIDA ***");
							}
						else
							{
							printf("\nINGRESE EL VALOR A INSERTAR : ");
							scanf("%d",value);

							encontrado=0;
							repetido(temp,value,0,0);

							if (encontrado==1)
								{
								printf("\n*** EL NUMERO YA EXISTE EN EL ARBOL ***");
								}
							else
								{
								temp=insertar_hijo(temp,hijo,value);
								flag1=1;
								}
							}

						break;
						}
					case '3':
						{
						flag=1;
						break;
						}
					}
				}
			while (flag1==0);
			}
		}
	while (flag==0);

	}
/*************************************************/
void parentesco(od *abo)
	{
	int num1,num2;
	int flag;
	int padre1,padre2;
	int nivel1,nivel2;

	flag=0;
	do
		{
		fflush(stdin);

		printf("\nINGRESE EL VALOR QUE IDENTIFICA AL NODO 1 :");
		scanf("%d",num1);

		encontrado=0;
		repetido(abo,num1,0,0);
		if (encontrado==0)
			{
			printf("*** EL NODO NO EXISTE ***");
			}
		else
			{
			padre1=padre;
			nivel1=level;
			flag=1;
			}
		}
	while (flag==0);


	flag=0;
	do
		{
		fflush(stdin);

		printf("\nINGRESE EL VALOR QUE IDENTIFICA AL NODO 2 :");
		scanf("%d",num2);

		encontrado=0;
		repetido(abo,num2,0,0);
		if (encontrado==0)
			{
			printf("*** EL NODO NO EXISTE ***");
			}
		else
			{
			padre2=padre;
			nivel2=level;
			flag=1;
			}
		}
	while (flag==0);

	/*BUSCAR PARENTEZCO*/
	if (padre1==padre2)
		{
		printf ("\nLOS NODOS SON HERMANOS");
		}
	else
		{
		if (nivel1==nivel2)
			{
			printf("\nLOS NODOS SON PRIMOS");
			}
		else
			{
			if (nivel1>nivel2)
				{
				printf("\nEL NODO1 ES ANTEPASADO DEL NODO2");
				}
			else
				{
				printf("\nEL NODO2 ES ANTEPASADO DEL NODO1");
				}
			}
		}

	}
/*************************************************/
void lista_hijos(od *abo)
	{
	if (abo->ptr1==NULL)
		{
		printf("\n1 NULO.");
		}
	else
		{
		printf("\n1 CON VALOR.");
		}

	if (abo->ptr2==NULL)
		{
		printf("\n2 NULO.");
		}
	else
		{
		printf("\n2 CON VALOR.");
		}

	if (abo->ptr3==NULL)
		{
		printf("\n3 NULO.");
		}
	else
		{
		printf("\n3 CON VALOR.");
		}

	if (abo->ptr4==NULL)
		{
		printf("\n4 NULO.");
		}
	else
		{
		printf("\n4 CON VALOR.");
		}

	if (abo->ptr5==NULL)
		{
		printf("\n5 NULO.");
		}
	else
		{
		printf("\n5 CON VALOR.");
		}

	if (abo->ptr6==NULL)
		{
		printf("\n6 NULO.");
		}
	else
		{
		printf("\n6 CON VALOR.");
		}
	if (abo->ptr7==NULL)
		{
		printf("\n7 NULO.");
		}
	else
		{
		printf("\n7 CON VALOR.");
		}
	if (abo->ptr8==NULL)
		{
		printf("\n8 NULO.");
		}
	else
		{
		printf("\n8 CON VALOR.");
		}
	}
/*************************************************/
int estado_hijo(od *abo,int hijo)
	{

	if (hijo==1)
		{
		if (abo->ptr1==NULL)
			{
			return(1);
			}
		}
	if (hijo==2)
		{
		if (abo->ptr2==NULL)
			{
			return(1);
			}
		}

	if (hijo==3)
		{
		if (abo->ptr3==NULL)
			{
			return(1);
			}
		}

	if (hijo==4)
		{
		if (abo->ptr4==NULL)
			{
			return(1);
			}
		}

	if (hijo==5)
		{
		if (abo->ptr5==NULL)
			{
			return(1);
			}
		}

	if (hijo==6)
		{
		if (abo->ptr6==NULL)
			{
			return(1);
			}
		}

	if (hijo==7)
		{
		if (abo->ptr7==NULL)
			{
			return(1);
			}
		}
	if (hijo==8)
		{
		if (abo->ptr8==NULL)
			{
			return(1);
			}
		}
	return(0);

	}
/***********************************************************/
od *insertar_hijo(od *abo,int hijo,int value)
	{
	od *tmp;

	tmp=(od*)malloc(sizeof(od));
	tmp->ptr1=NULL;
	tmp->ptr2=NULL;
	tmp->ptr3=NULL;
	tmp->ptr4=NULL;
	tmp->ptr5=NULL;
	tmp->ptr6=NULL;
	tmp->ptr7=NULL;
	tmp->ptr8=NULL;
	tmp->valor=value;


	if (hijo==1)
		{
		abo->ptr1=tmp;
		}
	if (hijo==2)
		{
		abo->ptr2==tmp;
		}

	if (hijo==3)
		{
		abo->ptr3==tmp;
		}

	if (hijo==4)
		{
		abo->ptr4==tmp;
		}

	if (hijo==5)
		{
		abo->ptr5==tmp;
		}

	if (hijo==6)
		{
		abo->ptr6==tmp;
		}

	if (hijo==7)
		{
		abo->ptr7==tmp;
		}
	if (hijo==8)
		{
		abo->ptr8==tmp;
		}

	return(abo);


	}
/*******************************************************/
od *ir_hijo(od *abo,int hijo)
	{
	if (hijo==1)
		{
		abo=abo->ptr1;
		}
	if (hijo==2)
		{
		abo=abo->ptr2;
		}

	if (hijo==3)
		{
		abo=abo->ptr3;
		}

	if (hijo==4)
		{
		abo=abo->ptr4;
		}

	if (hijo==5)
		{
		abo=abo->ptr5;
		}

	if (hijo==6)
		{
		abo=abo->ptr6;
		}

	if (hijo==7)
		{
		abo=abo->ptr7;
		}
	if (hijo==8)
		{
		abo=abo->ptr8;
		}

	return(abo);



	}
/******************************************/
void repetido(od *abo,int num,int nivel,int valAnt)
	{

	if (num==abo->valor)
		{
		encontrado=1;
		level=nivel;
		padre=valAnt;
		}
	else
		{
		if (abo->ptr1!=NULL)
			{
			repetido(abo->ptr1,num,nivel+1,abo->valor);
			}

		if (abo->ptr2!=NULL)
			{
			repetido(abo->ptr2,num,nivel+1,abo->valor);
			}
		if (abo->ptr3!=NULL)
			{
			repetido(abo->ptr3,num,nivel+1,abo->valor);
			}
		if (abo->ptr4!=NULL)
			{
			repetido(abo->ptr4,num,nivel+1,abo->valor);
			}
		if (abo->ptr5!=NULL)
			{
			repetido(abo->ptr5,num,nivel+1,abo->valor);
			}
		if (abo->ptr6!=NULL)
			{
			repetido(abo->ptr6,num,nivel+1,abo->valor);
			}
		if (abo->ptr7!=NULL)
			{
			repetido(abo->ptr7,num,nivel+1,abo->valor);
			}
		if (abo->ptr8!=NULL)
			{
			repetido(abo->ptr8,num,nivel+1,abo->valor);
			}
		}
	}
/*************************************************************/
