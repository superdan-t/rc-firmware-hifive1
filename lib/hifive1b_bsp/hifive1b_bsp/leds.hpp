#pragma once

#include <array>

#include <util/safety.hpp>

// Forward declarations
extern "C" {
	struct metal_led;
}

namespace hifive1b {

class LedDriver {
	public:
		LedDriver();
		DISALLOW_COPY_AND_MOVE(LedDriver);

		/// Set the value of each RGB LED, where 0 is off and non-zero is on
		void set(uint8_t r, uint8_t g, uint8_t b);

	private:
		std::array<metal_led*, 3> rgb_led;

};

}