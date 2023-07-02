#include <hifive1b_bsp/core_clock.hpp>

#include <cpu.hpp>

hifive1b::CoreClock::CoreClock() {
	set_max_speed();
}

Frequency hifive1b::CoreClock::get_frequency() {
	// Calculate the frequency from the PLL clock configuration
	PllConfigStatus cfg;
	get_pll_cfg(cfg);

	// If not using the PLL, the CPU is running at the default frequency from the internal oscillator
	if (!cfg.select) {
		return RESET_FREQUENCY;
	}

	// If bypassing the PLL, then the external oscillator frequency is being passed through
	if (cfg.bypass) {
		return HFXOSC_FREQUENCY;
	}

	// Otherwise, calculate how the reference frequency is transformed by the PLL
	return HFXOSC_FREQUENCY / cfg.R * cfg.F / cfg.Q;
}

void hifive1b::CoreClock::set_max_speed() {
	/* temporary impl */ if (false) {
		cpu_clock_init();
		return;
	}

	PllConfigStatus cfg;

	// Reference clock is the 16 MHz external crystal
	cfg.reference_select = PllReferenceClock::HFXOSC;

	// Set the divide ratio R to 2 to get out 8 MHz
	cfg.R = 2;

	// Multiply by 80 to get 640 MHz
	cfg.F = 80;

	// Divide by 2 to get the max rated CPU speed of 320 MHz
	cfg.Q = 2;

	cfg.bypass = false;

	// Do not drive the core clock using the PLL until the lock signal is received
	cfg.select = false;

	// Write the new configuration to the register
	set_pll_cfg(cfg);

	// The lock signal will not be stable for up to 100 microseconds; Delay for some CPU cycles
	volatile uint32_t counter = 0;
	while (counter < 1000) { ++counter; }

	// Wait for lock signal
	PllConfigStatus readConfig;
	do {
		get_pll_cfg(readConfig);
	} while (!readConfig.lock);

	// Select PLL to drive the core clock
	cfg.select = true;
	set_pll_cfg(cfg);

}

void hifive1b::CoreClock::get_pll_cfg(PllConfigStatus& cfg) {
	uint32_t pllreg = *reinterpret_cast<uint32_t*>(PLLCFG_REGISTER);

	cfg.R = (pllreg & 0b111) + 1;
	cfg.F = 2 * (((pllreg >> 4) & 0b11'1111) + 1);

	switch ((pllreg >> 10) & 0b11) {
		case 0b01:
			cfg.Q = 2;
			break;
		case 0b10:
			cfg.Q = 4;
			break;
		case 0b11:
			cfg.Q = 8;
			break;
		default:
			cfg.Q = 0;
	}

	cfg.select = (pllreg >> 16) & 0x1;
	cfg.reference_select = static_cast<PllReferenceClock>((pllreg >> 17) & 0x1);
	cfg.bypass = (pllreg >> 18) & 0x1;

	cfg.lock = (pllreg >> 31) & 0x1;

}

void hifive1b::CoreClock::set_pll_cfg(const PllConfigStatus& cfg) {
	uint32_t pllr = cfg.R - 1;
	uint32_t pllf = cfg.F / 2 - 1;

	// The value of pllq requires log to calculate but there are only 3 supported values - just hard-code the result
	uint32_t pllq = 0;
	switch (cfg.Q) {
		case 2:
			pllq = 0b01;
			break;
		case 4:
			pllq = 0b10;
			break;
		case 8:
			pllq = 0b11;
			break;
	}

	uint32_t pllreg = 0;
	pllreg |= pllr;
	pllreg |= pllf << 4;
	pllreg |= pllq << 10;

	pllreg |= cfg.select << 16;
	pllreg |= static_cast<uint32_t>(cfg.reference_select) << 17;
	pllreg |= cfg.bypass << 18;

	*reinterpret_cast<uint32_t*>(PLLCFG_REGISTER) = pllreg;

}