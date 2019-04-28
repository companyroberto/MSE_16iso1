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
#include "os_tareas.h"

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

void initOS(void);
void osTaskCreate(task_type entry_point, uint32_t stack_size_b, eTaskPrioridades prioridad, void * arg);
void task_delay( uint32_t systick);
uint32_t get_tick(void);
TaskParameters_t * get_current_task(void);
void schedule(void);
void ini_SeccionCritica(void);
void fin_SeccionCritica(void);

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/

#endif /* OS_H_ */
