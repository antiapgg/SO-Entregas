
/********************************************************************************
*																				*
*							   PRÁCTICA INTERMEDIA   							*
*						   SIMULACIÓN CARRERA DE KARTS							*
*																				*
*	*************************************************************************	*
*																				*
*																				*
*	OBJETIVOS:																	*
*																				*
*		- Manejos de procesos 													*
*		- Mecanismos de sincronización entre procesos							*
*																				*
*																				*
*	ENUNCIADO PRÁCTICA:															*
*																				*
*		1º PARTE: CARRERA DE KARTS 												*
*			- Programa que simula una carrera de KARTS. Se trabajará con varios	*
*				procesos. Para poder seguir la actividad del programa, cada 	*
*				proceso debe mostrar trazas de lo que hace en cada momento, 	*
*				identificanco qué proceso hace cada cosa.						*
*				Participantes:													*
*					* Juez principal- proceso principal							*
*						Al lanzarse crea al juez principal y a los corredores.	*
*						Una vez creados,quedan esperando hasta recibir una señal*
*						Para comenzar la carrera debe asegurar que la pista esta*
*						en condiciones (envía señal SIGUSR1 al juez de pista) @	*
*						Si recibe 0, duerme 3 seg, y envía una señal al juez de *
*						pista para que lo vuelva a comprobar.					*
*						Si recibe 1, manda una señal a los corredores y se queda*
*						esperando por ellos.									*
*						El proceso principal debe mostrar la lista de tiempos 	*
*						de los procesos que corren, determinando el PID y núm 	*
*						de segundos empleados en la carrera.					*
*						Cuando se imprima esto, acabará la carrera, y se imprime*
*						mensaje al respecto.									*
*					* Juez de pista- proceso hijo del principal					*
*						Debe verificar el estado de la pista. @ Para comprobarlo*
*						dicho proceso dormirá 5 seg y generará un aleatorio 	*
*						(entre 0 y 1) según el resultado:						*
*							0: envía SIGUSR1 La carrera NO puede continuar		*
*							1: envía SIGUSR2 La carrera SÍ puede continuar		*
*						Si sale 1, el juez de pista finaliza su ejecución		*
*					* Corredores- procesos hijos del principal. 				*
*						Nº de Corredores dado por el num de args del programa 	*
*						Si  arg = 3, se crean 3 procesos hijos que los simulen	*
*						En cada proceso hijo se duermen un num aleatorio de seg *
*						(entre 1 y 5) y se devuelve al proceso padre el num de  *
*						seg que se ha dormido. Una vez hecho esto, el hijo 		*
*						finaliza la carrera.									*
*			- Funciones en C:													*
*					* Calcular núms aleatorios en un intervalo: 				*
*							Biblioteca:											*
*								#include <stdlib.h>								*
*							Función:											*
*								int calculaAleatorios(int min, int max){		*
*									return rand() % (max - min + 1) + min;		*
*								}												*
*							Iniciación semilla (en main):						*
*								srand (time(NULL));								*
*					* Dormir procesos:											*
*							Suspender su ejecución un num de segundos. Función:	*
*								sleep(x);										*
*																				*
*																				*
*	OTROS ASPECTOS A TENER EN CUENTA:											*
*																				*
*		- La practica debe funcionar correctamente.								*
*		- No deben usarse variables globales.									*
*		- Los nombres de las variables deben ser inteligibles.					*
*		- El código debe estar indentado y deben usarse comentarios.			*
*		- Para esta práctica no deben utilizarse threads, solamente procesos 	*
*			y señales.															*			
*																				*
********************************************************************************/

 //Librerias
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

//#include <sys/types.h>

//Declaración de funciones
int calculaAleatorios(int min, int max);
void Manejadora1();	//Manejadora de Juez Principal
void Manejadora2();	//Manejadora de Juez de Pista si recibe señal SIGUSR1
void Manejadora3();	//Manejadora de Juez de Pista si recibe señal SIGUSR2
void Manejadora4();	//Manejadora de Corredores

//Variable global
pid_t juezPista;

