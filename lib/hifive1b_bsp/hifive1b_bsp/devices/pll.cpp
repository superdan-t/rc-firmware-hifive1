#include <hifive1b_bsp/devices/pll.hpp>

#include <util/control_register.hpp>

// Constants for the pllcfg register

static constexpr ControlRegister<uint32_t> pllcfg(0x10008008);

static constexpr auto PLL_R = BitField<uint32_t>::from_range<2, 0>();
static constexpr auto PLL_F = BitField<uint32_t>::from_range<9, 4>();
static constexpr auto PLL_Q = BitField<uint32_t>::from_range<11, 10>();

static constexpr auto PLL_SEL = BitField<uint32_t>::single_bit<16>();
static constexpr auto PLL_REF_SEL = BitField<uint32_t>::single_bit<17>();
static constexpr auto PLL_BYPASS = BitField<uint32_t>::single_bit<18>();

static constexpr auto PLL_LOCK = BitField<uint32_t>::single_bit<31>();

Frequency hifive1b::Pll::get_output_frequency() const {
	// Calculate the frequency from the PLL clock configuration
	ConfigStatus cfg;
	get_config(cfg);

	// If bypassing the PLL, then the external oscillator frequency is being passed through
	if (cfg.bypass) {
		return HFXOSC_FREQUENCY;
	}

	// Otherwise, calculate how the reference frequency is transformed by the PLL
	return HFXOSC_FREQUENCY / cfg.R * cfg.F / cfg.Q;
}

void hifive1b::Pll::set_config(const ConfigStatus& cfg) const {

	// Encode the mathematical config inputs into the register's format
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


	// Begin modifying the fields in one operation
	
	auto reg_transact = pllcfg.start_atomic_transaction();
	reg_transact.set_field(PLL_R, pllr);
	reg_transact.set_field(PLL_F, pllf);
	reg_transact.set_field(PLL_Q, pllq);
	reg_transact.set_field(PLL_SEL, cfg.select);
	reg_transact.set_field(PLL_REF_SEL, static_cast<uint32_t>(cfg.reference_select));
	reg_transact.set_field(PLL_BYPASS, cfg.bypass);

	// Write the new config
	reg_transact.finalize();
}

void hifive1b::Pll::get_config(ConfigStatus& cfg) const {
	// Make a copy of the register instead of reading directly from it every time
	auto register_value = pllcfg.copy_value();

	// Decode the register fields into the mathematical config values

	cfg.R = register_value.get_field(PLL_R) + 1;
	cfg.F = 2 * (register_value.get_field(PLL_F) + 1);

	switch (register_value.get_field(PLL_Q)) {
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

	cfg.select = register_value.get_field<bool>(PLL_SEL);
	cfg.reference_select = register_value.get_field<ReferenceClock>(PLL_REF_SEL);
	cfg.bypass = register_value.get_field<bool>(PLL_BYPASS);

	cfg.lock = register_value.get_field<bool>(PLL_LOCK);

}

void hifive1b::Pll::configure_and_select(const ConfigStatus& cfg) const {
	ConfigStatus local_cfg;

	// If the new configuration is bypassing the PLL, then there's no need to temporarily use an alternate clock and we
	// shouldn't wait for the lock signal
	if (cfg.bypass) {
		local_cfg = cfg;
		local_cfg.select = true;
		set_config(local_cfg);
	} else {
		// The PLL cannot drive hfclk during reconfiguration, so switch to the default internal oscillator and make sure
		// the PLL is deselected
		local_cfg = cfg;
		local_cfg.select = false;

		// Write the new configuration to the register
		set_config(local_cfg);

		// The lock signal will not be stable for up to 100 microseconds
		// At the HFROSC reset frequency, counting to 1000 will surely exceed this
		volatile uint32_t counter = 0;
		while (counter < 1000) { ++counter; }

		// Wait for lock signal
		do {
			local_cfg.lock = pllcfg.get_field<bool>(PLL_LOCK);
		} while (!local_cfg.lock);

		// Switch back to the PLL after locking
		pllcfg.set_field(PLL_SEL, true);
	}
}

bool hifive1b::Pll::is_selected() const {
	return pllcfg.get_field<bool>(PLL_SEL);
}
