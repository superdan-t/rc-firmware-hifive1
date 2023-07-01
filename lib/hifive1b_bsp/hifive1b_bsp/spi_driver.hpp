#pragma once

#include <cstdint>

#include <util/safety.hpp>

// Forward declarations

extern "C" {
	struct metal_spi;
}

namespace hifive1b {

/// Driver for an SPI bus using the Freedom Metal BSP
class SpiDriver {
	public:

		/// Possible states of the driver
		enum class State : uint8_t {
			/// Device does not exist or is otherwise unavailable
			INVALID,
			/// Device has a valid handle but the hardware not been initialized
			VALID_UNINITIALIZED,
			/// Device is initialized and ready to transmit data
			INITIALIZED,
		};

		/// Construct an SPI driver and load the device handle. Sets state to VALID if successful
		/// @param device_number An integer in [0,2] corresponding to one of the 3 SPI devices on the Hifive1
		SpiDriver(uint32_t device_number);
		
		DISALLOW_COPY_AND_MOVE(SpiDriver);

		/// Set the baud rate in hertz
		void set_baud_rate(uint32_t rate);

		inline State get_state() const { return state; }
		inline uint32_t get_baud_rate() const { return baud_rate; } 

	private:
		metal_spi* handle = nullptr;
		uint32_t baud_rate = 0;
		State state = State::INVALID;

};

/// Empty driver for devices that shouldn't use SPI
class EmptySpiDriver {
	public:
		EmptySpiDriver(uint32_t device_number) {}
		DISALLOW_COPY_AND_MOVE(EmptySpiDriver);

		void set_baud_rate(uint32_t) {}
		uint32_t get_baud_rate() const { return 0; }
		SpiDriver::State get_state() const { return SpiDriver::State::VALID_UNINITIALIZED; }

};

} // namespace hifive1b