//Función principal main
int main(int argc, char *argv[]){

	if(argc != 2){
		perror("\nNúmero de argumentos no válido.\n");
	}

	//Cojo el argumento para saber el número de corredores
	int aux = atoi(argv[1]);
	//Declaración de variables
	int i;
	int aleatorio;
	int tiempo = 0; 
	int estado;
	int cont = 1;
	int ganador = 0;
	int tGanador = 6;
	int corredores[aux];
	int dormirCorredores[aux];
	int empate[aux];
	int empateCorredores[aux];
	pid_t juezPrincipal; 
	pid_t corredor;

	//Imprimimos el Juez Principal
	printf("\nJuez Principal PID: %u.\n", getppid());

	//Bucle que crea procesos hijo
	for(i = 0; i <= aux; i++){
		//Creo Procesos hijo
		juezPrincipal = fork();
		//Iniciamos la semilla
		srand(getpid());

			  /******************************/
			 /** CODIGO DE LOS CORREDORES **/
			/******************************/			
			if((i != 0) && (juezPrincipal == 0)){

				//Imprimo por pantalla a los corredores
				printf("\nCorredor %d PID:     %u.\n", i, getpid());
				wait(NULL);	//Los dejo esperando por una señal

				signal(SIGUSR2, Manejadora4);			//Cuando reciba señal SIGUSR2 llamo a Manejadora4 (la de corredores)
				pause();								//Los pauso

				aleatorio = calculaAleatorios(1, 5);	//Calculo el numero aleatorio de segundos que duermo
				//sleep(aleatorio);						//Duermo a cada corredor el numero de segundos indicado
				exit(aleatorio);						//Devuelvo el numero de segundos
			}
			else{	

				  /******************************/
				 /** CODIGO DEL JUEZ DE PISTA **/
				/******************************/
				if((i == 0) && (juezPrincipal == 0)){

					//Imprimo por pantalla de  juez de pista
					printf("\nJuez de Pista PID:  %u.\n", getpid());
					signal(SIGUSR1, Manejadora1);	//Cuando recibo la señal SIGUSR1 llamo a la Manejadora1(La del juez de Pista)
					while(1) pause(); 				//Pauso al juez de Pista
				}
				else{ 
					if(i == 0){
						juezPista = juezPrincipal;		//Guardo el pid del juez de Pista
					}
					else{
						corredores[i] = juezPrincipal;	//Guardo el pid de cada uno de los corredores
					}	
				}
			}
	}

	  /*******************************/
	 /** CODIGO DEL JUEZ PRINCIPAL **/
	/*******************************/

	//Cuando recibo la señal SIGUSR1 llamo a Manejadora 2			
	signal(SIGUSR1, Manejadora2);
	//Cuando recibo la señal SIGUSR2 llamo a Manejadora 3	
	signal(SIGUSR2, Manejadora3);
	//Duermo al juez Principal
	sleep(1);
	//Envio señal SIGUSR1 a juez de Pista
	kill(juezPista, SIGUSR1);
	printf("\n 			El juez Principal envía la señal SIGUSR1 al juez de Pista.\n");
	//Lo dejo esperando a que reciba una señal
	wait(NULL);

	//Recorro el bucle de los corredores
	for(i = 1; i <= aux; i++){
		
		printf("\n 			El juez Principal envía la señal SIGUSR2 al corredor %d.\n", corredores[i]);
		//Mando la señal SIGUSR2 a cada uno de los corredores
		kill(corredores[i], SIGUSR2);
		//Espero a que me devuelva el estado	
		wait(&estado);
		//Guardo el estado que acabo de recibir en un array 
		dormirCorredores[i] = WEXITSTATUS(estado);	
		//Aumento el entero tiempo sumandole el estado que acabo de recibir
		tiempo += WEXITSTATUS(estado);		
	}

	//En este codigo compruebo 	quien ha sido el ganador	
	for(i = 1; i <= aux; i++){		
		sleep(dormirCorredores[i]);	//Duermo a cada corredor el numero de segundos indicado
		printf("\nEl corredor %d tarda: %d segundos.\n", corredores[i], dormirCorredores[i]);	//Imprimo el tiempo que tarda cada corredor en realizar la carrera
		if(tGanador == dormirCorredores[i]){
			
			if(cont == 1){
				empate[cont] = tGanador;
				empateCorredores[cont] = ganador;
			}
			cont++;
			empate[cont] = dormirCorredores[i];
			empateCorredores[cont] = corredores[i];
		}
		if(tGanador > dormirCorredores[i]){
			tGanador = dormirCorredores[i];
			ganador = corredores[i];
		}
		
	}

	//Imprimo el tiempo total de la carrera y el ganador
	printf("\nLa suma de los tiempos empleados por los corredores para finalizar la carrera es: %d segundos.\n", tiempo);
	if(cont != 0 ){
		if(tGanador == empate[1]){
			printf("\nLa carrera ha finalizado.\n\n¡EMPATE!\nHay %d ganadores: \n ", cont);
			for(i = 1; i <= aux; i++){
				if(empate[i] == tGanador){
					printf("\n 	Corredor %d con un tiempo de %d segundos. \n", empateCorredores[i], empate[i]);
				}	

			}
			printf("\n¡FELICIDADES CORREDORES!\n");
		}
		else{
			printf("\n****** LA CARRERA HA FINALIZADO. ******\n\nEl ganador de la carrera es el corredor %d con un tiempo de %d segundos. \n\n 	¡FELICIDADES CORREDOR %d!\n", ganador, tGanador, ganador);
		}
	}
	else{
		printf("\nLa carrera ha finalizado.\n\nEl ganador de la carrera es el corredor %d con un tiempo de %d segundos. \n\n 	¡FELICIDADES CORREDOR %d!\n", ganador, tGanador, ganador);
	}


}

