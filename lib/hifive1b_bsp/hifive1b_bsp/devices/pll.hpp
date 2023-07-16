#pragma once

#include <cstdint>
#include <embedded_util/control_register.hpp>
#include <hifive1b_bsp/frequency.hpp>

namespace hifive1b {

/// Driver for the Phase-locked loop (PLL) clock multiplier on the FE310-G002
///
/// Default-constructed instances of this class refer to the only on-chip PLL of the SoC with the
/// intent that it's only instantiated once per chip. The class has no data members with its state
/// being determined by the PLL config register itself.
///
/// More information on the PLL is available in the FE310-G002 Manual Section 6.5
class Pll {
	public:

		constexpr Pll(uintptr_t addr = 0x10008008) : 
			pllcfg(addr)
		{}

		/// Possible input (reference) clocks for the PLL
		enum class ReferenceClock : uint8_t {
			/// Presumably inoperational
			NONE = 0,
			/// High Frequency External Oscillator
			HFXOSC = 1
		};

		/// The frequency of the High-Frequency External Oscillator (HFXOSC) on the Hifive1 Rev B which is connected to the PLL
		static constexpr Frequency HFXOSC_FREQUENCY = frequency::MHz(16);

		/// See FE310-G002 Manual Section 6.5 for more information about fields
		///
		/// The R, F, and Q fields use the desired value for R, F, and Q. The number that must be
		/// written in the register will be calculated by the set function according to the
		/// formulas in section 6.5.
		struct ConfigStatus {
			uint8_t R = 0x1;
			uint8_t F = 0x1F;
			uint8_t Q = 0x3;

			bool select = false;
			bool bypass = false;
			bool lock = false;
			ReferenceClock reference_select = ReferenceClock::HFXOSC;
		};

		/// Calculate and return the output frequency
		Frequency get_output_frequency() const;

		/// Read the PLL configuration and status register
		void get_config(ConfigStatus&) const;

		/// Write a new configuration to the PLL and select it to drive the clock
		void configure_and_select(const ConfigStatus&) const;

		/// Returns true if the PLL is driving the hfclk on the device
		bool is_selected() const;
	
	private:
		/// Set the PLL configuration and status register
		void set_config(const ConfigStatus&) const;

		ControlRegister<uint32_t> pllcfg;

};

}