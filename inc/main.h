/*
 * os.h
 *
 *  Created on: 4/4/2019
 *      Author: TitO
 */

#ifndef _MAIN_H_
#define _MAIN_H_


/*==================[inclusions]=============================================*/

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

/** led number to toggle */
#define LED_ROJO  0
#define LED_VERDE 1
#define LED_AZUL  2
#define LED_1     3
#define LED_2     4
#define LED_3     5

/*==================[typedef]================================================*/

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

/** @brief main function
 * @return main function should never return
 */
int main(void);

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/
#endif /* #ifndef _MAIN_H_ */
