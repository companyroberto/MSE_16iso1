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


/*==================[internal data definition]===============================*/

uint32_t current_task = 0;

uint32_t sp1, sp2;
uint32_t stack1[STACK_SIZE_B / 4];
uint32_t stack2[STACK_SIZE_B / 4];

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

	stack[stack_size_b / 4-1] = 1 << 24;							/* xPSR.T = 1 		*/ // Posici�n 1
	stack[stack_size_b / 4-2] = (uint32_t) entry_point;				/* Program Counter	*/ // Posici�n 2
	stack[stack_size_b / 4-3] = (uint32_t) task_return_hook;		/* LR				*/ // Apunta a la direcci�n de retorno
	// Si pongo NULL es como FreeRTOS en donde no retorna valor. Si pongo 0 da error porque no es una direcci�n v�lida ya que todas terminan en 1.
	stack[stack_size_b / 4-8] = (uint32_t) arg;						/* R0				*/ // Para salvar el argunmento

	// Ahora en el systisk llamo a otra funcion entonces tengo que guardar espacio en la pila para ese Link Register
	stack[stack_size_b / 4-9] = 0xFFFFFFF9;							/* LR IRO			*/

	//*sp = (uint32_t)&(stack[stack_size_b / 4-8]);					// Es es la base de la pila, pero faltan los otros 8 registros
	//*sp = (uint32_t)&(stack[stack_size_b / 4-16]);				// aca considero los otros 8 registros

	//
	*sp = (uint32_t)&(stack[stack_size_b / 4-17]);					// ahora tengo que sumar uno mas al stack
}

uint32_t get_next_context(uint32_t current_sp)
{
	uint32_t next_sp;

	switch (current_task) {
	case 0:
		/* despues vere que hacer con el contexto del main 		*/
		/* ? = current_sp										*/
		current_task = 1;
		next_sp = sp1;
		break;
	case 1:
		sp1 = current_sp;
		next_sp = sp2;
		current_task = 2;
		break;
	case 2:
		sp2 = current_sp;
		next_sp = sp1;
		current_task = 1;
		break;
	default:
		__WFI();
	}

	return next_sp;
}

void task_delay( uint32_t systick)
{

}

void * task1( void * arg)
{
	while(1){
		uint32_t tmp = 0;
		for (tmp = 0; tmp < 10000000; ++tmp)	// max 4294967296
		{
			// esperando
		}
		Board_LED_Toggle(LED_1);
	}
}

void * task2( void * arg)
{
	while(1){
		uint32_t tmp = 0;
		for (tmp = 0; tmp < 5000000; ++tmp)		// max 4294967296
		{
			// esperando
		}
		Board_LED_Toggle(LED_2);
	}
}


/*==================[external functions definition]==========================*/

int main(void)
{
	init_stack(stack1, STACK_SIZE_B, &sp1, task1, (void*) 0x11223344);
	init_stack(stack2, STACK_SIZE_B, &sp2, task2, (void*) 0x55667788);
	// va antes de la configuracion porque si justo se dispara el systick antes de iniciaalizar las pilas es un problema...

	initHardware();

	while (1)
	{
		__WFI();
	}
}

/*==================[end of file]============================================*/
