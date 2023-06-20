#include "esp32_at_app.hpp"

#include <array>

#include "hifive1b_bsp/device_driver.hpp"

extern "C" {

#	include "metal/led.h"
#	include "metal/shutdown.h"
#	include "metal/spi.h"
#	include "metal/cpu.h"
#	include "metal/clock.h"

}

static std::array<metal_spi*, 3> spi_d {nullptr};

static bool get_spi_devices() {
	for (size_t i = 0; i < spi_d.size(); ++i) {
		auto device_n = metal_spi_get_device(i);
		if (!device_n) {
			printf("Error getting SPI device %u (does not exist)\n", i);
			return false;
		}
		spi_d[i] = device_n;
	}
	return true;
}

int esp32_at_main() {

	hifive1b::Hifive1B driver;

//	MetalUartStream basicLogger;
//	basicLogger << "Application esp32_at_main; Entering main loop.\n";

	for (;;) {

	}

	// Return non-OK status if exit is reached
	return 1;
}