/*
 * main.c
 *
 *  Created on: 24/4/2019
 *      Author: Roberto Compañy
 *
 *  Importante:
 *  	- Si se presiona una tecla y se suelta sin presionar la otra, mensaje de error por UART.
 *  	- Si se presiona una tecla y luego la otra, luego una de las 2 se suelta y vuelve a presionar sin soltar la otra, mensaje de error por UART.
 *
 *  Notas:
 *  	- B1 se corresponde con TEC 1
 *  	- B2 se corresponde con TEC 2
 */

/*==================[inclusions]=============================================*/

#include "main.h"
#include "os.h"

#include "os_semaforos.h"
#include "perifericos.h"
#include "ciaaUART.h"

#include <stdint.h>
#include <stdlib.h>

/*==================[macros and definitions]=================================*/

enum estados_pulsadores {
	stSOLTADO, stPRESIONADO
};

/*==================[internal data declaration]==============================*/

typedef struct {
	pulsadores_e pulsador;
	uint8_t estado;
	uint8_t flanco;
	uint32_t tiempo_inicio_flanco_ascendente;
	uint32_t tiempo_inicio_flanco_descendente;
	uint8_t error;
} Pulsadores_t;

static Pulsadores_t pulsador[_PULSADORES_HABILITADOS_];

char str1[_LONG_LINEA_UART_];

/*==================[internal functions declaration]=========================*/

static void pulsador_inicializar(Pulsadores_t * p, pulsadores_e n);

void * tarea_estoyVivo(void * arg);

void * tarea_actualizar_pulsador_1(void * arg);
void * tarea_actualizar_pulsador_2(void * arg);
void analizar_pulsadores(void);
void * tarea_notificar_UART(void * arg);

/*==================[internal data definition]===============================*/

Semaforo_t s_actualizar_pulsador_1;
Semaforo_t s_actualizar_pulsador_2;
Semaforo_t s_notificar_UART;

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

void GPIO6_IRQHandler(void) {

	pulsador[0].flanco = Periferico_Get_Pulsador(pulsador[0].pulsador);
	if (pulsador[0].flanco)
		if (pulsador[0].tiempo_inicio_flanco_descendente == 0)
			pulsador[0].tiempo_inicio_flanco_descendente = get_tick();
		else
			pulsador[0].error = 1;
	else
		if (pulsador[0].tiempo_inicio_flanco_ascendente == 0)
			pulsador[0].tiempo_inicio_flanco_ascendente = get_tick();
		else
			pulsador[0].error = 1;

	semaforo_give(&s_actualizar_pulsador_1);

	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(6)); // INT0 (canal 0 -> hanlder GPIO0)
	NVIC_ClearPendingIRQ(PIN_INT6_IRQn);
}

void GPIO7_IRQHandler(void) {

	pulsador[1].flanco = Periferico_Get_Pulsador(pulsador[1].pulsador);
	if (pulsador[1].flanco)
		if (pulsador[1].tiempo_inicio_flanco_descendente == 0)
			pulsador[1].tiempo_inicio_flanco_descendente = get_tick();
		else
			pulsador[1].error = 1;
	else
		if (pulsador[1].tiempo_inicio_flanco_ascendente == 0)
			pulsador[1].tiempo_inicio_flanco_ascendente = get_tick();
		else
			pulsador[1].error = 1;

	semaforo_give(&s_actualizar_pulsador_2);

	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(7)); // INT0 (canal 0 -> hanlder GPIO0)
	NVIC_ClearPendingIRQ(PIN_INT7_IRQn);
}

static
void pulsador_inicializar(Pulsadores_t * p, pulsadores_e n) {
	p->pulsador = n;
	p->estado = stSOLTADO;
	p->flanco = 0;
	p->tiempo_inicio_flanco_ascendente = 0;
	p->tiempo_inicio_flanco_descendente = 0;
	p->error = 0;
}

