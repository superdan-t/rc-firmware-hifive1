#include <cstdio>
#include <cstdint>
#include <cstring>

#include <hifive1b_bsp/device_driver.hpp>

#include "uart.hpp"
#include "cpu.hpp"
#include "spi.hpp"
#include "led.hpp"

#define DELAY           20000000
#define BAUDRATE_115200 115200
#define SPICLOCK_80KHZ  80000
#define STR_LEN         256
#define BUF_LEN         2048
#ifdef __ICCRISCV__
#define fflush(a)
#endif

static char *tty_gets(char *str_p, uint32_t size);
static void get_ssid_pwd(char *ssid, char *pwd, uint32_t size);

static const uint32_t interactive = 1; // Set to 0 to use hardcoded SSID and pwd
static char wifi_ssid[STR_LEN] = "AndroidAPDE9B";
static char wifi_pwd[STR_LEN] = "isll3425";
static char at_cmd[STR_LEN*2];
static char recv_str[BUF_LEN];

void* operator new(size_t size) noexcept {
    auto new_region = malloc(size);
    if (!new_region) {
        // Halt and catch fire
        for (;;) {}
    }
    return new_region;
}

void operator delete(void* mem) noexcept {
    free(mem);
}

int wifi_main()
{
    // Create board driver without SPI drivers since they are managed manually by this app
    hifive1b::Hifive1B<MetalUartStream, hifive1b::EmptySpiDriver> board_driver;

    auto& status_led = board_driver.get_led_driver();
    status_led.set(1, 0, 0);

    auto& hfclk = board_driver.get_clock_driver();

    uart_init(BAUDRATE_115200, hfclk);

    hfclk.add_frequency_change_listener([&board_driver](Frequency new_frequency) {
        uart_init(BAUDRATE_115200, board_driver.get_clock_driver());
        printf("---- CPU Frequency Update: %i MHz\r\n", static_cast<int>(std::chrono::duration_cast<frequency::MHz>(new_frequency).count()));
    });

    printf("---- HiFive1 Rev B WiFi Demo --------\r\n");
    printf("* UART: 115200 bps\r\n");
    printf("* SPI: 80 KHz\r\n");
    printf("* CPU: %i MHz\r\n", static_cast<int>(std::chrono::duration_cast<frequency::MHz>(hfclk.get_frequency()).count()));
    fflush(stdout);

    spi_init(SPICLOCK_80KHZ);

    status_led.set(0, 0, 1);

    printf("[+] ESP32 reset\r\n");
    spi_send("AT+RST\r\n");

    // Set WiFi Station Mode
    spi_send("AT+CWMODE=1\r\n");

    status_led.set(0, 1, 0);

    printf("* Optional: Enter AT commands (see \"ESP32 AT Instruction Set and Examples\")\r\n");
    while(1) {
        if (TRANS_ON == spi_transparent()) {
            printf("* ----> ");
        } else {
            printf("* Enter AT command: ");
        }
        fflush(stdout);
        while (NULL == tty_gets(wifi_ssid, sizeof(wifi_ssid))) {}
        printf("\r\n");
        snprintf(at_cmd, sizeof(at_cmd), "%s\r\n", wifi_ssid);
        spi_send(at_cmd);
        if (TRANS_OFF == spi_transparent()) {
            spi_recv(recv_str, sizeof(recv_str));
        }
    }
}

//----------------------------------------------------------------------
// A gets function that stops for \n AND \r (good with PUTTY)
//----------------------------------------------------------------------
static char *tty_gets(char *str_p, uint32_t size)
{
    uint32_t i;
    int32_t c;

    for (i = 0; i < size; i++) {
        c = uart_getchar();
        if (c == '\n' || c == '\r') {
            str_p[i] = '\0';
            return str_p;
        } else if (c > 0xFF) {
            return NULL;
        }
        str_p[i] = c;
    }

    if (size > 0) {
        str_p[size-1] = '\0';
    }
    return str_p;
}


//----------------------------------------------------------------------
// Hack to read ssid and pwd from terminal
//----------------------------------------------------------------------
static void get_ssid_pwd(char *ssid, char *pwd, uint32_t size)
{
    printf("Greetings!\r\n");
    printf("Enter SSID: ");
    fflush(stdout);
    while (NULL == tty_gets(ssid, size)) {}
    printf("\r\n");

    printf("Enter Password: ");
    fflush(stdout);
    while (NULL == tty_gets(pwd, size)) {}
    printf("\r\n");
}
