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

typedef enum {
	eRunning = 0, /* A task is querying the state of itself, so must be running. */
	eReady, /* The task being queried is in a read or pending ready list. */
	eWaiting, /* The task being queried is in the Blocked state. */
} eTaskState;

typedef struct {
	task_type entry_point;
	uint32_t stack_size_b;
	eTaskPrioridades prioridad;
	void * arg;
	uint32_t stack[STACK_SIZE_B / 4]; //uint32_t stack[];
	uint32_t sp;
	eTaskState estado;
	uint32_t delay;
} TaskParameters_t;

typedef enum {
	FROM_ISR = 0, // Llamado desde INT
	NO_FROM_ISR, // Llamado desde DELAY
} eISR;

#define ePrioIDLE	   0
#define sinAsignar	   -1

/*==================[internal data declaration]==============================*/

int32_t puntero_proxima_tarea = 0;
TaskParameters_t tareas[maximas_tareas];

int32_t puntero_proxima_tarea_prioMAX = 0;
int32_t tareasPrioMAX[maximas_tareas];

int32_t puntero_proxima_tarea_prioMED = 0;
int32_t tareasPrioMED[maximas_tareas];

int32_t puntero_proxima_tarea_prioMIN = 0;
int32_t tareasPrioMIN[maximas_tareas];

uint32_t * contexto_current_task_sp = 0;

uint32_t * contexto_next_task_sp = 0;

int32_t current_task = sinAsignar;

TaskParameters_t tareaIDLE;

/*==================[internal functions declaration]=========================*/

static void initHardware(void);

void __attribute__((weak)) task_return_hook(void * ret_val);

void init_stack(uint32_t stack[], uint32_t stack_size, uint32_t * sp,
		task_type entry_point, void * arg);

uint32_t get_next_context(uint32_t current_sp);

void schedule(eISR isr);

void * task_idle(void * arg);

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

static void initHardware(void) {
	Board_Init();
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / 1000);

	// Se le da la menor prioridad de interrupcion.
	NVIC_SetPriority(PendSV_IRQn, (1 << __NVIC_PRIO_BITS) - 1);
}

void __attribute__((weak)) task_return_hook(void * ret_val) {
	while (1) {
		__WFI();
	}
}

void init_stack(uint32_t stack[], uint32_t stack_size_b, uint32_t * sp,
		task_type entry_point, void * arg) {

	bzero(stack, stack_size_b);

	stack[stack_size_b / 4 - 1] = 1 << 24; /* xPSR.T = 1 		*/ // Posición 1
	stack[stack_size_b / 4 - 2] = (uint32_t) entry_point; /* Program Counter	*/ // Posición 2
	stack[stack_size_b / 4 - 3] = (uint32_t) task_return_hook; /* LR				*/ // Apunta a la dirección de retorno
	// Si pongo NULL es como FreeRTOS en donde no retorna valor. Si pongo 0 da error porque no es una dirección válida ya que todas terminan en 1.
	stack[stack_size_b / 4 - 8] = (uint32_t) arg; /* R0				*/ // Para salvar el argunmento

	// Ahora en el systisk llamo a otra funcion entonces tengo que guardar espacio en la pila para ese Link Register
	stack[stack_size_b / 4 - 9] = 0xFFFFFFF9; /* Valor de retorno de excepcion a cargar en el LR.	0xFFFFFFF9 / 0xFFFFFFF0 / 0xFFFFFFF1 */

	//*sp = (uint32_t)&(stack[stack_size_b / 4 - 8]);			// Es es la base de la pila, pero faltan los otros 8 registros
	//*sp = (uint32_t)&(stack[stack_size_b / 4 - 16]);			// aca considero los otros 8 registros

	*sp = (uint32_t) &(stack[stack_size_b / 4 - 17]);// ahora tengo que sumar uno mas al stack por el LR IRO
}

uint32_t get_next_context(uint32_t current_sp) {
	/*
	 * Como el PendSV puede ser interrumpido por INT de mayor prioridad, aca solo cambio a la tarea seleccionada
	 * y entonces si se genera una interrupcion de mayor prioridad no afecta
	 */

	// Guardar contexto de la tarea actual y actualizar estado (el contexto del main no lo guardo)
	if (contexto_current_task_sp != 0)
		*contexto_current_task_sp = current_sp;
	return *contexto_next_task_sp;
}

void SysTick_Handler(void) {
	/*
	 * Se recorren todas las tareas con delay configurado para decrementarlo
	 * Se llama al schedule para reprogramar la próxima tarea
	 */

	int t;
	for (t = 0; t < puntero_proxima_tarea; ++t)
		if (tareas[t].delay > 0)
			if (--tareas[t].delay == 0)
				tareas[t].estado = eReady;

	schedule(FROM_ISR);
}

