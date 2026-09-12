/*
 * DAC80501M_SPI.c
 *
 *  Created on: Sep 9, 2026
 *      Author: SRIVATHSJAYAN
 */
#include "DAC80501M_SPI.h"

static HAL_StatusTypeDef DAC80501M_WriteReg(DAC80501M_Handle_t *hdac,DAC80501M_Register_t Reg,uint16_t Data);
/**
 * @brief  Initializes the DAC80501M device over SPI based on the handle configuration.
 * @details Validates the handle pointers and initialization parameters, then transmits
 *          configuration frames over the SPI bus to initialize the SYNC, CONFIG, and
 *          GAIN registers.
 *
 * @param[in] hdac Pointer to the DAC80501M handle structure containing the
 *                 SPI hardware linkage and initialization parameters.
 *
 * @retval HAL_OK       Initialization completed successfully over SPI.
 * @retval HAL_ERROR    A pointer is NULL, an initialization parameter is out of
 *                      the valid range, or an SPI communication failure occurred.
 * @retval HAL_BUSY     The SPI peripheral is currently busy.
 * @retval HAL_TIMEOUT  The SPI transmission timed out.
 */
HAL_StatusTypeDef DAC80501M_Init(DAC80501M_Handle_t *hdac)
{
	if(hdac == NULL || hdac->hspi == NULL || hdac->CS_Port == NULL)
	{
		return HAL_ERROR;
	}
	if(hdac->Init.int_ref > DAC80501M_INT_REF_DI ||
			hdac->Init.dac_sync > DAC80501M_SYNC_EN ||
			hdac->Init.buff_gain > DAC80501M_BUFF_GAIN_2 ||
			hdac->Init.ref_div > DAC80501M_REF_DIV_2)
	{
		return HAL_ERROR;
	}
	HAL_StatusTypeDef status;
	uint16_t cfg_reg = 0;
	if(hdac->Init.dac_sync == DAC80501M_SYNC_EN)
	{
		cfg_reg |= (uint16_t)(0x01U << __DAC80501M_SYNC_EN_POS);
	}

	status = DAC80501M_WriteReg(hdac,DAC80501M_REG_SYNC, cfg_reg);
	if(status != HAL_OK)
	{
		return status;
	}

	cfg_reg = 0;
	if(hdac->Init.int_ref == DAC80501M_INT_REF_DI)
	{
		cfg_reg |= (uint16_t)(0x01U << __DAC80501M_REF_PWDWN_POS);
	}
	status = DAC80501M_WriteReg(hdac,DAC80501M_REG_CONFIG, cfg_reg);
	if(status != HAL_OK)
	{
		return status;
	}

	cfg_reg = 0;
	if(hdac->Init.buff_gain == DAC80501M_BUFF_GAIN_2)
	{
		cfg_reg |= (0x01U << __DAC80501M_BUFF_GAIN_POS);
	}
	if(hdac->Init.ref_div == DAC80501M_REF_DIV_2)
	{
		cfg_reg |= (0x01U << __DAC80501M_REF_DIV_POS);
	}
	status = DAC80501M_WriteReg(hdac,DAC80501M_REG_GAIN, cfg_reg);

	return status;
}
/**
 * @brief  Performs a software reset on the DAC80501M device over SPI.
 * @details Validates the handle pointers, formats the soft-reset command payload,
 *          and transmits it to the TRIGGER register (05h) via SPI to initiate
 *          a power-on reset (POR) cycle.
 *
 * @param[in] hdac Pointer to the DAC80501M handle structure containing the
 *                 SPI hardware linkage and chip select configuration.
 *
 * @retval HAL_OK       Software reset command was successfully transmitted over SPI.
 * @retval HAL_ERROR    A pointer argument is NULL or an SPI communication failure occurred.
 * @retval HAL_BUSY     The SPI peripheral is currently busy.
 * @retval HAL_TIMEOUT  The SPI transmission timed out.
 */
