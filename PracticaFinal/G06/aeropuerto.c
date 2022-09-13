 

   //////////////////////////////////////////////////////////////////////////////////////////
  //				DECLARACION DE LIBRERIAS				  //
 //////////////////////////////////////////////////////////////////////////////////////////

	# include <stdio.h>
	# include <stdlib.h>
	# include <ctype.h>
	# include <time.h>
	# include <pthread.h>
	# include <signal.h>
	# include <sys/types.h> 
	# include <sys/syscall.h>
	# include <sys/wait.h>												
	# include <unistd.h>

   
   //////////////////////////////////////////////////////////////////////////////////////////
  //				DECLARACION DE FUNCIONES 				  //
 //////////////////////////////////////////////////////////////////////////////////////////

	pid_t gettid(void);
	void writeLogMessage(char *id, char *msg);
	int calculaAleatorios(int min,int max);
	void finalizar(int sig);
	void *accionesUsuario(void *idUsuario);
	int posicion(int id);
	void *agenteSeguridad(void *ptr);
	void *accionesFacturador(void *ptr);
	void nuevoUsuario(int sig);
	void sacarCola(int pos);
	int getAtendido(int pos);
	int getContadorUsuario();
	int buscoUsuario(int tipofacturador);
	int buscaUsuarioTipo(int tipofacturador);
	int mayorTiempoEspera(void);
	int getFinalizarPrograma();
	int getID(int i);


   //////////////////////////////////////////////////////////////////////////////////////////
  //				DECLARACION DE CONSTANTES				  //
 //////////////////////////////////////////////////////////////////////////////////////////

	#define FACTURADORES 2.0
	#define PASAJEROS 10.0


   //////////////////////////////////////////////////////////////////////////////////////////
  //				DECLARACION DE SEMÁFOROS GLOBALES			  //
 //////////////////////////////////////////////////////////////////////////////////////////

	pthread_mutex_t facturacion;
	pthread_mutex_t loger;	
	pthread_mutex_t ctrl_seguridad;
	pthread_mutex_t usuarios;
	pthread_mutex_t global;
	pthread_mutex_t segurata;

	pthread_cond_t seguridad;
	pthread_cond_t apto;


   //////////////////////////////////////////////////////////////////////////////////////////
  //				DECLARACION DE STRUCTS					  //
 //////////////////////////////////////////////////////////////////////////////////////////

	struct usuario{

		pthread_t hiloUsuario;
		int id;
		int atendido;// toma el valor 0 si no ha facturado, 1 si esta facturando y 2 si ya ha facturado
		int ha_facturado;
		int facturador;
		int salir;

	}*usuario_;

	struct facturador{

		pthread_t hiloFacturador;
		int id;
		


	}*facturador_;

	//Puntero que guarda el contenido del archivo
	FILE *logFile;
	char *logFileName = "registroTiempos.log";


   //////////////////////////////////////////////////////////////////////////////////////////
  //				DECLARACION DE VARIABLES				  //
 //////////////////////////////////////////////////////////////////////////////////////////

	int contadorUsuario; 			//declaro un contador de usuarios para que no entren mas de 10
	int finalizarPrograma;			// Sería como un boolean 1 si finalizamos programa
	int tamCola; 				//Cola usuarios en facturacion
	int tamFacturado;
	int UsuarioEnControl;
	int usuariosAtendidosTotal;


   //////////////////////////////////////////////////////////////////////////////////////////
  //				FUNCIÓN PRINCIPAL MAIN 					  //
 //////////////////////////////////////////////////////////////////////////////////////////

	int main(int argc, char* argv[]){	

		printf("\n\n----------------------------AEROPUERTO------------------------------\n\n");
		printf("\tEl aeropuerto inicializara 2 facturadores con un maximo de 10 usuarios:\n");
		printf("\tUtilize 'kill - 10 PID'(SIGUSR1) en el terminal para enviar una señal que cree un nuevo usuario en el facturador normal.\n");
		printf("\tUtilize 'kill -12 PID'(SIGUSR2) en el terminal enviar una señal que cree un nuevo usuario en el facturador VIP.\n");
		printf("\tUtilize 'kill -2 PID'(SIGINT) en el terminal si desea finalizar el programa.\n");

		// Inicio log
		logFile = fopen(logFileName, "w");
		fclose(logFile);
		writeLogMessage("Correcto ", "Log iniciado satisfactoriamente.\n");

		char id[10];
		char msg[100];
		sprintf(id, "MENSAJE ");
		sprintf(msg, "RECIBIDO");
		writeLogMessage(id, msg);
		//Inicialización de la semilla
		srand(gettid());

		/* --------------------------Semaforos y condiciones--------------------------*/

		pthread_mutex_init(&facturacion, NULL);
		pthread_mutex_init(&loger, NULL);
		pthread_mutex_init(&ctrl_seguridad, NULL);
		pthread_mutex_init(&usuarios, NULL);
		pthread_mutex_init(&global, NULL);
		pthread_mutex_init(&segurata, NULL);

		pthread_cond_init(&seguridad, NULL);
		pthread_cond_init(&apto, NULL);


		if(argc == 3){ // PARTE OPCIONAL, NO TOCAR 

		// Asignacion dinamica de memoria en la cola de usuarios

		 int argUsuarios = atoi(argv[1]);
		 int argFacturadores = atoi(argv[2]);

		 usuario_ = (struct usuario *) malloc (sizeof(struct usuario)*argUsuarios);
		 facturador_ = (struct facturador *)malloc (sizeof(struct facturador)*argFacturadores);

		 tamCola = argUsuarios;
		 tamFacturado = argFacturadores;

		//logs


		} else if (argc == 1){

			usuario_ = (struct usuario *)malloc (sizeof(struct usuario)*PASAJEROS);
			facturador_ = (struct facturador *)malloc (sizeof(struct facturador)*FACTURADORES);
			tamCola = 10;
			tamFacturado = 2;
			printf("Se van a tomar los valores por defecto.\n");

			//log de valores por defecto, el printf es para que lo veamos nosotrs mas rápido




		}else{

			printf("ERROR - > Numero de argumentos: %d\n",argc);
			return -1;
		}

			if(signal(SIGUSR1,nuevoUsuario) == SIG_ERR){ perror("\nError señal SIGUSR1 en nuevoUsuario\n"); 	exit(-1); }
			if(signal(SIGUSR2,nuevoUsuario) == SIG_ERR){ perror("\nError señal SIGUSR2 en nuevoUsuario \n"); 	exit(-1); }
			if(signal(SIGINT,finalizar)     == SIG_ERR){ perror("\nError al invocar SIGINT\n"); 	            exit(-1); }


	

	/*--------------------------INICIALIZACIÓN--------------------------*/
	
	 contadorUsuario = 0;
	 finalizarPrograma = 0;
	 UsuarioEnControl = 0;
	 usuariosAtendidosTotal = 0;

	 int i;
	 for (i = 0; i < tamCola; i++) {	
	 	/* Inicialización */
	 	sacarCola(i);
	 }

	  for (i = 1; i <= tamFacturado; i++) {
	 	/* code */
	 	facturador_[i].id = i;
	 	pthread_create(&facturador_[i].hiloFacturador,NULL,accionesFacturador,(void *)&facturador_[i].id); 
	 }

	 pthread_t agente;
	 pthread_create(&agente,NULL,agenteSeguridad,NULL); 


	 // RECEPCION DE PROGRAMAS
	 //log

	 printf("En ejecucion\n");
	 while(getFinalizarPrograma() == 0){
	 	printf("\t\t\t\t...OK...\n\n");
	 	pause();
	 }

		char id2 [21];
		char msg2[100];
		sprintf(id2,"Avión:");
		sprintf(msg2, "Cerrando puertas y despegando\n");
		writeLogMessage(id2, msg2);
		return 0;
	}



   //////////////////////////////////////////////////////////////////////////////////////////
  //					FUNCIONES					  //
 //////////////////////////////////////////////////////////////////////////////////////////


 /*******************************************************************************
 * 				FUNCIÓN  GETTID 				*
 * 										*
 *  Función que no está implementada en la biblioteca.				*
 *******************************************************************************/

	pid_t gettid(void){
		
		return syscall(__NR_gettid);
	}


 /*******************************************************************************
 * 				FUNCIÓN WRITELOGMESSAGE 			*
 * 										*
 *  Función que escribe mensajes en el log.					*
 *******************************************************************************/

	void writeLogMessage(char *id, char *msg) {

		//Inicio del mutex de Log
		pthread_mutex_lock(&loger);
		// Calculamos la hora actual
		time_t now = time(0);
		struct tm *tlocal = localtime(&now);
		char stnow [19];
		 strftime(stnow, 19, "%d/%m/%y %H:%M:%S", tlocal);

		// Escribimos en el log
		logFile = fopen(logFileName , "a");
		fprintf(logFile, "[%s] %s: %s\n", stnow, id, msg);
		fclose(logFile);

		//Finalización del mutex de Log
		pthread_mutex_unlock(&loger);
	 }