void schedule(eISR isr) {
	/*
	 * Puede ser llamado desde una interrupción (por SysTick) o no (por Delay) y
	 * este codigo se hace como sección crítica porque es un proceso atomico que
	 * se debe completar aunque se dispare una interrupción.
	 *
	 * En esta función se determina la próxima tarea a ejecutar y luego se
	 * genra la interrupción por soft para cambiar de contexto
	 */

	// Solo se ingresa como NO_FROM_ISR cuando viene desde 'task_delay'
	if (isr == NO_FROM_ISR) {
		// Ingreso a sección crítica, enmascarar interrupciones
		// ...falta el codigo de int...
	}

	// Aseguramos que se ejecuten todas las interrupciones en el pipeline
	__ISB();
	// Aseguramos que se completen todos los accesos a memoria
	__DSB();

	/*
	 * Guardar contexto de la tarea actual y actualizar estado (el contexto del main no lo guardo)
	 */
	if (tareaIDLE.estado == eRunning) {
		contexto_current_task_sp = &tareaIDLE.sp;
		tareaIDLE.estado = eReady;
	} else if (current_task != sinAsignar) { // no es el main
		contexto_current_task_sp = &tareas[current_task].sp;
		switch (tareas[current_task].estado) {
		case eRunning: // Situacion normal
			tareas[current_task].estado = eReady;
			break;
		case eWaiting: // Esperando un Delay
			/*
			 * Esta esperando por otra operación, dicho operación se encarga...
			 */
			break;
		case eReady:
			/*
			 * No debería darse nunca este caso...
			 */
			break;
		default:
			while (1) {
				__WFI();
			}
		}
	}

	/*
	 * Determinar la proxima tarea a ejecutar.
	 * - Limpiar el vector de prioridades
	 * - Cargamos el vector de prioridades con tareas eReady
	 */
	puntero_proxima_tarea_prioMAX = 0;
	puntero_proxima_tarea_prioMED = 0;
	puntero_proxima_tarea_prioMIN = 0;

	int indiceTarea = current_task;
	int t;
	for (t = 0; t < puntero_proxima_tarea; ++t) {

		// Política Round-Robin para recorrer todas las cargadas
		indiceTarea = (indiceTarea + 1) % (puntero_proxima_tarea);

		if (tareas[indiceTarea].estado == eReady) {
			switch (tareas[indiceTarea].prioridad) {
			case ePrioMAX:
				tareasPrioMAX[puntero_proxima_tarea_prioMAX++] = indiceTarea;
				break;
			case ePrioMED:
				tareasPrioMED[puntero_proxima_tarea_prioMED++] = indiceTarea;
				break;
			case ePrioMIN:
				tareasPrioMIN[puntero_proxima_tarea_prioMIN++] = indiceTarea;
				break;
			default:
				while (1) {
					__WFI();
				}
			}
		}
	}

	current_task = sinAsignar;

	// Buscar proxima tarea a ejecutar en este orden hasta que se asigne una
	if (current_task == sinAsignar && puntero_proxima_tarea_prioMAX > 0)
		current_task = tareasPrioMAX[0];

	if (current_task == sinAsignar && puntero_proxima_tarea_prioMED > 0)
		current_task = tareasPrioMED[0];

	if (current_task == sinAsignar && puntero_proxima_tarea_prioMIN > 0)
		current_task = tareasPrioMIN[0];

	// Si no hay tareas para ejecutar, asigno IDLE
	if (current_task == sinAsignar) {
		contexto_next_task_sp = &tareaIDLE.sp;
		tareaIDLE.estado = eRunning;
	} else {
		contexto_next_task_sp = &tareas[current_task].sp;
		tareas[current_task].estado = eRunning;
	}

	// Activo PendSV para llevar a cabo el cambio de contexto
	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;

	if (isr == NO_FROM_ISR) {
		// Salir de sección crítica, habilitar interrupciones
		// ...falta el codigo de int...
	}
}

void * task_idle(void * arg) {
	while (1) {
		__WFI();
	}
}

/*==================[external functions definition]==========================*/

void initOS(void) {
	initHardware();

	/*
	 * Todas las tareas e usuario ya fueron definidas, por lo tanto idle es la ultima
	 */
	tareaIDLE.entry_point = (task_type) task_idle;
	tareaIDLE.stack_size_b = STACK_SIZE_B;
	tareaIDLE.prioridad = ePrioIDLE;
	tareaIDLE.arg = 0;
	tareaIDLE.estado = eReady;

	init_stack(tareaIDLE.stack, tareaIDLE.stack_size_b, &tareaIDLE.sp,
			tareaIDLE.entry_point, tareaIDLE.arg);

	bzero(tareasPrioMAX, maximas_tareas);
	bzero(tareasPrioMED, maximas_tareas);
	bzero(tareasPrioMIN, maximas_tareas);
}

void osTaskCreate(task_type entry_point, uint32_t stack_size_b,
		eTaskPrioridades prioridad, void * arg) {
	if (puntero_proxima_tarea < maximas_tareas) {
		tareas[puntero_proxima_tarea].entry_point = entry_point;
		tareas[puntero_proxima_tarea].stack_size_b = stack_size_b;
		tareas[puntero_proxima_tarea].prioridad = prioridad;
		tareas[puntero_proxima_tarea].arg = arg;

		tareas[puntero_proxima_tarea].estado = eReady;

		init_stack(tareas[puntero_proxima_tarea].stack,
				tareas[puntero_proxima_tarea].stack_size_b,
				&tareas[puntero_proxima_tarea].sp,
				tareas[puntero_proxima_tarea].entry_point,
				tareas[puntero_proxima_tarea].arg);

		puntero_proxima_tarea++;
	}
}

void task_delay(uint32_t tick) {
	/*
	 * En esta tarea se asigna la cantidad de tick para delay de cada tarea
	 *
	 * En la proxima implementación voy a sacar delay como atributo de tareas y
	 * las guardo en un vector las que llaman a delay
	 */

	tareas[current_task].delay = tick;
	tareas[current_task].estado = eWaiting;
	schedule(NO_FROM_ISR);
}

/*==================[end of file]============================================*/

