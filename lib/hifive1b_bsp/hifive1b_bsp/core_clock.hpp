#pragma once

#include <util/safety.hpp>

#include <hifive1b_bsp/clock.hpp>
#include <hifive1b_bsp/devices/pll.hpp>

namespace hifive1b {

/// Core clock manager class for the Hifive1 Rev B
///
/// This class manages the high-frequency clocks used to drive the core. It assumes that this class is the sole manager
/// of the high-frequency clock devices (HFROSC, HFXOSC, HFPLL). Configuring the clocks externally may result in
/// incorrect behavior.
class CoreClock : public Clock {
	public:
		/// Construct a class for driving the core clock of the FE310-G002
		///
		///	During construction, the PLL is initialized and the clock switches from the default internal oscillator
		/// (HFROSC) to the high-frequency external oscillator (HFXOSC). The HFROSC is switched off after initialization
		CoreClock(bool initial_max_speed = true);

		DISALLOW_COPY_AND_MOVE(CoreClock);

		void set_max_speed();
		void set_low_speed();

		Frequency get_frequency() override;
	private:

		/// The clock frequency of the FE310-G002 after reset
		///
		/// After reset, the FE310-G002 uses the internal oscillator as the clock source
		static constexpr Frequency RESET_FREQUENCY = frequency::KHz(13800);

		/// Driver for the clock multiplier which can be used to drive the core clock
		static constexpr Pll pll_driver {};

};

}