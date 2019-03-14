// Metodo SysTick_Handler que realiza el cambio de contexto

	/**
	 * Directiva al ensablador que permite indicar que se encarga de buscar
	 * la instruccion mas apropiada entre thumb y thumb2
	 */
	.syntax unified

	/**
	 * .text permite indicar una seccion de codigo.
	 */
	.text

	/**
	 * .global permite definir un simbolo exportable,
	 * es decir que podemos verlo desde otros modulos (equivalente a extern).
     * Definimos la rutina como global para que sea visible desde otros modulos.
     */
	.global SysTick_Handler


	.extern sp1


	/**
	 * Indicamos que la siguiente subrutina debe ser ensamblada en modo thumb,
	 * entonces en las direcciones en el ultimo bit tendran el 1 para que se reconozcan como en modo thumb.
	 * Siempre hay que ponerla antes de la primer instruccion.
	 */
	.thumb_func


/**
 *	prototipo de la funcion en C
 *
 *	void SysTick_Handler( void );
 *
*/

SysTick_Handler:
    push {r4-r11}

	ldr r0,=sp1		/* r0 = &sp1	*/
	ldr r0,[r0]		/* r0 = *r0		*/

	msr msp,r0		/* msp = sp1	*/

    pop {r4-r11}

return:
	bx lr
