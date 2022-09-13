
#################################################################################
#																				#
#							   PRÁCTICA INTERMEDIA   							#
#						   SIMULACIÓN CARRERA DE KARTS							#
#																				#	
#	________________________________________________________________________	#
#																				#
#	OBJETIVOS:																	#
#		-Manejos de procesos 													#
#		-Mecanismos de sincronización entre procesos							#
#																				#
#	ENUNCIADO PRÁCTICA:															#
#																				#
#		2º PARTE: SCRIPT SHELL 													#
#			- Presenta un menú con 4 opciones:									#
#				* 1º OPCIÓN- El SCRIPT muestra el código del programa mediante	#
#							  el uso del comando cat (cat programa.c)			#
#							  (cat PracticaIntermedia.c)						#
#				* 2º OPCIÓN- Se lanzará la compilación del archivo.c en que 	#
#							  entregue el programa (gcc programa.c -o programa)	#
#							  (gcc PracticaIntermedia.c -o PracticaIntermedia)	#
#				* 3º OPCIÓN- Se ejecutará el programa, siempre que exista el 	#
#							  el ejecutable y tenga permisos de ejecución.		#
#							  Para proceder a la ejecución, se pedirá el numero #
#							  de corredores que participan en la carrera y que 	#
#							  luego se pasan como argumentos al programa.		#
#				* 4º OPCIÓN- Salida del SCRIPT 									#
#																				#
#################################################################################


#!/bin/bash

#Funcion que muestra el codigo del programa
function codigo(){
	cat PracticaIntermedia.c	
}

#Funcion que compila el programa
function compila(){
	chmod 777 PracticaIntermedia.c
	gcc -o PracticaIntermedia PracticaIntermedia.c
}

#Funcion que ejecuta el programa
function ejecuta(){
	read entrada
	./PracticaIntermedia $entrada
}

while true
	do
		echo -e "\n 	MENÚ.\nSelecciona una opción: "
		echo -e "1. Código del Programa."
		echo -e "2. Compilar el Programa."
		echo -e "3. Ejecutar el Programa."
		echo -e "4. Salir."

		echo -e "Opcion: " 
		read opcion
		case $opcion in

    		1) echo -e "Código del Programa."
				codigo
				;;
			2) echo -e "Compilar el Programa.\n"
				compila
				;;
			3) echo -e "Ejecutar el Programa."
				echo -e "\nIntroduce un número de corredores: "	
				ejecuta;;
			4) echo -e "\nHasta la próxima!\n"
				break
       		 ;; # Abortamos la ejecución del bucle while
    		*) echo -e "Opción no válida"
      		 ;;
 		esac
	done
exit
# Fin del script