/*==================[inclusions]=============================================*/

#include "os_semaforos.h"
#include "os.h"

/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/

void semaforo_init(Semaforo_t * t){
	t->tomado = 0;
}

void semaforo_take(Semaforo_t * t){

	ini_SeccionCritica();

	if ( t->tomado == 0 ){
		t->tarea = get_current_task();

		t->tomado = 1;
		t->tarea->estado = eWaiting;

		schedule();
	}
	else{
		// semaforo ya tomado
	}
	fin_SeccionCritica();
}

void semaforo_give(Semaforo_t * t){

	ini_SeccionCritica();

	if ( t->tomado == 1 ){
		t->tomado = 0;
		t->tarea->estado = eReady;

		schedule();
	}
	else{
		// semaforo no tomado
	}
	fin_SeccionCritica();
}

/*==================[end of file]============================================*/
