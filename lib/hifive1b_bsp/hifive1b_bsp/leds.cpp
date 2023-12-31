#include <hifive1b_bsp/leds.hpp>

#ifndef NATIVE
extern "C" {
#	include <metal/led.h>
}
#endif

hifive1b::LedDriver::LedDriver(uint8_t r, uint8_t g, uint8_t b) {
#ifndef NATIVE
	rgb_led[0] = metal_led_get_rgb("LD0", "red");
    rgb_led[1] = metal_led_get_rgb("LD0", "green");
    rgb_led[2] = metal_led_get_rgb("LD0", "blue");

	for (auto led_ptr : rgb_led) {
		if (led_ptr != nullptr) {
			metal_led_enable(led_ptr);
		}
	}

	set(r, g, b);
#endif
}

void hifive1b::LedDriver::set(uint8_t r, uint8_t g, uint8_t b) {
#ifndef NATIVE
	// Note that the on and off calls are inverted on the HiFive1 Rev B...

	// Default all off
	for (auto le : rgb_led) {
		metal_led_on(le);
	}

	// Enable channels that should be on
	if (r)
		metal_led_off(rgb_led[0]);
	if (g)
		metal_led_off(rgb_led[1]);
	if (b)
		metal_led_off(rgb_led[2]);
#endif
}
