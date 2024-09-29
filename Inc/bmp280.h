/*
 * bmp280.h
 *
 *  Created on: Sep 25, 2024
 *      Author: ceci
 */

#ifndef BMP280_H
#define BMP280_H

#include <stdint.h>

// Definições de endereço I2C
#define BMP280_ADDR 					((uint8_t)0x77)
#define BMP280_REG_ID                   ((uint8_t)0xD0) // Chip ID
#define BMP280_REG_CALIB00              ((uint8_t)0x88) // Calibration data calib00
#define BMP280_REG_CTRL_MEAS            ((uint8_t)0xF4) // Pressure and temperature measure control register
#define BMP280_REG_CONFIG               ((uint8_t)0xF5) // Configuration register
#define BMP280_REG_PRESS_TEMP           ((uint8_t)0xF7)

// Inicializa o BMP280 (inclui leitura de ID, calibração e configuração)
void BMP280_Init(void);

// Lê temperatura e pressão, retornando a temperatura em centésimos de graus Celsius
// e a pressão em Pa
void BMP280_ReadTemperatureAndPressure(int32_t *temperature, uint32_t *pressure);

// Lê os dados de calibração do BMP280
void BMP280_ReadCalibrationData(void);

// Configura o BMP280 (oversampling, modo de operação, etc.)
void BMP280_Config(void);

// Lê os dados brutos de temperatura e pressão do BMP280
void BMP280_ReadRawData(int32_t *adc_T, int32_t *adc_P);

// Calcula a temperatura em centésimos de graus Celsius a partir dos dados brutos de temperatura
int32_t BMP280_CalculateTemperature(int32_t adc_T);

// Calcula a pressão em Pa a partir dos dados brutos de pressão
uint32_t BMP280_CalculatePressure(int32_t adc_P);

#endif // BMP280_H
