#pragma once

#include <array>

#include <hifive1b_bsp/core_clock.hpp>
#include <hifive1b_bsp/freedom_logger.hpp>
#include <hifive1b_bsp/leds.hpp>
#include <hifive1b_bsp/spi_driver.hpp>

//#include <util/logger.hpp>
#include <util/safety.hpp>

extern "C" {
#	include "metal/cpu.h"
}

namespace hifive1b {

/**
 * Device driver for the Hifive1 Rev B board
*/
template<typename Logger = MetalUartStream,
	typename SpiDriverT = SpiDriver,
	typename CoreClockDriverT = CoreClock>
class Hifive1B {
	//static_assert(std::is_base_of_v<BasicOutStream<uint8_t>, Logger>, "Logger must be a BasicOutStream that consumes uint8_t");

	public:
		Hifive1B() :
			core(metal_cpu_get(metal_cpu_get_current_hartid())),

			// Set LEDs to orange during initialization
			leds(1, 1, 0),

			// Construct 3 SPI devices
			spi_drivers{SpiDriverT(0), SpiDriverT(1), SpiDriverT(2)}
		{
			// Devices are constructed above

			bool failure = false;

			// Make sure the SPI drivers constructed correctly
			for (const auto& d : spi_drivers) {
				failure = (d.get_state() != SpiDriver::State::VALID_UNINITIALIZED) || failure;
			}

			// Halt system in the event of failure
			if (failure) {
				halt_and_catch_fire();
			}

			// Set onboard LED to blue when handing back to the program
			leds.set(0, 0, 1);
		}

		DISALLOW_COPY_AND_MOVE(Hifive1B);

		LedDriver& get_led_driver() { return leds; }

		/// Get the number of SPI devices supported
		std::size_t num_spi_devices() { return spi_drivers.size(); }

		/// Get an SPI device. i must be strictly less than the return value of num_spi_devices() or behavior is undefined
		SpiDriverT& get_spi(std::size_t i) { return spi_drivers[i]; }

		/// Get the driver for the high-frequency clock (hfclk) that drives the core and other devices 
		CoreClockDriverT& get_clock_driver() { return hf_clock; }

	private:
		metal_cpu* core;
		CoreClockDriverT hf_clock;
		LedDriver leds;
		Logger logger;

		std::array<SpiDriverT, 3> spi_drivers;

		void halt_and_catch_fire() {
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

};

}
