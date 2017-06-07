#include "ch.h"
#include "hal.h"
#include "debug.h"

static module_conf_t *registered_threads[10];
static uint8_t threads_cnt = 0;
static bool led_sw;

// Hardware Watchdog configuration
static const WDGConfig wdgcfg = {
	.pr =	STM32_IWDG_PR_256,
	.rlr =	STM32_IWDG_RL(10000)
};

void register_thread_at_wdg(module_conf_t *thread_config)
{
	registered_threads[threads_cnt++] = thread_config;
}

THD_FUNCTION(wdgThread, arg) {
	(void)arg;

	while(true)
	{
		bool healthy = true;
		for(uint8_t i=0; i<threads_cnt; i++) {
			if(registered_threads[i]->wdg_timeout < chVTGetSystemTimeX())
			{
				TRACE_ERROR("WDG  > Thread %s not healty", registered_threads[i]->name);
				healthy = false; // Threads reached timeout
			}
		}

		if(healthy)
			wdgReset(&WDGD1);	// Reset hardware watchdog at no error

		// Switch LEDs
		palWritePad(PORT(IO_LED3), PIN(IO_LED3), led_sw);	// Show I'M ALIVE
		if(!healthy) {
			palWritePad(PORT(IO_LED1), PIN(IO_LED1), led_sw);	// Show error
		} else {
			palSetPad(PORT(IO_LED1), PIN(IO_LED1));	// Shut off error
		}
		led_sw = !led_sw;

		chThdSleepMilliseconds(500);
	}
}

void init_watchdog(void)
{
	// Initialize Watchdog
	TRACE_INFO("WDG  > Initialize Watchdog");
	wdgStart(&WDGD1, &wdgcfg);
	wdgReset(&WDGD1);

	TRACE_INFO("WDG  > Startup Watchdog thread");
	thread_t *th = chThdCreateFromHeap(NULL, THD_WORKING_AREA_SIZE(256), "WDG", NORMALPRIO, wdgThread, NULL);
	if(!th) {
		// Print startup error, do not start watchdog for this thread
		TRACE_ERROR("TRAC > Could not startup thread (not enough memory available)");
	}
}

