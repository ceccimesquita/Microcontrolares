#include <stdio.h>
#include <stdint.h>
#include "i2c.h"
#include "uart.h"
#include "bmp280.h"
#include "stm32f1xx.h"

void SysTick_Init(void) {
    // Assumindo que o clock do sistema é 72 MHz
    SysTick->LOAD = 8000 - 1; // Carrega o valor para 1 ms
    SysTick->VAL = 0;          // Limpa o valor atual
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
}

void Delay_ms(uint32_t ms) {
    for (uint32_t i = 0; i < ms; i++) {
        // Aguarda até que o contador atinja zero
        while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));
    }
}

int main(void) {
    // Inicializar UART
    UART_Init();

    SysTick_Init();

    // Inicializar o BMP280
    BMP280_Init();

    print_uart("Inicialização concluída\n\r");

    // Variáveis para armazenar as leituras
    int32_t temperature = 0;
    uint32_t pressure = 0;
    char buffer[100];

    // Loop principal para ler dados continuamente
    while (1) {
        // Ler temperatura e pressão
        BMP280_ReadTemperatureAndPressure(&temperature, &pressure);

        // Formatar e imprimir os resultados
        sprintf(buffer, "Temperatura: %ld.%02ld °C\n\r", temperature / 100, temperature % 100);
        print_uart(buffer);

        sprintf(buffer, "Pressão: %lu.%02lu hPa\n\r", pressure / 100, pressure % 100);
        print_uart(buffer);

        // Atraso entre as leituras
        Delay_ms(100);
    }

    return 0;
}