/*******************************************************************************
 *				FUNCIÓN CALCULAALEATORIOS			*
 * 										*
 *  Función que calcula un número aleatorio entre 2 números que se le 		*
 *   pasan por argumento (uno mínimo y otro máximo)				*
 *******************************************************************************/

	int calculaAleatorios(int min, int max){	

	    return rand() % (max-min+1) + min;	
	}


 /*******************************************************************************
 * 				FUNCIÓN Finalizar			        *
 * 										*
 *  Recoge el fin del programa. Comprueba si todos los hilos han terminado, en  *
 *   caso de que esto no suceda, se espera hasta su finalización		*
 *******************************************************************************/

	void finalizar(int sig){

		printf("Señal finalizar\n");

		char id2 [21];
		char msg2[100];
		sprintf(id2,"Vuelo completo");
		sprintf(msg2, "Esperando por la facturacion de todos los usuarios\n");
		writeLogMessage(id2, msg2);

		/* Cuando entramos en finalizar tratamos aqui todas las señales, ya que no queremos nuevos usuarios */
		if(signal(SIGUSR1,finalizar) == SIG_ERR){ perror("\nError señal SIGUSR1 en competidor\n"); 	exit(-1); }
		if(signal(SIGUSR2,finalizar) == SIG_ERR){ perror("\nError señal SIGUSR2 en competidor\n"); 	exit(-1); }

		int bandera = 0,  i = 0;
		if(sig == SIGUSR2 || sig== SIGUSR1){ // Trato si me llegan más señales
		}else if(sig == SIGINT){
			printf("Ya no entran más usuarios. Vuelo completo y apunto de despegar.\n\n");
			// BUSQUEDA.	
					int contador = 0, comprobar = 0;
					while(comprobar == 0){
						
						contador = 0;
						for(i = 0;i < tamCola; i++){
							if(usuario_[i].id == 0 && usuario_[i].ha_facturado == 0 && usuario_[i].atendido == 0 && usuario_[i].facturador==0){
								++contador;
								if(contador == tamCola){
									comprobar=1;
								}
	
							}
						}
							
						sleep(1);

					}					
				
			}
							pthread_mutex_lock(&global);
							finalizarPrograma = 1;
							pthread_mutex_unlock(&global);

							sleep(1);
							sprintf(id2,"Fin del programa");
							sprintf(msg2, "Con los siguientes resultados\n");
							writeLogMessage(id2, msg2);
		
							
	}


 /*******************************************************************************
 * 				FUNCIÓN ACCIONES USUARIO			*
 * 										*
 *  Contiene todo el funcionamiento desde el inicio hasta el fin del hilo. 	*
 *******************************************************************************/

	void *accionesUsuario(void *idUsuario){
	
		int id,pos,ir;
		id = *(int *) idUsuario;
		pos = posicion(id);
		

		/*LOG*/		
		char iden [21];
		char msg[100];
		sprintf(iden,"Usuario_%d",id);
		sprintf(msg, "entra a la cola de facturacion."); ;
		writeLogMessage(iden,msg); 	
		sleep(4);


		while(usuario_[pos].atendido == 0){

			ir = calculaAleatorios(1,100);
			if(ir <= 20){
				//guardo en el log q quiere irse
				//libero el espacio en cola

				char iden [21];
				char msg[100];
				sprintf(iden,"Usuario_%d",id);
				sprintf(msg, "se cansa y abandona la cola");
				writeLogMessage(iden,msg); 

				sacarCola(pos);
				//doy fin al hilo
				pthread_exit(NULL);
			
			}else if(ir >= 20  && ir < 30){
				//guardo en el log q quiere irse al baño
					/*LOG*/		
				char iden [21];
				char msg[100];
				sprintf(iden,"Usuario_%d",id);
				sprintf(msg, "se va al baño y abandona la cola."); ;
				writeLogMessage(iden,msg); 

				//libero el espacio en cola
				sacarCola(pos);
			
				pthread_exit(NULL);
			}else{
				char iden [21];
				char msg[100];
				sprintf(iden,"Usuario_%d",id);
				sprintf(msg, "esperando ha ser atendido."); ;
				writeLogMessage(iden,msg); 
				sleep(3);

			}				

		}

		sprintf(iden,"Usuario_%d",id);
		sprintf(msg, "atendido, pasando a facturación."); ;
		writeLogMessage(iden,msg); 
	
		while(usuario_[pos].atendido == 1){
			char iden [21];
			char msg[100];
			sprintf(iden,"Usuario_%d",id);
			sprintf(msg, "Esta facturando..."); ;
			writeLogMessage(iden,msg);
			sleep(1);
		}
		if(usuario_[pos].ha_facturado == 1 && usuario_[pos].atendido == 2){
			
			char iden [21];
			char msg[100];
			sprintf(iden,"Usuario_%d",id);
			sprintf(msg, "esta esperando para el control de seguridad."); 
			writeLogMessage(iden,msg);

			int local = usuario_[pos].id;

			pthread_mutex_lock(&segurata);
			sleep(1);
			pthread_mutex_lock(&ctrl_seguridad);
			pthread_cond_signal(&seguridad);
			UsuarioEnControl=local;

			pthread_cond_wait(&apto,&ctrl_seguridad);

			sprintf(iden,"Usuario_%d",id);
			sprintf(msg, "pasa la seguridad y coge el vuelo."); ;
			writeLogMessage(iden,msg); 
			
			pthread_mutex_unlock(&ctrl_seguridad);
			sleep(1);
			pthread_mutex_unlock(&segurata);
			
			
			
		}else{
			//guardo en el log q no ha podido facturar
			char iden [21];
			char msg[100];
			sprintf(iden,"Usuario_%d",id);
			sprintf(msg, "No ha podido facturar y no coge el vuelo"); ;
			writeLogMessage(iden,msg); 
			
		}

		//libero el espacio en cola
		sacarCola(pos);
		pthread_exit(NULL);
	}


 /*******************************************************************************
 *			FUNCION PARA SACAR LA POSICION 				*
 * 										*
 *  Funcion que calcula la posicion de un determinado usuario 			*
 *******************************************************************************/

	int posicion(int id){//funcion que calcula la posicion de un determinado usuario
		int i, posicion;//declaracion de variables	
			for(i=0;i<tamCola;i++){//bucle for que me recorre el puntero
				if(usuario_[i].id == id){//si el id del usuario en el que estoy es igual al id que busco
					posicion=i;//la posicion es igual a i 
				}
			}
		return posicion;//devuelvo la posicion
	}


 /*******************************************************************************
 *				FUNCION AGENTE SEGURIDAD 	 		*
 * 										*
 *******************************************************************************/

	void *agenteSeguridad(void *idUsuario){
	
		while(1){ 
			
			pthread_mutex_lock(&ctrl_seguridad);
			pthread_cond_wait(&seguridad,&ctrl_seguridad);
			int inspeccion,espera;

			inspeccion=calculaAleatorios(1,100);
	
			if(inspeccion<=40){
				char iden [100];
				char msg [100];
				sprintf(iden,"Usuario_%d",UsuarioEnControl);
				sprintf(msg, "Pasa a la inspeccion en el control de seguridad."); 
				writeLogMessage(iden,msg); 
				espera=calculaAleatorios(10,15);
				sleep(espera);
			}
			else{
				char iden [100];
				char msg [100];
				sprintf(iden,"Usuario_%d",UsuarioEnControl);
				sprintf(msg, "Pasa el control sin inspeccion."); 
				writeLogMessage(iden,msg); 
				espera=calculaAleatorios(1,2);
				sleep(espera);
			}
			pthread_cond_signal(&apto);
			pthread_mutex_unlock(&ctrl_seguridad);			
		}
		if(getFinalizarPrograma() == 0){
			pthread_exit(NULL);
		}	
	}
	

 /*******************************************************************************
 * 			FUNCIÓN ACCIONES FACTURADOR 		  		*
 * 										*
 *  Función que contiene todo el funcionamiento del facturador.			*
 *******************************************************************************/

	void *accionesFacturador(void *ptr){

	 	//Espera para la inicialiacion de los hilos 
		sleep(10);
		int fID = *(int *)ptr;
		int usuarioAtender = 0, usuariosAtendidos = 0,facturando = 0, i = 0, tipoFacturacion=0, TEspera = 0, motivoFinAtencion = 0, tEsperaUsuario = 0;
			char facturador [100];
			sprintf(facturador,"facturador_%d",fID);
			writeLogMessage(facturador,"Esta listo para atender"); 
		
		//Mientras el programa no se finalice, los dos hilos se quedan esperando a usuarios de vuelo 
		while(getFinalizarPrograma() == 0){
		
			// Si el hilo esta ocupado, no entra
			while(facturando == 0){
				// Nos da un usuario vip, con el menor tiempo, o un usuario normal
				pthread_mutex_lock(&usuarios);	//Bloqueamos el acceso
				usuarioAtender = buscoUsuario(fID); 
				//Bloqueo el acceso
				
				if (usuarioAtender >= 0 && usuario_[usuarioAtender].atendido == 0){	//SI el usuario coincide y ha llegado a la cola de facturador
					facturando=1;
					pthread_mutex_unlock(&usuarios);
				
				}else{		// SI no hay usuarios, duerme 1 segundos
					pthread_mutex_unlock(&usuarios);							
					sleep(1);
				}

																	  							
				} 

				//Usuario atendido = 1 entra al bucle de facturando, y no sale hasta que se sepa si pasará el control o no de seguridad
			usuario_[usuarioAtender].atendido = 1;	

			tipoFacturacion=calculaAleatorios(1,100);

			if(tipoFacturacion<=10){

				char tiempo [100];
				char id [100];
				sprintf(id,"Usuario_%d",usuario_[usuarioAtender].id);
				sprintf(tiempo,"ha pasado la facturacion con exceso de peso.");
				writeLogMessage(id,tiempo);
				pthread_mutex_lock(&usuarios);

					usuario_[usuarioAtender].ha_facturado = 1;
					usuariosAtendidos=usuariosAtendidos+1;
					usuariosAtendidosTotal=usuariosAtendidosTotal+1;

				pthread_mutex_unlock(&usuarios);
				//El usuario ha facturado con exceso de peso
				TEspera=calculaAleatorios(2,6);
				//Almacenamos si hay exceso de peso en el log
				
				
			}

			else if(tipoFacturacion<=20){

				//Almacenamos la situación del visado en el log
				char tiempo [100];
				char id [100];
				sprintf(id,"Usuario_%d",usuario_[usuarioAtender].id);
				sprintf(tiempo,"No tiene el visado en regla.");				
				writeLogMessage(id,tiempo);

				pthread_mutex_lock(&usuarios);
				usuario_[usuarioAtender].ha_facturado=0;
				usuariosAtendidos=usuariosAtendidos+1;
				usuariosAtendidosTotal=usuariosAtendidosTotal+1;
				pthread_mutex_unlock(&usuarios);
				//El usuario no tiene el visado en regla
				TEspera=calculaAleatorios(6,10);
				
				
			}	

			else{
				char tiempo [100];
				char id [100];
				sprintf(id,"Usuario_%d",usuario_[usuarioAtender].id);
				sprintf(tiempo,"ha pasado la facturacion sin problema.");
				writeLogMessage(id,tiempo);

				pthread_mutex_lock(&usuarios);
				usuario_[usuarioAtender].ha_facturado=1;
				usuariosAtendidos=usuariosAtendidos+1;
				usuariosAtendidosTotal=usuariosAtendidosTotal+1;
				pthread_mutex_unlock(&usuarios);
				//el usuario ha facturado sin problema
				TEspera=calculaAleatorios(1,4);
				//Almacenamos el tiempo de facturación en el log
				
			}
			sleep(TEspera);	

			pthread_mutex_lock(&usuarios);
			usuario_[usuarioAtender].atendido=2;
			pthread_mutex_unlock(&usuarios);	
			
			//Almaceno en el log la hora de fin de atencion 	
			char tiempo [100];
			char id [100];
			sprintf(id,"Usuario_%d",usuario_[usuarioAtender].id);
			sprintf(tiempo,"ha finalizado la facturacion.");
			writeLogMessage(id,tiempo);
			facturando = 0;
			usuarioAtender=-2;
			
			if(usuariosAtendidos % 5 == 0){

				//almaceno en el logque el facturador se va a tomar cafe
				char iden [21];
				char msg[100];
				sprintf(iden,"Facturador_[%d]",fID);
				sprintf(msg, "esta yendo a tomar café."); 
				writeLogMessage(iden,msg); 
					sleep(10);
				sprintf(iden,"Facturador_[%d]",fID);
				sprintf(msg, "ha finalizado de tomar café."); 
				writeLogMessage(iden,msg); 
					
					
				
			}
		}// Volvemos a buscar un usuario mientras el programa no finalice
		char iden [21];
		char msg[100];
		sprintf(iden,"Facturador_[%d]",fID);
		sprintf(msg, "cerrando Check-In."); 
		writeLogMessage(iden,msg); 
		pthread_exit(NULL);
	}
 

 /*******************************************************************************
 * 				FUNCIÓN NUEVO USUARIO				*
 * 										*
 *  Creacion de todos los hilos pertenecientes al programa, excepto los		*
 *   facturadores.								*
 * 	1º Recorremos todo el array, para ver si está o no creado.		*
 *	2º Donde este vacio creamos.						*
 *******************************************************************************/

	void nuevoUsuario(int sig){ 

		int i = 0 ,j = 0, contador = 0, bandera = 0 ;
		printf("Señal recibida\n");

		 /* ----- Inicio del semáforo ----- */
		pthread_mutex_lock(&usuarios);
		// Igual mejor ponerlo ariba, ya que si está llena, se salta el buscar una posición
		for(i=0;i < tamCola;i++){
			if(usuario_[i].id != 0){
				contador++;
			}
		}
		 
		i = 0;
		if(contador < tamCola){ // Si hay algun sitio, entro
					//log de cola llena		
			while(i < tamCola && bandera == 0){ // Bandera: Para que si esta creado salga directamente 
			
				if(usuario_[i].id == 0){ // No entra si las posiciones estan recogidas
					// Entramos en la cola de facturacion
					pthread_mutex_lock(&global);
					/* Lo demás esta ya inicializado y a 0 */
					usuario_[i].id = ++usuariosAtendidosTotal;
					pthread_mutex_unlock(&global);
					/*  ASIGNACIÓN DE RECURSOS SEGUN LA SEÑAL*/
					switch(sig){

						case SIGUSR1:	/* FACTURADOR 1 NORMAL */
					  		usuario_[i].facturador = 1;
					  	 	break;

						case SIGUSR2:	/* FACTURADOR 2 VIP */
					  	 	usuario_[i].facturador = 2;
					  	 	break;
					}
					pthread_create(&usuario_[i].hiloUsuario,NULL,accionesUsuario,(void *)&usuario_[i].id);
					bandera = 1;
				}
				i++;
			}
			
		}
		 /* ----- Fin del semáforo ----- */
		pthread_mutex_unlock(&usuarios);
	
	}


 /*******************************************************************************
 * 				FUNCIÓN SACAR BOLA				*
 * 										*
 *******************************************************************************/

	void sacarCola(int pos){

 		/* ----- Inicio del semáforo ----- */
		pthread_mutex_lock(&usuarios);
			
		usuario_[pos].id=0;
		usuario_[pos].atendido=0;
		usuario_[pos].ha_facturado=0;
		usuario_[pos].facturador=0;
		usuario_[pos].salir=0;

		--contadorUsuario;
		 /* ----- Fin del semáforo ----- */
		pthread_mutex_unlock(&usuarios);
	}


 /*******************************************************************************
 * 				FUNCIÓN GET ATENDIDO				*
 * 										*
 *******************************************************************************/

	int getAtendido(int pos){

		/* ----- Inicio del semáforo ----- */
		pthread_mutex_lock(&usuarios);	

		if(usuario_[pos].atendido == 0){
			return 0;

		}else if(usuario_[pos].atendido == 1){
			return 1;

		}else{
			return 2;
		}
		 /* ----- Fin del semáforo ----- */
		pthread_mutex_unlock(&usuarios);
	}

