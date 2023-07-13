#include <hifive1b_bsp/core_clock.hpp>

/// Constants defining the layout of the pllcfg register
namespace PllcfgFields {
	static constexpr uint32_t R_OFFSET = 0;
	static constexpr uint32_t F_OFFSET = 4;
	static constexpr uint32_t Q_OFFSET = 10;

	static constexpr uint32_t R_MASK = 0b111;
	static constexpr uint32_t F_MASK = 0b11'1111;
	static constexpr uint32_t Q_MASK = 0b11;

	static constexpr uint32_t SEL_OFFSET = 16;
	static constexpr uint32_t REF_SEL_OFFSET = 17;
	static constexpr uint32_t BYPASS_OFFSET = 18;

	static constexpr uint32_t LOCK_OFFSET = 31;
};

hifive1b::CoreClock::CoreClock(bool initial_max_speed) {
	// Either path switches away from the internal oscillator and shuts it off when finished
	if (initial_max_speed)
		set_max_speed();
	else
		set_low_speed();
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
	PllConfigStatus cfg;

	// Reference clock is the 16 MHz external crystal
	cfg.reference_select = PllReferenceClock::HFXOSC;

	// Set the divide ratio R to 2 to get out 8 MHz
	cfg.R = 2;

	// Multiply by 80 to get 640 MHz
	cfg.F = 80;

	// Divide by 2 to get the max rated CPU speed of 320 MHz
	cfg.Q = 2;

	// Do not bypass the PLL
	cfg.bypass = false;

	// Apply this configuration
	configure_and_select_pll(cfg);

}

void hifive1b::CoreClock::set_low_speed() {
	PllConfigStatus cfg;

	// Use the 16 MHz crystal
	cfg.reference_select = PllReferenceClock::HFXOSC;

	// Shut off and bypass the PLL, using the reference clock directly
	cfg.bypass = true;

	configure_and_select_pll(cfg);

}

void hifive1b::CoreClock::get_pll_cfg(PllConfigStatus& cfg) {
	uint32_t pllreg = *PLLCFG_REGISTER;

	using namespace PllcfgFields;

	cfg.R = (pllreg & R_MASK) + 1;
	cfg.F = 2 * (((pllreg >> F_OFFSET) & F_MASK) + 1);

	switch ((pllreg >> Q_OFFSET) & Q_MASK) {
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

	cfg.select = (pllreg >> SEL_OFFSET) & 0x1;
	cfg.reference_select = static_cast<PllReferenceClock>((pllreg >> REF_SEL_OFFSET) & 0x1);
	cfg.bypass = (pllreg >> BYPASS_OFFSET) & 0x1;

	cfg.lock = (pllreg >> LOCK_OFFSET) & 0x1;

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

	using namespace PllcfgFields;

	uint32_t pllreg = 0;
	pllreg |= pllr;
	pllreg |= pllf << F_OFFSET;
	pllreg |= pllq << Q_OFFSET;

	pllreg |= cfg.select << SEL_OFFSET;
	pllreg |= static_cast<uint32_t>(cfg.reference_select) << REF_SEL_OFFSET;
	pllreg |= cfg.bypass << BYPASS_OFFSET;

	*PLLCFG_REGISTER = pllreg;

}

void hifive1b::CoreClock::configure_and_select_pll(const PllConfigStatus& cfg) {

	PllConfigStatus local_cfg;

	// If the new configuration is bypassing the PLL, then there's no need to temporarily use an alternate clock and we
	// shouldn't wait for the lock signal
	if (cfg.bypass) {
		local_cfg = cfg;
		local_cfg.select = true;
		set_pll_cfg(local_cfg);
	} else {

		// The PLL cannot drive hfclk during reconfiguration, so switch to the default internal oscillator and make sure
		// the PLL is deselected
		local_cfg = cfg;
		local_cfg.select = false;

		// Write the new configuration to the register
		set_pll_cfg(local_cfg);

		// The lock signal will not be stable for up to 100 microseconds
		// At the HFROSC reset frequency, counting to 1000 will surely exceed this
		volatile uint32_t counter = 0;
		while (counter < 1000) { ++counter; }

		// Wait for lock signal
		do {
			get_pll_cfg(local_cfg);
		} while (!local_cfg.lock);

		// Switch back to the PLL after locking
		local_cfg.select = true;
		set_pll_cfg(local_cfg);
	}

	// Notify devices of the new clock speed
	emit_frequency_change(get_frequency());

}
