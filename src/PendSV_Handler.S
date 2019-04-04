// Metodo PendSV_Handler que realiza el cambio de contexto

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
	.global PendSV_Handler


	.extern get_next_context


	/**
	 * Indicamos que la siguiente subrutina debe ser ensamblada en modo thumb,
	 * entonces en las direcciones en el ultimo bit tendran el 1 para que se reconozcan como en modo thumb.
	 * Siempre hay que ponerla antes de la primer instruccion.
	 */
	.thumb_func


/**
 *	prototipo de la funcion en C
 *
 *	void PendSV_Handler( void );
 *
*/

PendSV_Handler:
    push {r4-r11,lr}				// Como voy a saltar a una funcion estando dentro de otra, tengo que guardar el Link Register*/

	mrs r0,msp						/* r0 = msp		*/
	bl get_next_context
	msr msp,r0						/* msp = sp1	*/

    pop {r4-r11,lr}

return:
	bx lr
