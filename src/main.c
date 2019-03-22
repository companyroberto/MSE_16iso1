/* Copyright 2016, Pablo Ridolfi
 * All rights reserved.
 *
 * This file is part of Workspace.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */
 

/*==================[inclusions]=============================================*/

#include "main.h"
#include "board.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*==================[macros and definitions]=================================*/

#define STACK_SIZE_B 512

typedef void * ( * task_type)(void *);

/*==================[internal data declaration]==============================*/


/*==================[internal functions declaration]=========================*/

/** @brief hardware initialization function
 *	@return none
 */
static void initHardware(void);

void task_return_hook( void * ret_val);
void init_stack(uint32_t stack[], uint32_t stack_size, uint32_t * sp, task_type entry_point, void * arg);
uint32_t get_next_context(uint32_t current_sp);
void task_delay( uint32_t systick);

void * task1( void * arg);
void * task2( void * arg);
void * task3( void * arg);


/*==================[internal data definition]===============================*/

uint32_t current_task = 0;

uint32_t sp1, sp2, sp3;
uint32_t stack1[STACK_SIZE_B / 4];
uint32_t task1_delay = 0;
uint32_t stack2[STACK_SIZE_B / 4];
uint32_t task2_delay = 0;
uint32_t stack3[STACK_SIZE_B / 4];
uint32_t task3_delay = 0;

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

static void initHardware(void)
{
	Board_Init();
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / 1000);
}

void task_return_hook( void * ret_val)
{
	while(1){
		__WFI();
	}
}

void init_stack(uint32_t stack[], uint32_t stack_size_b, uint32_t * sp, task_type entry_point, void * arg)
{

	bzero(stack, stack_size_b);

	stack[stack_size_b / 4 - 1] = 1 << 24;							/* xPSR.T = 1 		*/ // Posición 1
	stack[stack_size_b / 4 - 2] = (uint32_t) entry_point;			/* Program Counter	*/ // Posición 2
	stack[stack_size_b / 4 - 3] = (uint32_t) task_return_hook;		/* LR				*/ // Apunta a la dirección de retorno
	// Si pongo NULL es como FreeRTOS en donde no retorna valor. Si pongo 0 da error porque no es una dirección válida ya que todas terminan en 1.
	stack[stack_size_b / 4 - 8] = (uint32_t) arg;					/* R0				*/ // Para salvar el argunmento

	// Ahora en el systisk llamo a otra funcion entonces tengo que guardar espacio en la pila para ese Link Register
	stack[stack_size_b / 4 - 9] = 0xFFFFFFF9;						/* LR IRO			*/

	//*sp = (uint32_t)&(stack[stack_size_b / 4 - 8]);				// Es es la base de la pila, pero faltan los otros 8 registros
	//*sp = (uint32_t)&(stack[stack_size_b / 4 - 16]);				// aca considero los otros 8 registros

	//
	*sp = (uint32_t) &(stack[stack_size_b / 4 - 17]);				// ahora tengo que sumar uno mas al stack por el LR IRO
}

uint32_t get_next_context(uint32_t current_sp)
{
	uint32_t next_sp;

	// Política Round-Robin para recorrer las tareas. La 0 es el main y la 3 es la idle.

	switch (current_task) {
	case 0:
		/* despues vere que hacer con el contexto del main 		*/
		/* ? = current_sp										*/
		next_sp = sp1;
		current_task = 1;
		break;
	case 1:
		sp1 = current_sp;

		if ( task2_delay == 0 ){		// Si la tarea 2 no está en delay, se le pasa el puntero.
			next_sp = sp2;
			current_task = 2;
		}
		else{							// Si no paso a la idle
			next_sp = sp3;
			current_task = 3;
		}

		break;
	case 2:
		sp2 = current_sp;

		if ( task1_delay == 0 ){		// Si la tarea 1 no está en delay, se le pasa el puntero.
			next_sp = sp1;
			current_task = 1;
		}
		else{							// Si no paso a la idle
			next_sp = sp3;
			current_task = 3;
		}

		break;
	case 3:
		sp3 = current_sp;				// Por defecto guardo para volver a esta tarea idle
		next_sp = sp3;
		current_task = 3;

		if ( task1_delay == 0 ){		// Si la tarea 1 no está en delay, se le pasa el puntero.
			next_sp = sp1;
			current_task = 1;
		}
		else{
			if ( task2_delay == 0 ){	// Si la tarea 2 no está en delay, se le pasa el puntero.
				next_sp = sp2;
				current_task = 2;
			}
		}
		break;
	default:
		while(1){
			__WFI();
		}
	}

	// Si algún delay está configurado, lo decremento
	if ( task1_delay > 0 )
		task1_delay--;

	if ( task2_delay > 0 )
		task2_delay--;

	return next_sp;
}

void task_delay( uint32_t tick)
{
	// En esta tarea se asigna la cantidad de tick para delay de cada tarea

	switch (current_task) {
	case 0:
		/* no se permite la llamada a task_delay con el contexto del main */
		__WFI();
		break;
	case 1:
		task1_delay = tick;
		break;
	case 2:
		task2_delay = tick;
		break;
	default:
		__WFI();
	}
	__WFI();
}

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
	init_stack(stack1, STACK_SIZE_B, &sp1, task1, (void*) 0x11223344);
	init_stack(stack2, STACK_SIZE_B, &sp2, task2, (void*) 0x55667788);
	// Creo una tercer tarea del tipo idle la cual no debe hacer nada
	init_stack(stack3, STACK_SIZE_B, &sp3, task3, (void*) 0x99001122);
	// va antes de la configuracion porque si justo se dispara el systick antes de iniciaalizar las pilas es un problema...

	initHardware();

	while (1)
	{
		__WFI();
	}
}

/*==================[end of file]============================================*/
