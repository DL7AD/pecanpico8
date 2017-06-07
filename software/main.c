#include "ch.h"
#include "hal.h"

#include "debug.h"
#include "modules.h"
#include "padc.h"

/**
  * Main routine is starting up system, runs the software watchdog (module monitoring), controls LEDs
  */
int main(void) {
	halInit();					// Startup HAL
	chSysInit();				// Startup RTOS

	boost_voltage(true); // FIXME: TMP

	DEBUG_INIT();				// Debug Init (Serial debug port, LEDs)
	TRACE_INFO("MAIN > Startup");

	start_essential_threads();	// Startup required modules (tracking managemer, watchdog)
	start_user_modules();		// Startup optional modules (eg. POSITION, LOG, ...)

	// Print time every 10 sec
	while(true) {
		PRINT_TIME("MAIN");
		chThdSleepMilliseconds(10000);
	}
}

