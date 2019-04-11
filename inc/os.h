/*
 * os.h
 *
 *  Created on: 4/4/2019
 *      Author: TitO
 */

#ifndef OS_H_
#define OS_H_


/*==================[inclusions]=============================================*/

#include "board.h"

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

#define maximas_tareas 10
#define STACK_SIZE_B   512

typedef void * ( * task_type)(void *);

typedef enum {
	ePrioMAX = 1,
	ePrioMED,
	ePrioMIN,
} eTaskPrioridades;

/*==================[typedef]================================================*/

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

void initOS(void);
void osTaskCreate(task_type entry_point, uint32_t stack_size_b, eTaskPrioridades prioridad, void * arg);
void task_delay( uint32_t systick);

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/

#endif /* OS_H_ */
