#include "i2c.h"
#include "uart.h"
#include "stm32f1xx.h"

//#define debugON

// Função para inicializar o I2C
void I2C_Init(void) {
    // Ativar clocks para I2C e GPIO
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;  // Ativar GPIOB (SCL, SDA)
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;  // Ativar I2C1

    // Reset I2C1 para estado inicial
    I2C1->CR1 |= I2C_CR1_SWRST;
    I2C1->CR1 &= ~I2C_CR1_SWRST;

    // Configurar PB6 (SCL) e PB7 (SDA) para I2C em modo alternativo open-drain
    GPIOB->CRL &= ~(GPIO_CRL_MODE6 | GPIO_CRL_MODE7 | GPIO_CRL_CNF6 | GPIO_CRL_CNF7);
    GPIOB->CRL |= (GPIO_CRL_MODE6_1 | GPIO_CRL_MODE7_1) | (GPIO_CRL_CNF6 | GPIO_CRL_CNF7);  // 2 MHz, função alternada open-drain

    // Configurar I2C: clock e tempo de subida
    I2C1->CR2 = 8;  // Freq do clock (MHz)
    I2C1->CCR = 40; // Clock para 100 kHz (para 8 MHz)
    I2C1->TRISE = 9; // Tempo de subida
    I2C1->CR1 |= I2C_CR1_PE;  // Habilitar I2C1
}

// Função para gerar condição de start
int I2C_Start(void) {
    I2C1->CR1 |= I2C_CR1_START;  // Gerar condição de start
    uint32_t timeout = 10000;
    while (!(I2C1->SR1 & I2C_SR1_SB) && timeout--) {}
    if (timeout == 0) {
#ifdef debugON
        print_uart("Erro: Timeout em I2C_Start\n\r");
#endif
        return -1;
    } else {
#ifdef debugON
        print_uart("I2C_Start OK\n\r");
#endif
        return 0;
    }
}

// Função para gerar condição de stop
int I2C_Stop(void) {
    I2C1->CR1 |= I2C_CR1_STOP;  // Gerar condição de stop
    // Aguardar até que o bit STOP seja limpo pelo hardware
    uint32_t timeout = 10000;
    while ((I2C1->CR1 & I2C_CR1_STOP) && timeout--) {}
    if (timeout == 0) {
        print_uart("Erro: Timeout em I2C_Stop\n\r");
        return -1;
    } else {
#ifdef debugON
        print_uart("I2C_Stop OK\n\r");
#endif
        return 0;
    }
}

// Função para escrever dados no barramento I2C
void I2C_Write(uint8_t SlaveAddress, uint8_t *data, uint16_t size) {
    uint32_t timeout;

    // 1. Gerar condição de início (START)
    if (I2C_Start() != 0) {
        // Falha ao gerar START
        return;
    }

    // 2. Enviar endereço do dispositivo escravo com bit de escrita (0)
    I2C1->DR = I2C_WRITE_ADDR(SlaveAddress); // Usar macro para endereço de escrita

    // 3. Aguardar até que o bit ADDR seja definido (endereço enviado e reconhecido)
    timeout = 10000;
    while (!(I2C1->SR1 & (I2C_SR1_ADDR | I2C_SR1_AF)) && timeout--) {}
    if (timeout == 0 || (I2C1->SR1 & I2C_SR1_AF)) {
        print_uart("Erro: Endereço não reconhecido em I2C_Write\n\r");
        I2C1->SR1 &= ~I2C_SR1_AF; // Limpar flag de falha de acknowledge
        I2C_Stop(); // Gerar condição de parada em caso de erro
        return;
    }

    // Limpar registros SR1 e SR2 lendo-os
    (void)I2C1->SR1;
    (void)I2C1->SR2;

    // 4. Enviar os dados
    while (size--) {
        // Escrever dado no registro de dados
        I2C1->DR = *data++;


        timeout = 10000;
        while (!(I2C1->SR1 & I2C_SR1_BTF) && timeout--) {}
        if (timeout == 0) {
            print_uart("Erro: Timeout em I2C_Write durante envio de dados\n\r");
            I2C_Stop(); // Gerar condição de parada em caso de erro
            return;
        }
    }

    // 5. Gerar condição de parada (STOP)
    I2C_Stop();
#ifdef debugON
    print_uart("I2C_Write OK\n\r");
#endif
}

