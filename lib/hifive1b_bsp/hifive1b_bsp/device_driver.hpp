#pragma once

#include <hifive1b_bsp/freedom_logger.hpp>
#include <hifive1b_bsp/leds.hpp>

//#include <util/logger.hpp>
#include <util/safety.hpp>

extern "C" {
#	include "metal/cpu.h"
}

namespace hifive1b {

/**
 * Device driver for the Hifive1 Rev B board
*/
template<typename Logger = MetalUartStream>
class Hifive1B {
	//static_assert(std::is_base_of_v<BasicOutStream<uint8_t>, Logger>, "Logger must be a BasicOutStream that consumes uint8_t");

	public:
		Hifive1B() {
			// Set the onboard LED to orange during initialization
			leds.set(1, 1, 0);
			logger << "Begin hardware initialization ...\n";

			bool failure = false;

			core = metal_cpu_get(metal_cpu_get_current_hartid());

			logger << "Constructor is unfinished ... failing out\n";
			failure = true;

			// Halt system in the event of failure
			if (failure) {
				// Attempt to blink red led on failure; LED and CPU timer initialization will not fail
				logger << "Error initializing hardware\n";
				bool toggle = false;
				leds.set(1, 0, 0);

				uint64_t endDelay = 0;
				for (;;) {
					// Minimal CPU delay function that works with default clocks
					endDelay = metal_cpu_get_timer(core) + 20'000'000;
					while (metal_cpu_get_timer(core) < endDelay) {
						;
					}
					leds.set(toggle, 0, 0);
					toggle = !toggle;
				}
			}

			// Set onboard LED to blue when handing back to the program
			leds.set(0, 0, 1);
			logger << "... Done\n";
		}

		DISALLOW_COPY_AND_MOVE(Hifive1B);

		LedDriver& get_led_driver() { return leds; }

	private:
		metal_cpu* core;
		LedDriver leds;
		Logger logger;

};

}
