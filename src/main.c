/*
 * main.c
 *
 *  Created on: 4/4/2019
 *      Author: TitO
 *
 *  Caracteristicas:
 *  	- Cambio de contexto por PendSV.
 *  	- Vector de tareas implementado.
 *  	- Código de OS separado del main.
 *  	- Vector de prioridades implementado.
 *
 *  Limitaciones en esta versión:
 *  	- Las tareas no se pueden eliminar.
 *  	- El stack de las tareas es el mismo para todas porque esta definido en el vector
 *  	y por ahora no se utiliza el parametro pasado al crear la tarea.
 *  	- Todavía no se implementa sección crítica en el schedule.
 *
 */

/*==================[inclusions]=============================================*/

#include "main.h"
#include "os.h"

#include <stdint.h>
#include <stdlib.h>

/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

void * task1( void * arg);
void * task2( void * arg);

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

void * task1( void * arg)
{
	while(1){
		Board_LED_Toggle(LED_1);
		task_delay( 1000 );
	}
}

void * task2( void * arg)
{
	while(1){
		Board_LED_Toggle(LED_2);
		task_delay( 2000 );
	}
}


/*==================[external functions definition]==========================*/

int main(void)
{
	osTaskCreate(task1, STACK_SIZE_B, ePrioMAX, (void*) 0x11223344);
	osTaskCreate(task2, STACK_SIZE_B, ePrioMAX, (void*) 0x11223344);

	/*
	 * La inicializacion del OS debe ser posterior a la declaracion de tareas
	 */
	initOS();

	while (1)
	{
		__WFI();
	}
}

/*==================[end of file]============================================*/