void * tarea_estoyVivo(void * arg) {
	while (1) {

		//float pi = 3.14;
		//pi = pi * 3;

		Board_LED_Toggle(LED_1);
		task_delay(1000);
	}
}

void * tarea_actualizar_pulsador_1(void * arg) {
	while (1) {

		semaforo_take(&s_actualizar_pulsador_1);

		// Detectar antirrebotes (no hace falta porque esta por HW tambien)
		task_delay(50);

		uint8_t flanco = Periferico_Get_Pulsador(pulsador[0].pulsador);
		if (flanco == pulsador[0].flanco) {

			switch (pulsador[0].estado) {
			case stSOLTADO:
				pulsador[0].estado = flanco ? stPRESIONADO : stSOLTADO;
				break;
			case stPRESIONADO:
				pulsador[0].estado = flanco ? stPRESIONADO : stSOLTADO;
				break;
			default:
				pulsador[0].estado = stSOLTADO;
				break;
			}

			if (pulsador[0].estado == stSOLTADO)
				analizar_pulsadores();
		}
	}
}

void * tarea_actualizar_pulsador_2(void * arg) {
	while (1) {

		semaforo_take(&s_actualizar_pulsador_2);

		// Detectar antirrebotes (no hace falta porque esta por HW tambien)
		task_delay(50);

		uint8_t flanco = Periferico_Get_Pulsador(pulsador[1].pulsador);
		if (flanco == pulsador[1].flanco) {

			switch (pulsador[1].estado) {
			case stSOLTADO:
				pulsador[1].estado = flanco ? stPRESIONADO : stSOLTADO;
				break;
			case stPRESIONADO:
				pulsador[1].estado = flanco ? stPRESIONADO : stSOLTADO;
				break;
			default:
				pulsador[1].estado = stSOLTADO;
				break;
			}

			if (pulsador[1].estado == stSOLTADO)
				analizar_pulsadores();
		}
	}
}

