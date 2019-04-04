/*
 * os.c
 *
 *  Created on: 4/4/2019
 *      Author: TitO
 */

/*==================[inclusions]=============================================*/

#include "os.h"

#include <string.h>

/*==================[macros and definitions]=================================*/

/* Task states returned by eTaskGetState. */
typedef enum
{
	eRunning = 0,	/* A task is querying the state of itself, so must be running. */
	eReady,			/* The task being queried is in a read or pending ready list. */
	eWaiting,		/* The task being queried is in the Blocked state. */
} eTaskState;

typedef struct xTASK_PARAMETERS
{
	task_type entry_point;
	uint32_t stack_size_b;
	uint32_t tskIDLE_PRIORITY;
	void * arg;
	uint32_t stack[];				// uint32_t stack[STACK_SIZE_B / 4];
	uint32_t * sp;
} TaskParameters_t;

/*==================[internal data declaration]==============================*/

uint32_t current_task = 0;
uint32_t task1_delay = 0;
uint32_t task2_delay = 0;
uint32_t task3_delay = 0;

uint32_t maximas_tareas = 10;		// Maximo numero de tareas a declarar
uint32_t puntero_proxima_tarea = 0;

TaskParameters_t tareas[maximas_tareas];

/*==================[internal functions declaration]=========================*/

/** @brief hardware initialization function
 *	@return none
 */
static void initHardware(void);

void task_return_hook( void * ret_val);

uint32_t get_next_context(uint32_t current_sp);

void schedule( void );

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

static void initHardware(void)
{
	Board_Init();
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / 1000);
	NVIC_SetPriority(PendSV_IRQn, (1 << __NVIC_PRIO_BITS) -1);
}

void task_return_hook( void * ret_val)
{
	while(1){
		__WFI();
	}
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

void SysTick_Handler( void )
{
	//Board_LED_Toggle(LED_3);
	schedule();
}

void schedule( void )
{
	// Aseguramos que se ejecuten todas las interrupciones en el pipeline
	__ISB();
	// Aseguramos que se completen todos los accesos a memoria
	__DSB();

	// Activo PendSV para llevar a cabo el cambio de contexto
	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

/*==================[external functions definition]==========================*/

void initOS(void)
{
	initHardware();
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
	// El 0xFFFFFFF9 es un numero magino que puede ser 0xFFFFFFF9 / 0xFFFFFFF0 / 0xFFFFFFF1 creo... tiene que ver con la interrupción

	//*sp = (uint32_t)&(stack[stack_size_b / 4 - 8]);				// Es es la base de la pila, pero faltan los otros 8 registros
	//*sp = (uint32_t)&(stack[stack_size_b / 4 - 16]);				// aca considero los otros 8 registros

	//
	*sp = (uint32_t) &(stack[stack_size_b / 4 - 17]);				// ahora tengo que sumar uno mas al stack por el LR IRO
}

void osTaskCreate(task_type entry_point, uint32_t stack_size_b, uint32_t tskIDLE_PRIORITY, void * arg)
{
	tareas[puntero_proxima_tarea].entry_point      = entry_point;
	tareas[puntero_proxima_tarea].stack_size_b     = stack_size_b;
	tareas[puntero_proxima_tarea].tskIDLE_PRIORITY = tskIDLE_PRIORITY;
	tareas[puntero_proxima_tarea].arg              = arg;

	uint32_t stack[STACK_SIZE_B / 4];
	tareas[puntero_proxima_tarea].stack            = stack;

	init_stack(stack1, stack_size_b, &sp1, entry_point, arg);
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

/*==================[end of file]============================================*/
