#include <hifive1b_bsp/spi_driver.hpp>

#include <cstddef>

extern "C" {
#	include <metal/spi.h>
}

hifive1b::SpiDriver::SpiDriver(uint32_t device_number) {
	handle = metal_spi_get_device(device_number);
	if (handle != nullptr) {
		state = State::VALID_UNINITIALIZED;
	}
}

void hifive1b::SpiDriver::set_baud_rate(uint32_t rate) {
	switch (state) {
		case State::VALID_UNINITIALIZED:
			metal_spi_init(handle, rate);
			state = State::INITIALIZED;
			break;
		case State::INITIALIZED:
			metal_spi_set_baud_rate(handle, rate);
			break;
	}
	baud_rate = metal_spi_get_baud_rate(handle);
}
