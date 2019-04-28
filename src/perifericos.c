/*
 * perifericos.c
 *
 *  Created on: 17/4/2019
 *      Author: TitO
 *
 */

/*==================[inclusions]=============================================*/

#include "perifericos.h"

#include "board.h"

/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/

void Perifericos_Init(void) {
	int gpio_pin;

	gpio_pin = 1 << 4;
	Chip_SCU_PinMux(1, 0, MD_PUP | MD_EZI | MD_ZI, FUNC0); /* SW1 */
	Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, 0, gpio_pin);

	gpio_pin = 1 << 8;
	Chip_SCU_PinMux(1, 1, MD_PUP | MD_EZI | MD_ZI, FUNC0); /* SW2 */
	Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, 0, gpio_pin);

	gpio_pin = 1 << 9;
	Chip_SCU_PinMux(1, 2, MD_PUP | MD_EZI | MD_ZI, FUNC0); /* SW3 */
	Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, 0, gpio_pin);

	gpio_pin = 1 << 9;
	Chip_SCU_PinMux(1, 6, MD_PUP | MD_EZI | MD_ZI, FUNC0); /* SW4 */
	Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, 1, gpio_pin);

}

void Pulsadores_Init(void) {
	//Seteo la interrupcion para el flanco ascendente y descendente de la tecla 1
	Chip_SCU_GPIOIntPinSel(6, 0, 4);

	//Borra el pending de la IRQ
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(6));

	//Selecciona activo por flanco
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH(6));

	//Selecciona activo por flanco descendente
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH(6));

	//Selecciona activo por flanco ascendente
	Chip_PININT_EnableIntHigh(LPC_GPIO_PIN_INT, PININTCH(6));

	// Asignar prioridad
	//NVIC_SetPriority(PIN_INT6_IRQn, 0xFFFF);

	//Borra el clear pending de la IRQ y lo activa
	NVIC_ClearPendingIRQ(PIN_INT6_IRQn);
	NVIC_EnableIRQ(PIN_INT6_IRQn);

	//Seteo la interrupcion para el flanco ascendente y descendente de la tecla 2
	Chip_SCU_GPIOIntPinSel(7, 0, 8);

	//Borra el pending de la IRQ
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(7));

	//Selecciona activo por flanco
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH(7));

	//Selecciona activo por flanco descendente
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH(7));

	//Selecciona activo por flanco ascendente
	Chip_PININT_EnableIntHigh(LPC_GPIO_PIN_INT, PININTCH(7));

	// Asignar prioridad
	//NVIC_SetPriority(PIN_INT7_IRQn, 0xFFFF);

	//Borra el clear pending de la IRQ y lo activa
	NVIC_ClearPendingIRQ(PIN_INT7_IRQn);
	NVIC_EnableIRQ(PIN_INT7_IRQn);
}

uint8_t Periferico_Get_Pulsador(uint8_t pulsador) {

	uint8_t resultado;

	switch (pulsador) {
	case PULSADOR_1:
		resultado = !Chip_GPIO_GetPinState(LPC_GPIO_PORT, 0, 4);
		break;
	case PULSADOR_2:
		resultado = !Chip_GPIO_GetPinState(LPC_GPIO_PORT, 0, 8);
		break;
	case PULSADOR_3:
		resultado = !Chip_GPIO_GetPinState(LPC_GPIO_PORT, 0, 9);
		break;
	case PULSADOR_4:
		resultado = !Chip_GPIO_GetPinState(LPC_GPIO_PORT, 1, 9);
		break;
	default:
		resultado = 0;
		break;
	}

	return resultado;
}

/*==================[end of file]============================================*/
