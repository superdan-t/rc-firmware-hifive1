
/*
 * Apps:
 * 	HELLO_APP	- SiFive Hello World application
 *  WIFI_APP	- WiFi demo application
 *  ESP32_AT_APP	- ESP32 AT command set testing
 */
#define WIFI_APP 1

typedef int (*const main_fn_ptr)(void);

#ifdef HELLO_APP

#include "hello.hpp"
static main_fn_ptr app_entry {&hello_main};

#elif defined WIFI_APP

#include "wifi_app.hpp"
static main_fn_ptr app_entry {&wifi_main};

#elif defined ESP32_AT_APP

#include "esp32_at_app.hpp"
static main_fn_ptr app_entry {&esp32_at_main};

#else

#	error Please select a startup app.

#endif

int main(void) {
	return app_entry();
}