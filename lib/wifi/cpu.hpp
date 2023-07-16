#pragma once

#include <cstdint>

#include <embedded_util/clock.hpp>

uint32_t cpu_freq(void);
void delay(uint32_t counter);
void cpu_clock_init(void);

class FixedCoreClock : public Clock {
	public:
		FixedCoreClock(bool __ignore = true) {
			cpu_clock_init();
		}

		Frequency get_frequency() override {
			return frequency::Hz(cpu_freq());
		}

		void set_max_speed() {}
		void set_low_speed() {}
};