/*******************************************************************************
 * 				FUNCIÓN GET ATENDIDO				*
 * 										*
 *******************************************************************************/

	int get(int pos){

		/* ----- Inicio del semáforo ----- */
		pthread_mutex_lock(&usuarios);	

		if(usuario_[pos].atendido == 0){
			return 0;

		}else if(usuario_[pos].atendido == 1){
			return 1;

		}else{
			return 2;
		}
		 /* ----- Fin del semáforo ----- */
		pthread_mutex_unlock(&usuarios);
	}



 /*******************************************************************************
 * 				FUNCIÓN SET ATENDIDO				*
 * 										*
 *******************************************************************************/

	void setAtendido(int pos, int i){

		/* ----- Inicio del semáforo ----- */
		pthread_mutex_lock(&usuarios);	
		usuario_[pos].atendido =i;
		 /* ----- Fin del semáforo ----- */
		pthread_mutex_unlock(&usuarios);
	}


 /*******************************************************************************
 * 				FUNCIÓN BUSCAR USUARIO 				*
 * 										*
 *  Función que busca si hay usuarios en la cola y devuelve al que le toca	*
 *   facturar.									*
 *******************************************************************************/

	int buscoUsuario(int tipoFacturador){

		int usuarioAtender = -2;
		//Compruebo si en esta cola hay usuarios VIPs para atender
		usuarioAtender = buscaUsuarioTipo(tipoFacturador);
		//Si no hay usuarios VIPs para antender
		if(usuarioAtender == -2){
			//Como no hay VIPs puedo atender a otros usuarios
			usuarioAtender = mayorTiempoEspera();
		}	
		

		return usuarioAtender;
	}


 /*******************************************************************************
 * 			FUNCIÓN BUSCAR USUARIO Tipo 				*
 * 										*
 *  Función que busca si un usuario es VIP o no según el Flag de facturador.	*
 *******************************************************************************/

	int buscaUsuarioTipo(int tipoFacturador){

		int pos = -2, i = 0, bandera = 0;

		while(i < tamCola && bandera == 0){

			if(usuario_[i].facturador == tipoFacturador && usuario_[i].atendido == 0 && usuario_[i].id > 0){
				pos=i;
				bandera = 1;
			}
			i++;
		}
		return pos;
	}


 /*******************************************************************************
 * 			FUNCIÓN MAYOR TIEMPO ESPERA 		  		*
 * 										*
 *  Función que calcula cual es el usuario que lleva mas tiempo en la cola. 	*
 *   Como estan numerados, el de menor ID sera el que entro primero.		*
 *******************************************************************************/

	int mayorTiempoEspera(void){

		int tEspera = 0, menor = 0, posMenor = -2, i = 0;
		//El que tenga el menor ID sera el que lleve mas tiempo esperando en la cola

		while(i < tamCola){
			if(i==0&&usuario_[i].id>0&& usuario_[i].atendido==0){
				posMenor=i;
				menor=usuario_[i].id;
			}
			else if(usuario_[i].id>0 && usuario_[i].atendido==0){
				if (menor>usuario_[i].id){					
					posMenor=posicion(usuario_[i].id);
					menor=usuario_[i].id;
				}		
			}
			i++;
		}
		return posMenor;
	}


 /*******************************************************************************
 * 			FUNCIÓN GET CONTADOR USUARIO 		  		*
 *										*
 *******************************************************************************/
	
	int getContadorUsuario(){

		pthread_mutex_lock(&global);
		int valor = contadorUsuario;
		pthread_mutex_unlock(&global);
		return valor;
	}

 /*******************************************************************************
 * 			FUNCIÓN GET FINALIZAR PROGRAMA		  		*
 *										*
 *******************************************************************************/
	int getFinalizarPrograma(){
		pthread_mutex_lock(&global);
		int valor = finalizarPrograma;
		pthread_mutex_unlock(&global);
		return finalizarPrograma;
	}
 /*******************************************************************************
 * 			FUNCIÓN GET ID		  		*
 *										*
 *******************************************************************************/
	int getID(int i){
		/* ----- Inicio del semáforo ----- */
		pthread_mutex_lock(&usuarios);	
		int m = usuario_[i].id;
		pthread_mutex_unlock(&usuarios);
		return m;
	}