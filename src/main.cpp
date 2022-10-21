#include <cstdio>

#include <hifive1_revb_memory_map.hpp>

int main() {
	printf("Metal OS boot start\n");

	// Test if SPI persistent memory works
	int* my_int {reinterpret_cast<int*>(hifive1_bsp::QSPI_NVMEM_REGION_START)};

	printf("Value of my_int upon boot: %i\n", *my_int);
	++ *my_int;
	printf("my_int incremented by 1\n");
	
	for (;;);
}