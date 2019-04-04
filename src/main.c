/*
 * main.c
 *
 *  Created on: 4/4/2019
 *      Author: TitO
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
void * task3( void * arg);

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

void * task1( void * arg)
{
	while(1){
		task_delay( 1000 );
		Board_LED_Toggle(LED_1);
	}
}

void * task2( void * arg)
{
	while(1){
		task_delay( 2000 );
		Board_LED_Toggle(LED_2);
	}
}

void * task3( void * arg)
{
	// Creo una tercer tarea del tipo idle la cual no debe hacer nada
	while(1){
		__WFI();
	}
}

/*==================[external functions definition]==========================*/

int main(void)
{
	void osTaskCreate(task1, STACK_SIZE_B, 0, (void*) 0x11223344);
	void osTaskCreate(task2, STACK_SIZE_B, 0, (void*) 0x11223344);
	void osTaskCreate(task3, STACK_SIZE_B, 0, (void*) 0x11223344);

	//init_stack(stack1, STACK_SIZE_B, &sp1, task1, (void*) 0x11223344);
	//init_stack(stack2, STACK_SIZE_B, &sp2, task2, (void*) 0x55667788);

	//* esta tarea se dee crear desde el OS //

	// Creo una tercer tarea del tipo idle la cual no debe hacer nada
	//init_stack(stack3, STACK_SIZE_B, &sp3, task3, (void*) 0x99001122);
	// va antes de la configuracion porque si justo se dispara el systick antes de inicializar las pilas es un problema...

	initOS();

	while (1)
	{
		__WFI();
	}
}

/*==================[end of file]============================================*/
