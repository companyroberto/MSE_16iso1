#ifndef _OS_TAREAS_H_
#define _OS_TAREAS_H_

/*==================[inclusions]=============================================*/

#include <stdint.h>

/*==================[macros]=================================================*/

#define maximas_tareas 10
#define STACK_SIZE_B   1024

typedef void * ( * task_type)(void *);

typedef enum {
	eRunning = 0, /* A task is querying the state of itself, so must be running. */
	eReady, /* The task being queried is in a read or pending ready list. */
	eWaiting, /* The task being queried is in the Blocked state. */
} eTaskState;

typedef enum {
	ePrioMAX = 1,
	ePrioMED,
	ePrioMIN,
} eTaskPrioridades;

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

/*==================[typedef]================================================*/

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

/*==================[end of file]============================================*/
#endif /* #ifndef _OS_TAREAS_H_ */
