/*
 * Kamera.h
 *
 *  Created on: 2023. gada 27. janv.
 *      Author: emils
 */

#ifndef SRC_KAMERA_H_
#define SRC_KAMERA_H_

#include "main.h"

//#define SPI2_CS_Pin MC_CS_Pin
#define SPI2_CS_GPIO_Port GPIOB

void INIT_SPI(SPI_HandleTypeDef *spiPointer);
HAL_StatusTypeDef Kamera_TransmitReceive(
	uint8_t *pTxData,
	uint8_t *pRxData,
	uint16_t Size,
	uint32_t Timeout
);

uint8_t bus_read(int address);
uint8_t bus_write(int address,int value);
//uint8_t read_reg(uint8_t addr);
//void write_reg(uint8_t addr, uint8_t data);

#endif /* SRC_KAMERA_H_ */