// Função para ler dados do barramento I2C
void I2C_Read(uint8_t SlaveAddress, uint8_t *data, uint16_t size) {
    uint32_t timeout;

    if (size == 0) return;

    // 1. Gerar condição de início (START)
    if (I2C_Start() != 0) {
        // Falha ao gerar START
        return;
    }

    // 2. Enviar endereço do dispositivo escravo com bit de leitura (1)
    I2C1->DR = I2C_READ_ADDR(SlaveAddress); // Usar macro para endereço de leitura

    // 3. Aguardar até que o bit ADDR seja definido (endereço enviado e reconhecido)
    timeout = 10000;
    while (!(I2C1->SR1 & (I2C_SR1_ADDR | I2C_SR1_AF)) && timeout--) {}
    if (timeout == 0 || (I2C1->SR1 & I2C_SR1_AF)) {
        print_uart("Erro: Endereço não reconhecido em I2C_Read\n\r");
        I2C1->SR1 &= ~I2C_SR1_AF; // Limpar flag de falha de acknowledge
        I2C_Stop(); // Gerar condição de parada em caso de erro
        return;
    }

    // Limpar registros SR1 e SR2
    (void)I2C1->SR1;
    (void)I2C1->SR2;

    // 4. Configurar ACK e NACK conforme o número de bytes a receber
    if (size == 1) {
        // Desabilitar ACK para receber apenas um byte
        I2C1->CR1 &= ~I2C_CR1_ACK;
        // Gerar condição de parada
        I2C_Stop();
    } else {
        // Habilitar ACK para receber múltiplos bytes
        I2C1->CR1 |= I2C_CR1_ACK;
    }

    // 5. Ler os dados
    while (size--) {
        // Aguardar até que o bit RXNE (Receive Buffer Not Empty) seja definido
        timeout = 10000;
        while (!(I2C1->SR1 & I2C_SR1_RXNE) && timeout--) {}
        if (timeout == 0) {
            print_uart("Erro: Timeout em I2C_Read durante recepção de dados\n\r");
            I2C_Stop(); // Gerar condição de parada em caso de erro
            return;
        }

        // Ler dado do registro de dados
        *data++ = I2C1->DR;

        // Se restar apenas um byte, desabilitar ACK e gerar STOP
        if (size == 1) {
            I2C1->CR1 &= ~I2C_CR1_ACK;
            I2C_Stop();
        }
    }
#ifdef debugON
    print_uart("I2C_Read OK\n\r");
#endif
}

// Função para verificar se o dispositivo está pronto
uint8_t I2C_IsDeviceReady(uint8_t SlaveAddress, uint32_t Trials) {
    uint32_t timeout;
    uint16_t attempts = 0;

    while (attempts < Trials) {
        // Gerar condição de início (START)
        if (I2C_Start() != 0) {
            attempts++;
            continue;
        }

        // Enviar endereço do dispositivo escravo com bit de escrita (0)
        I2C1->DR = I2C_WRITE_ADDR(SlaveAddress); // Usar macro para endereço de escrita

        // Aguardar até que o bit ADDR ou AF seja definido
        timeout = 10000;
        while (!(I2C1->SR1 & (I2C_SR1_ADDR | I2C_SR1_AF)) && timeout--) {}
        if (timeout == 0) {
            I2C_Stop();
            attempts++;
            continue;
        }

        if (I2C1->SR1 & I2C_SR1_ADDR) {
            // Dispositivo respondeu
            // Limpar registros SR1 e SR2
            (void)I2C1->SR1;
            (void)I2C1->SR2;
            I2C_Stop();
#ifdef debugON
            print_uart("Dispositivo encontrado\n\r");
#endif
            return 1; // Dispositivo está pronto
        } else if (I2C1->SR1 & I2C_SR1_AF) {
            // Falha ao reconhecer endereço, limpar flag AF
            I2C1->SR1 &= ~I2C_SR1_AF;
            print_uart("Falha ao reconhecer endereço\n\r");
            I2C_Stop();
        }

        attempts++;
    }

    print_uart("Dispositivo não está pronto após múltiplas tentativas\n\r");
    return 0; // Dispositivo não está pronto
}
