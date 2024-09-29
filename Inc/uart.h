#ifndef UART_H
#define UART_H

#include <stdint.h>  // Para tipos de dados padrão
#include "stm32f1xx.h"  // Incluir registros STM32

// Definições de configuração da UART
#define CPU_CLK 8000000
//#define CPU_CLK 72000000
#define BaudRate 115200

// Funções da UART
void UART_Init(void);        // Inicializar a UART
void print_uart(const char *label);
void uart_write(uint8_t *ch);  // Enviar uma string via UART
void uart_print_int(const char *label, uint32_t value);  // Enviar inteiros via UART com uma label

#endif  // UART_H
