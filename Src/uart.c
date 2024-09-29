#include "uart.h"
#include <stdio.h>  // Para sprintf

uint8_t *ptr;  // Ponteiro global para a função de transmissão

// Inicializar a UART
void UART_Init(void) {
    /* Habilitar o clock para GPIOA */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

    /* Configurar PA9 (TX) como saída*/
    GPIOA->CRH &= 0xFFFFFF0F;
    GPIOA->CRH |= 0x000000B0;

    /* Habilitar o clock para USART1 */
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    /* Configurar o baud rate da UART */
    USART1->BRR = ((CPU_CLK + (BaudRate / 2U)) / BaudRate);

    /* Habilitar a transmissão e a UART */
    USART1->CR1 |= USART_CR1_TE;
    USART1->CR1 |= USART_CR1_UE;
}

// Função para enviar uma string simples pela UART
void print_uart(const char *label) {
    uart_write((uint8_t*)label);  // Converte para uint8_t* e envia via UART
}

// Função para enviar uma string pela UART
void uart_write(uint8_t *ch) {
    while (*ch) {
        // Aguarde até que o registrador de transmissão esteja vazio
        while (!(USART1->SR & USART_SR_TXE)) {}
        // Enviar o byte atual
        USART1->DR = (*ch & 0xFF);
        ch++;  // Avançar para o próximo caractere
    }
}

// Função para enviar um número inteiro com uma label via UART
void uart_print_int(const char *label, uint32_t value) {
    char buffer[50];
    sprintf(buffer, "%s: %lu\n\r", label, value);
    uart_write((uint8_t*)buffer);
}
