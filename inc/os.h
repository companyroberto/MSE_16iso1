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

#define STACK_SIZE_B 512

typedef void * ( * task_type)(void *);

/*==================[typedef]================================================*/

/*==================[external data declaration]==============================*/

//uint32_t sp1, sp2, sp3;
//uint32_t stack1[STACK_SIZE_B / 4];
//uint32_t stack2[STACK_SIZE_B / 4];
//uint32_t stack3[STACK_SIZE_B / 4];

/*==================[external functions declaration]=========================*/

void initOS(void);
void init_stack(uint32_t stack[], uint32_t stack_size, uint32_t * sp, task_type entry_point, void * arg);
void osTaskCreate(task_type entry_point, uint32_t stack_size_b, uint32_t tskIDLE_PRIORITY, void * arg);
void task_delay( uint32_t systick);

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/

#endif /* OS_H_ */
