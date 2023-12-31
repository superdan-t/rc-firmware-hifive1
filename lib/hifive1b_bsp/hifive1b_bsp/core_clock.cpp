#include <hifive1b_bsp/core_clock.hpp>

hifive1b::CoreClock::CoreClock(bool initial_max_speed) {
	// Either path switches away from the internal oscillator and shuts it off when finished
	if (initial_max_speed)
		set_max_speed();
	else
		set_low_speed();
}

Frequency hifive1b::CoreClock::get_frequency() {
	if (pll_driver.is_selected()) {
		return pll_driver.get_output_frequency();
	} else {
		return RESET_FREQUENCY;
	}
}

void hifive1b::CoreClock::set_max_speed() {
	Pll::ConfigStatus cfg;

	// Reference clock is the 16 MHz external crystal
	cfg.reference_select = Pll::ReferenceClock::HFXOSC;

	// Set the divide ratio R to 2 to get out 8 MHz
	cfg.R = 2;

	// Multiply by 80 to get 640 MHz
	cfg.F = 80;

	// Divide by 2 to get the max rated CPU speed of 320 MHz
	cfg.Q = 2;

	// Do not bypass the PLL
	cfg.bypass = false;

	// Apply this configuration
	pll_driver.configure_and_select(cfg);

	emit_frequency_change(get_frequency());

}

void hifive1b::CoreClock::set_low_speed() {
	Pll::ConfigStatus cfg;

	// Use the 16 MHz crystal
	cfg.reference_select = Pll::ReferenceClock::HFXOSC;

	// Shut off and bypass the PLL, using the reference clock directly
	cfg.bypass = true;

	pll_driver.configure_and_select(cfg);

	emit_frequency_change(get_frequency());

}

