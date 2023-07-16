#pragma once

#include <array>
#include <cstdint>

#include <util/safety.hpp>

#ifndef NATIVE
// Forward declarations
extern "C" {
	struct metal_led;
}
#else
using metal_led = uint8_t;
#endif

namespace hifive1b {

class LedDriver {
	public:
		LedDriver(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0);
		DISALLOW_COPY_AND_MOVE(LedDriver);

		/// Set the value of each RGB LED, where 0 is off and non-zero is on
		void set(uint8_t r, uint8_t g, uint8_t b);

	private:
		std::array<metal_led*, 3> rgb_led;

};

}