#pragma once

#include <hifive1b_bsp/frequency.hpp>

class Clock {

	public:
		virtual Frequency get_frequency() = 0;

};