HAL_StatusTypeDef DAC80501M_Soft_Reset(DAC80501M_Handle_t *hdac)
{
	if(hdac == NULL || hdac->hspi == NULL || hdac->CS_Port == NULL)
	{
		return HAL_ERROR;
	}
	uint16_t data = __DAC80501_TRIGGER_SOFTWARE_RESET << __DAC80501M_TRIGGER_SOFTRESET_POS;
	return DAC80501M_WriteReg(hdac, DAC80501M_REG_TRIGGER, data);
}
/**
 * @brief  Writes a 24-bit frame to a specified DAC80501M register over SPI.
 * @details Validates input arguments, formats a 3-byte payload comprising the 8-bit
 *          register address followed by the 16-bit data word (transmitted MSB-first),
 *          and asserts the Chip Select (SYNC) line low to transmit the frame over SPI.
 *          The CS line is de-asserted high immediately following transmission to latch
 *          the data into the device.
 *
 * @param[in] hdac Pointer to the DAC80501M handle structure containing the SPI instance
 *                 and Chip Select (SYNC) pin configuration.
 * @param[in] Reg  Target register address (type ::DAC80501M_Register_t).
 * @param[in] Data 16-bit data value to write into the register.
 *
 * @retval HAL_OK       Register write completed successfully.
 * @retval HAL_ERROR    A pointer argument is NULL, the register address is out of range,
 *                      or an SPI communication error occurred.
 * @retval HAL_BUSY     The SPI peripheral is currently busy.
 * @retval HAL_TIMEOUT  The SPI transmission timed out.
 */
static HAL_StatusTypeDef DAC80501M_WriteReg(DAC80501M_Handle_t *hdac,DAC80501M_Register_t Reg,uint16_t Data)
{
	if(hdac == NULL || hdac->hspi == NULL || hdac->CS_Port == NULL ||
			Reg > DAC80501M_REG_DAC_DATA || Reg < DAC80501M_REG_DEVID)
	{
		return HAL_ERROR;
	}
	HAL_StatusTypeDef status;
	uint8_t tx_buff[3];
	tx_buff[0] = (uint8_t)Reg;
	tx_buff[1] = (uint8_t)(Data>>8);
	tx_buff[2] = (uint8_t)Data;

	hdac->CS_Port->BSRR = ((uint32_t)hdac->CS_Pin << 16);

	status = HAL_SPI_Transmit(hdac->hspi,tx_buff, 3, HAL_MAX_DELAY);

	hdac->CS_Port->BSRR = ((uint32_t)hdac->CS_Pin);

	return status;
}
/**
 * @brief  Writes a 16-bit digital code to the DAC80501M output register over SPI.
 * @details Validates handle pointers and forwards the 16-bit data value to the
 *          DAC_DATA register (08h) via DAC80501M_WriteReg over the SPI bus.
 *          If DAC_SYNC_EN is disabled (default), the analog output updates
 *          immediately upon the rising edge of the Chip Select (SYNC) line.
 *
 * @param[in] hdac Pointer to the DAC80501M handle structure containing the
 *                 SPI instance and Chip Select (SYNC) pin configuration.
 * @param[in] Data 16-bit digital code (0 to 65535) to set the analog output voltage.
 *
 * @retval HAL_OK       Output data successfully written over SPI.
 * @retval HAL_ERROR    A pointer argument is NULL or an SPI communication failure occurred.
 * @retval HAL_BUSY     The SPI peripheral is currently busy.
 * @retval HAL_TIMEOUT  The SPI transmission timed out.
 */
HAL_StatusTypeDef DAC80501M_SPI_Write_DAC(DAC80501M_Handle_t *hdac,uint16_t Data)
{
	if(hdac == NULL || hdac->hspi == NULL || hdac->CS_Port == NULL)
	{
		return HAL_ERROR;
	}
	return DAC80501M_WriteReg(hdac, DAC80501M_REG_DAC_DATA, Data);
}
/**
 * @brief  Triggers an LDAC update on the DAC80501M over SPI.
 * @details Validates the handle pointers, formats the LDAC trigger command payload,
 *          and transmits it to the TRIGGER register (05h) via SPI to synchronously
 *          latch and update the DAC analog output with the contents of the DAC data buffer.
 *
 * @param[in] hdac Pointer to the DAC80501M handle structure containing the
 *                 SPI hardware linkage and chip select configuration.
 *
 * @retval HAL_OK       LDAC trigger command successfully transmitted over SPI.
 * @retval HAL_ERROR    A pointer argument is NULL or an SPI communication failure occurred.
 * @retval HAL_BUSY     The SPI peripheral is currently busy.
 * @retval HAL_TIMEOUT  The SPI transmission timed out.
 */
HAL_StatusTypeDef DAC80501M_Trigger_LADC(DAC80501M_Handle_t *hdac)
{
	if(hdac == NULL || hdac->hspi == NULL || hdac->CS_Port == NULL)
	{
		return HAL_ERROR;
	}
	uint16_t trgg = __DAC80501_TRIGGER_LDAC << __DAC80501M_TRIGGER_LDAC_POS;
	return DAC80501M_WriteReg(hdac, DAC80501M_REG_TRIGGER, trgg);
}
