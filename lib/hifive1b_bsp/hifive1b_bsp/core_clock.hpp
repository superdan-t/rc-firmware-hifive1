#pragma once

#include <hifive1b_bsp/clock.hpp>

namespace hifive1b {

/// Core clock manager class for the Hifive1 Rev B
///
/// This class manages the high-frequency clocks used to drive the core. It assumes that this class is the sole manager
/// of the high-frequency clock devices (HFROSC, HFXOSC, HFPLL). Configuring the clocks externally may result in
/// incorrect behavior.
class CoreClock : public Clock {
	public:
		/// Construct a class for driving the core clock of the FE310-G002
		CoreClock();

		void set_max_speed();

		Frequency get_frequency() override;
	private:

		/// The clock frequency of the FE310-G002 after reset
		///
		/// After reset, the FE310-G002 uses the internal oscillator as the clock source
		static constexpr Frequency RESET_FREQUENCY = frequency::KHz(13800);

		/// The frequency of the High-Frequency External Oscillator (HFXOSC) on the Hifive1 Rev B
		static constexpr Frequency HFXOSC_FREQUENCY = frequency::MHz(16);

		static constexpr uintptr_t PLLCFG_REGISTER = 0x10008008;

		/// Possible drivers for the PLL
		enum class PllReferenceClock : uint8_t {
			/// Presumably inoperational
			NONE = 0,
			/// High Frequency External Oscillator
			HFXOSC = 1
		};

		/// See FE310-G002 Manual Section 6.5 for more information about fields
		///
		/// The R, F, and Q fields use the desired value for R, F, and Q. The number that must be
		/// written in the register will be calculated by the set_pll_cfg function according to the
		/// formulas in section 6.5.
		struct PllConfigStatus {
			uint8_t R = 0x1;
			uint8_t F = 0x1F;
			uint8_t Q = 0x3;

			bool select = false;
			bool bypass = false;
			bool lock = false;
			PllReferenceClock reference_select = PllReferenceClock::HFXOSC;

		};

		/// Set the PLL configuration and status register
		void set_pll_cfg(const PllConfigStatus&);

		/// Read the PLL config and status register
		void get_pll_cfg(PllConfigStatus&);

};

}