/************************************************************************
 * 					FUNCIÓN CALCULA NUMEROS ALEATORIOS					*
 * 																		*
 * Función que calcula un número aleatorio entre 2 números que se le 	*
 * pasan por argumento (uno mínimo y otro máximo)						*
 ***********************************************************************/
int calculaAleatorios(int min, int max){	

    return rand() % (max-min+1) + min;	
}


/************************************************************************
 * 						MANEJADORA JUEZ DE PISTA 						*
 * 																		*
 * Calcula un numero aleatorio para saber si la pista esta preparada. 	*
 * Si sale un 0, le envía la señal SIGUSR1 al juez Principal			*
 *	(La carrera aún no se puede realizar)								*
 * Si sale un 1, le envía la señal SIGUSR2 al juez Principal y devuelve *
 * un 1	(La carrera se puede realizar)									*
 ***********************************************************************/
void Manejadora1(){

	int aleatorio;
	printf("\n 			El juez de Pista recibe la señal SIGUSR1.\n");
	//Preguntamos si se puede realizar la carrera
	printf("\nJuez de Pista, ¿Podemos empezar la carrera?\n");

	if(juezPista == 0){

		//Para saberlo, generamos un numero aleatorio entre 0 y 1
		aleatorio = calculaAleatorios(0, 1);
		//Si el numero es 0, no podemos realizar la carrera aun.
		if(aleatorio == 0){
			//Mando a JuezPrincipal la señal SIGUSR1
			printf("\n 			El juez de Pista envía la señal SIGUSR1 al juez Principal.\n");
			kill(getppid(), SIGUSR1);
			
		}
		else{
			//Mando a Juez Principal la señal SIGUSR2	
			printf("\n 			El juez de Pista envía la señal SIGUSR2 al juez Principal.\n");
			kill(getppid(), SIGUSR2);
		
			exit(1);	//Devuelvo un 1
		}
	}
}

/************************************************************************
 * 					MANEJADORA JUEZ PRINCIPAL SIGUSR1 					*
 * 																		*
 * Si la pista no esta preparada, duerme 3 segundos al juez de pista y 	*
 * le envia la señal SIGUSR1 al Juez Principal 							*
 ***********************************************************************/
void Manejadora2(){

	printf("\n 			El juez Principal recibe la señal SIGUSR1.\n");
	//La carrera no se puede realizar
	printf("\nNo, la pista aún no está preparada.\n");
	//Duermo a juez Principal 3 segundos
	sleep(3);
	//Mando a JuezPista la señal SIGUSR1 para que vuelva a 
	kill(juezPista, SIGUSR1);
	printf("\n 			El juez Principal envía la señal SIGUSR1 al juez de Pista.\n");
}

/************************************************************************
 * 					MANEJADORA JUEZ PRINCIPAL SIGUSR2 					*
 * 																		*
 * Si la pista esta preparada, imprime un mensaje.						*
 ***********************************************************************/
void Manejadora3(){

	printf("\n 			El juez Principal recibe la señal SIGUSR2.\n");
	//La carrera se puede realizar
	printf("\nSi, la pista está preparada.\n\n****** COMENZAMOS LA CARRERA. ******\n");	

}

/************************************************************************
 * 						MANEJADORA DE CORREDORES						*
 * 																		*
 * Si la pista esta preparada, imprime un mensaje.						*
 ***********************************************************************/
void Manejadora4(){

	printf("\n 			El corredor recibe la señal SIGUSR2.\n");
	//Verifico que la señal se recibió
	printf("\n 	El corredor %d puede iniciar la carrera.\n", getpid());
}
