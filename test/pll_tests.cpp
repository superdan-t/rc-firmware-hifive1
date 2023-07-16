/// Tests for the PLL Driver

#include <chrono>
#include <thread>

#include <gtest/gtest.h>

#include <hifive1b_bsp/devices/pll.hpp>

/// This is the value of the pllcfg register after the SiFive bootloader
constexpr uint32_t DEFAULT_PLLCFG = 0x70DF1;
constexpr uint32_t LOCK_MASK = 0x80000000;

static hifive1b::Pll::ConfigStatus get_320mhz_config() {
	hifive1b::Pll::ConfigStatus cfg;

	cfg.reference_select = hifive1b::Pll::ReferenceClock::HFXOSC;
	cfg.R = 2;
	cfg.F = 80;
	cfg.Q = 2;
	cfg.bypass = false;

	return cfg;
}

TEST(PllDriverTests, ConfigSetTest) {

	// The correct value of PLLCFG configured for 320 MHz without the lock bit set
	constexpr uint32_t PLLCFG_RATED_SPEED = 0x30671;

	// Set the lock bit to prevent the test from hanging since there's no actual hardware
	uint32_t mock_pllcfg_reg = DEFAULT_PLLCFG | LOCK_MASK;
	hifive1b::Pll pll(reinterpret_cast<uintptr_t>(&mock_pllcfg_reg));
	pll.configure_and_select(get_320mhz_config());

	EXPECT_EQ(mock_pllcfg_reg, LOCK_MASK | PLLCFG_RATED_SPEED);

}

TEST(PllDriverTests, WaitLockTest) {
	volatile uint32_t mock_pllcfg_reg = DEFAULT_PLLCFG;
	hifive1b::Pll pll(reinterpret_cast<uintptr_t>(&mock_pllcfg_reg));

	// Wait 150 ms to send a mock lock signal
	std::thread t1([&mock_pllcfg_reg]() {
		std::this_thread::sleep_for(std::chrono::milliseconds(150));
		mock_pllcfg_reg |= LOCK_MASK;
	});

	// Make sure the set function waits for the lock signal
	auto start_time = std::chrono::steady_clock::now();
	pll.configure_and_select(get_320mhz_config());
	auto duration = std::chrono::steady_clock::now() - start_time;

	auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

	t1.join();

	EXPECT_GE(duration_ms, 150);

}