void analizar_pulsadores(void){

	if (pulsador[0].estado == stSOLTADO && pulsador[1].estado == stSOLTADO){

		int tiempo_total = 0;
		int tiempo_entre_flancos_ascendentes = 0;
		int tiempo_entre_flancos_descendentes = 0;

		bzero(str1, _LONG_LINEA_UART_);

		char str2[_LONG_LINEA_UART_ / 2];
		bzero(str2, _LONG_LINEA_UART_ / 2);

		if (pulsador[0].error == 1								||
			pulsador[1].error == 1								||
			pulsador[0].tiempo_inicio_flanco_ascendente == 0	||
			pulsador[0].tiempo_inicio_flanco_descendente == 0	||
			pulsador[1].tiempo_inicio_flanco_ascendente == 0	||
			pulsador[1].tiempo_inicio_flanco_descendente == 0 	){

			strcpy(str1,"ERROR - Reintente por favor\r\n");
		}
		else{
			if ( pulsador[0].tiempo_inicio_flanco_descendente <= pulsador[1].tiempo_inicio_flanco_descendente ){
				// Verde o Rojo
				tiempo_entre_flancos_descendentes = pulsador[1].tiempo_inicio_flanco_descendente - pulsador[0].tiempo_inicio_flanco_descendente;
				if ( pulsador[0].tiempo_inicio_flanco_ascendente <= pulsador[1].tiempo_inicio_flanco_ascendente ){
					// Verde
					strcpy(str1, "Led Verde encendido: \n\r");
					Board_LED_Set(LED_3, TRUE);
					tiempo_entre_flancos_ascendentes = pulsador[1].tiempo_inicio_flanco_ascendente - pulsador[0].tiempo_inicio_flanco_ascendente;
				}else{
					// Rojo
					strcpy(str1, "Led Rojo encendido: \n\r");
					Board_LED_Set(LED_2, TRUE);
					tiempo_entre_flancos_ascendentes = pulsador[0].tiempo_inicio_flanco_ascendente - pulsador[1].tiempo_inicio_flanco_ascendente;
				}
			}else{
				//Amarillo o Azul
				tiempo_entre_flancos_descendentes = pulsador[0].tiempo_inicio_flanco_descendente - pulsador[1].tiempo_inicio_flanco_descendente;
				if ( pulsador[1].tiempo_inicio_flanco_ascendente <= pulsador[0].tiempo_inicio_flanco_ascendente ){
					//Azul
					strcpy(str1, "Led Azul encendido: \n\r");
					Board_LED_Set(LED_AZUL, TRUE);
					tiempo_entre_flancos_ascendentes = pulsador[0].tiempo_inicio_flanco_ascendente - pulsador[1].tiempo_inicio_flanco_ascendente;
				}else{
					//Amarillo
					strcpy(str1, "Led Amarillo encendido: \n\r");
					Board_LED_Set(LED_1, TRUE);
					tiempo_entre_flancos_ascendentes = pulsador[1].tiempo_inicio_flanco_ascendente - pulsador[0].tiempo_inicio_flanco_ascendente;
				}
			}
			tiempo_total =  tiempo_entre_flancos_ascendentes + tiempo_entre_flancos_descendentes;

			sprintf(str2,"\t Tiempo encendido: %d ms\n\r", tiempo_total);
			strcat(str1, str2);
			sprintf(str2,"\t Tiempo entre flancos descendentes: %d ms\n\r", tiempo_entre_flancos_descendentes);
			strcat(str1, str2);
			sprintf(str2,"\t Tiempo entre flancos ascendentes: %d ms\n\r\n\r", tiempo_entre_flancos_ascendentes);
			strcat(str1, str2);
		}
		semaforo_give(&s_notificar_UART);

		task_delay(tiempo_total);
		//Board_LED_Set(LED_VERDE, FALSE);
		//Board_LED_Set(LED_ROJO, FALSE);
		Board_LED_Set(LED_3, FALSE);	//VERDE
		Board_LED_Set(LED_2, FALSE);	//ROJO
		Board_LED_Set(LED_1, FALSE);	//Amarillo
		Board_LED_Set(LED_AZUL, FALSE);	//Azul

		pulsador[0].error = 0;
		pulsador[1].error = 0;
		pulsador[0].tiempo_inicio_flanco_ascendente = 0;
		pulsador[0].tiempo_inicio_flanco_descendente = 0;
		pulsador[1].tiempo_inicio_flanco_ascendente = 0;
		pulsador[1].tiempo_inicio_flanco_descendente = 0;
	}
}

void * tarea_notificar_UART(void * arg) {
	while (1) {
		semaforo_take(&s_notificar_UART);

		uartSend(str1, strlen(str1));
	}
}

/*==================[external functions definition]==========================*/

int main(void) {
	Board_Init();
	Perifericos_Init();
	Pulsadores_Init();
	uartInit();

	pulsador_inicializar(&pulsador[0], PULSADOR_1);
	pulsador_inicializar(&pulsador[1], PULSADOR_2);

	char str[50];
	strcpy(str, "\n\r\n\rInicio del OS - Examen ISO I\r\n");
	uartSend(str, strlen(str));

	//osTaskCreate(tarea_estoyVivo             , STACK_SIZE_B, ePrioMIN, (void*) 0x11223344);
	osTaskCreate(tarea_actualizar_pulsador_1 , STACK_SIZE_B, ePrioMED, (void*) 0x11223344);
	osTaskCreate(tarea_actualizar_pulsador_2 , STACK_SIZE_B, ePrioMED, (void*) 0x11223344);
	osTaskCreate(tarea_notificar_UART        , STACK_SIZE_B, ePrioMED, (void*) 0x11223344);

	semaforo_init(&s_actualizar_pulsador_1);
	semaforo_init(&s_actualizar_pulsador_2);
	semaforo_init(&s_notificar_UART);

	/*
	 * La inicializacion del OS debe ser posterior a la declaracion de tareas
	 */
	initOS();

	while (1) {
		__WFI();
	}
}

/*==================[end of file]============================================*/
