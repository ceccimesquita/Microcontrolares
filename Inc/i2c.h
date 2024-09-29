#ifndef I2C_H
#define I2C_H

#include "stm32f1xx.h"

// Macros para manipulação do endereço I2C
#define I2C_WRITE_ADDR(addr)  ((addr) << 1)       // Desloca o endereço de 7 bits e configura o bit R/W para escrita (0)
#define I2C_READ_ADDR(addr)   (((addr) << 1) | 1) // Desloca o endereço de 7 bits e configura o bit R/W para leitura (1)

// Funções da biblioteca I2C
void I2C_Init(void);

int I2C_Start(void);
int I2C_Stop(void);

void I2C_Write(uint8_t SlaveAddress, uint8_t *data, uint16_t size);
void I2C_Read(uint8_t SlaveAddress, uint8_t *data, uint16_t size);

uint8_t I2C_IsDeviceReady(uint8_t SlaveAddress, uint32_t Trials);

#endif // I2C_H
