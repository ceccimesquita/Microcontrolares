/*
 * bmp280.c
 *
 *  Created on: Sep 25, 2024
 *      Author: ceci
 */

// bmp280.c

#include "bmp280.h"
#include "i2c.h"
#include <stdio.h>

// Variáveis globais para os parâmetros de calibração
static uint16_t dig_T1;
static int16_t dig_T2, dig_T3;
static uint16_t dig_P1;
static int16_t dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;

// Variável global para t_fine, usada nos cálculos de temperatura e pressão
static int32_t t_fine;

void BMP280_Init(void) {
    uint8_t bmp280_id;
    uint8_t reg_addr = BMP280_REG_ID; // Endereço do registrador de ID do BMP280

    // Inicializar I2C
    I2C_Init();

    // Verificar se o BMP280 está pronto
    if (!I2C_IsDeviceReady(BMP280_ADDR, 10)) {

        return;
    }

    // Ler o ID do BMP280
    I2C_Write(BMP280_ADDR, &reg_addr, 1);
    I2C_Read(BMP280_ADDR, &bmp280_id, 1);

    // Verificar se o ID está correto (0x58 para BMP280)
    if (bmp280_id != 0x58) {

        return;
    }

    // Ler os dados de calibração
    BMP280_ReadCalibrationData();

    // Configurar o BMP280
    BMP280_Config();
}

void BMP280_ReadTemperatureAndPressure(int32_t *temperature, uint32_t *pressure) {
    int32_t adc_T = 0, adc_P = 0;

    // Ler os dados brutos de temperatura e pressão
    BMP280_ReadRawData(&adc_T, &adc_P);

    // Calcular a temperatura real em centésimos de grau Celsius
    *temperature = BMP280_CalculateTemperature(adc_T);

    // Calcular a pressão real em Pa
    *pressure = BMP280_CalculatePressure(adc_P);
}

void BMP280_ReadCalibrationData(void) {
    uint8_t calib_data[24];
    uint8_t reg_addr = BMP280_REG_CALIB00; // Endereço inicial dos dados de calibração

    I2C_Write(BMP280_ADDR, &reg_addr, 1);
    I2C_Read(BMP280_ADDR, calib_data, 24);

    // Converter os bytes lidos em valores de calibração
    dig_T1 = (uint16_t)(calib_data[1] << 8) | calib_data[0];
    dig_T2 = (int16_t)(calib_data[3] << 8) | calib_data[2];
    dig_T3 = (int16_t)(calib_data[5] << 8) | calib_data[4];
    dig_P1 = (uint16_t)(calib_data[7] << 8) | calib_data[6];
    dig_P2 = (int16_t)(calib_data[9] << 8) | calib_data[8];
    dig_P3 = (int16_t)(calib_data[11] << 8) | calib_data[10];
    dig_P4 = (int16_t)(calib_data[13] << 8) | calib_data[12];
    dig_P5 = (int16_t)(calib_data[15] << 8) | calib_data[14];
    dig_P6 = (int16_t)(calib_data[17] << 8) | calib_data[16];
    dig_P7 = (int16_t)(calib_data[19] << 8) | calib_data[18];
    dig_P8 = (int16_t)(calib_data[21] << 8) | calib_data[20];
    dig_P9 = (int16_t)(calib_data[23] << 8) | calib_data[22];
}

void BMP280_Config(void) {
    uint8_t config_data[2];

    // Configurar ctrl_meas (0xF4): temperatura e pressão com oversampling x1, modo normal
    config_data[0] = BMP280_REG_CTRL_MEAS; // Endereço do registrador ctrl_meas
    config_data[1] = 0x27; // 0b00100111
    // Bits [7:5] = 001 -> Oversampling x1 para pressão
    // Bits [4:2] = 001 -> Oversampling x1 para temperatura
    // Bits [1:0] = 11  -> Modo normal de operação
    I2C_Write(BMP280_ADDR, config_data, 2);

    // Configurar config (0xF5): filtro off, tempo de stand-by 1s
    config_data[0] = BMP280_REG_CONFIG; // Endereço do registrador config
    config_data[1] = 0xA0; // 0b10100000
    // Bits [7:5] = 101 -> Tempo de stand-by de 1s
    // Bits [4:2] = 000 -> Filtro desligado
    I2C_Write(BMP280_ADDR, config_data, 2);
}

void BMP280_ReadRawData(int32_t *adc_T, int32_t *adc_P) {
    uint8_t data[6];
    uint8_t reg_addr = BMP280_REG_PRESS_TEMP; // Endereço do registrador de dados (pressão e temperatura)

    // Ler 6 bytes de dados a partir do endereço
    I2C_Write(BMP280_ADDR, &reg_addr, 1);
    I2C_Read(BMP280_ADDR, data, 6);

    // Combinar os bytes para formar os valores brutos de pressão e temperatura
    *adc_P = ((int32_t)(data[0] << 16) | (int32_t)(data[1] << 8) | data[2]) >> 4;
    *adc_T = ((int32_t)(data[3] << 16) | (int32_t)(data[4] << 8) | data[5]) >> 4;
}

int32_t BMP280_CalculateTemperature(int32_t adc_T) {
    int32_t var1, var2, T;

    var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) *
             ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) *
             ((int32_t)dig_T3)) >> 14;

    t_fine = var1 + var2;

    T = (t_fine * 5 + 128) >> 8;

    return T; // Retorna a temperatura em centésimos de grau Celsius
}

uint32_t BMP280_CalculatePressure(int32_t adc_P) {
    int64_t var1, var2, p;

    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
    var2 = var2 + (((int64_t)dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;

    if (var1 == 0) {
        return 0; // Evitar divisão por zero
    }

    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;

    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);

    p = p / 256; // Converter para Pa

    return (uint32_t)p; // Retorna a pressão em Pa
}

