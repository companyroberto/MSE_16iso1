/*
 * perifericos.h
 *
 *  Created on: 17/4/2019
 *      Author: TitO
 */

#ifndef _PERIFERICOS_H_
#define _PERIFERICOS_H_


/*==================[inclusions]=============================================*/

#include <stdint.h>

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

#define LED_ROJO  0
#define LED_VERDE 1
#define LED_AZUL  2
#define LED_1     3
#define LED_2     4
#define LED_3     5

/*==================[typedef]================================================*/

typedef enum { PULSADOR_1, PULSADOR_2, PULSADOR_3, PULSADOR_4 } pulsadores_e;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

void Perifericos_Init(void);
void Pulsadores_Init(void);
uint8_t Periferico_Get_Pulsador(uint8_t pulsador);

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/
#endif /* #ifndef _PERIFERICOS_H_ */
