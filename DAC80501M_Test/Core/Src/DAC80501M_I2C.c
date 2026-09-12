/*
 * DAC80501M_I2C.c
 *
 *  Created on: Sep 9, 2026
 *      Author: SRIVATHSJAYAN
 */
#include "DAC80501M_I2C.h"

static HAL_StatusTypeDef DAC80501M_WriteReg(DAC80501M_Handle_t *hdac, DAC80501M_Register_t Reg, uint16_t Data);
/**
 * @brief  Initializes the DAC80501M device over I2C based on the handle configuration.
 * @details Validates handle pointers and initialization parameters, configures
 *          the SYNC and CONFIG registers via I2C, and verifies that the internal
 *          reference alarm (REF-ALARM) is not asserted.
 *
 * @param[in,out] hdac Pointer to the DAC80501M handle structure containing the
 *                     I2C peripheral instance, target address, and user settings.
 *
 * @retval HAL_OK    Device initialized successfully with no active reference alarm.
 * @retval HAL_ERROR A pointer argument is NULL, the I2C address is 0, an initialization
 *                   parameter is out of range, or an I2C communication failure occurred.
 * @retval 0x99      A reference alarm is active (headroom violation or reference fault),
 *                   or a communication failure occurred while polling the alarm status.
 */
HAL_StatusTypeDef DAC80501M_Init(DAC80501M_Handle_t *hdac)
{
	if(hdac == NULL || hdac->hi2c == NULL || hdac->Init.dev_i2c_addr == 0)
	{
		return HAL_ERROR;
	}
	if(hdac->Init.int_ref > DAC80501M_INT_REF_DI || hdac->Init.dac_sync > DAC80501M_SYNC_EN)
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
	if(DAC80501M_Get_Ref_Alarm(hdac) == 1 || DAC80501M_Get_Ref_Alarm(hdac) == 0x99)
	{
		return 0x99;
	}
	else
	{
		status = HAL_OK;
	}

	return status;

}
/**
 * @brief  Writes a 16-bit value to a specified DAC80501M register over I2C.
 * @details Validates the handle pointers, I2C address, and register range,
 *          then packs the 16-bit data word (transmitted MSB-first) and uses
 *          HAL_I2C_Mem_Write to transmit the 8-bit register address followed
 *          by the 2 data bytes to the DAC80501M device.
 *
 * @param[in] hdac Pointer to the DAC80501M handle structure containing the
 *                 I2C peripheral instance and target device address.
 * @param[in] Reg  Target register address (type ::DAC80501M_Register_t).
 * @param[in] Data 16-bit data value to write into the register.
 *
 * @retval HAL_OK       Register write completed successfully over I2C.
 * @retval HAL_ERROR    A pointer argument is NULL, device I2C address is 0,
 *                      the register address is out of range, or an I2C communication
 *                      failure occurred.
 * @retval HAL_BUSY     The I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  The I2C transmission timed out.
 */
static HAL_StatusTypeDef DAC80501M_WriteReg(DAC80501M_Handle_t *hdac,DAC80501M_Register_t Reg,uint16_t Data)
{
	if(hdac == NULL ||
			hdac->hi2c == NULL ||
			hdac->Init.dev_i2c_addr == 0 ||
			Reg > DAC80501M_REG_DAC_DATA ||
			Reg < DAC80501M_REG_DEVID)
	{
		return HAL_ERROR;
	}
	uint8_t tx_buff[2];
	tx_buff[0] = (uint8_t)(Data>>8);
	tx_buff[1] = (uint8_t)Data;
	return HAL_I2C_Mem_Write(hdac->hi2c, hdac->Init.dev_i2c_addr, Reg, 1, tx_buff, 2, 1000);
}
/**
 * @brief  Reads a 16-bit register from the DAC80501M over I2C.
 * @details Validates the handle pointers, I2C address, and destination buffer,
 *          then reads 2 consecutive data bytes (MSDB and LSDB) from the specified
 *          target register using HAL_I2C_Mem_Read. The bytes are then recombined
 *          into a single 16-bit word.
 *
 * @param[in,out] hdac  Pointer to the DAC80501M handle structure containing the
 *                      I2C peripheral instance and target device address.
 * @param[in]     Reg   Target register address to read from (type ::DAC80501M_Register_t).
 * @param[out]    pData Pointer to a 16-bit variable where the reconstructed register
 *                      value will be written. Must not be NULL.
 *
 * @retval HAL_OK       Register read completed successfully over I2C.
 * @retval HAL_ERROR    A pointer argument is NULL, the device I2C address is 0,
 *                      or an I2C communication failure occurred.
 * @retval HAL_BUSY     The I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  The I2C read operation timed out.
 */
HAL_StatusTypeDef DAC80501M_ReadReg(DAC80501M_Handle_t *hdac,DAC80501M_Register_t Reg,uint16_t *pData)
{
	if(hdac == NULL || hdac->hi2c == NULL || hdac->Init.dev_i2c_addr == 0 || pData == NULL)
	{
		return HAL_ERROR;
	}
	HAL_StatusTypeDef status;
	uint8_t rx_buff[2];
	status = HAL_I2C_Mem_Read(hdac->hi2c,hdac->Init.dev_i2c_addr,Reg,1,rx_buff,2, HAL_MAX_DELAY);
	if(status != HAL_OK)
	{
		return status;
	}

	*pData = ((uint16_t)rx_buff[0] << 8 | rx_buff[1]);

	return status;
}
/**
 * @brief  Reads the reference alarm (REF-ALARM) flag from the DAC80501M over I2C.
 * @details Reads the 16-bit STATUS register (07h) and isolates bit 0 (REF-ALARM)
 *          to determine if a reference fault or headroom violation condition exists.
 *
 * @param[in,out] hdac Pointer to the DAC80501M handle structure containing the
 *                     I2C peripheral instance and target device address.
 *
 * @retval 0x01   The REF-ALARM flag is asserted (reference fault or headroom violation).
 * @retval 0x00   The REF-ALARM flag is cleared (normal operation).
 * @retval 0x99   Input validation failed (NULL pointer or zero I2C address) or
 *                an I2C communication error occurred while reading the register.
 */
uint8_t DAC80501M_Get_Ref_Alarm(DAC80501M_Handle_t *hdac)
{
	if(hdac == NULL || hdac->hi2c == NULL || hdac->Init.dev_i2c_addr == 0)
	{
		return 0x99;
	}
	HAL_StatusTypeDef status;
	uint16_t data;
	status = DAC80501M_ReadReg(hdac, DAC80501M_REG_STATUS, &data);
	if(status != HAL_OK)
	{
		return 0x99;
	}
	return (data & 0x01U);
}
/**
 * @brief  Performs a software reset on the DAC80501M device over I2C.
 * @details Validates handle pointers and the target device address, formats
 *          the soft-reset command payload, and transmits it to the TRIGGER
 *          register (05h) via I2C to initiate a power-on reset (POR) cycle.
 *
 * @param[in,out] hdac Pointer to the DAC80501M handle structure containing the
 *                     I2C peripheral instance and target device address.
 *
 * @retval HAL_OK       Software reset command was successfully transmitted over I2C.
 * @retval HAL_ERROR    A pointer argument is NULL, the I2C address is 0, or
 *                      an I2C communication error occurred.
 * @retval HAL_BUSY     The I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  The I2C transmission timed out.
 */
HAL_StatusTypeDef DAC80501M_Soft_Reset(DAC80501M_Handle_t *hdac)
{
	if(hdac == NULL || hdac->hi2c == NULL || hdac->Init.dev_i2c_addr == 0)
	{
		return HAL_ERROR;
	}
	uint16_t tx = __DAC80501_TRIGGER_SOFTWARE_RESET << __DAC80501M_SOFTRESET_POS;

	return DAC80501M_WriteReg(hdac, DAC80501M_REG_TRIGGER,tx);
}
/**
 * @brief  Configures the internal output buffer gain of the DAC80501M over I2C.
 * @details Reads the current contents of the GAIN register (04h) to preserve
 *          other settings (such as the reference divider), modifies the BUFF-GAIN
 *          bit (bit 0) according to the requested gain mode, and writes the updated
 *          16-bit configuration word back to the device over I2C.
 *
 * @param[in,out] hdac              Pointer to the DAC80501M handle structure containing
 *                                  the I2C peripheral instance and target device address.
 * @param[in]     Dac80501_Buff_Gain Buffer gain setting (e.g., ::DAC80501M_BUFF_GAIN_1
 *                                  for 1x gain or ::DAC80501M_BUFF_GAIN_2 for 2x gain).
 *
 * @retval HAL_OK       Output buffer gain successfully configured over I2C.
 * @retval HAL_ERROR    A pointer argument is NULL, the target I2C address is 0,
 *                      the requested gain setting exceeds DAC80501M_BUFF_GAIN_2,
 *                      or an I2C communication error occurred during read/write.
 * @retval HAL_BUSY     The I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  The I2C read or write operation timed out.
 */
HAL_StatusTypeDef DAC80501M_Set_Buff_Gain(DAC80501M_Handle_t *hdac, uint8_t Dac80501_Buff_Gain)
{
	if(hdac == NULL || hdac->hi2c == NULL || hdac->Init.dev_i2c_addr == 0 || Dac80501_Buff_Gain > DAC80501M_BUFF_GAIN_2)
	{
		return HAL_ERROR;
	}
	HAL_StatusTypeDef status;
	uint16_t config = 0;

	status = DAC80501M_ReadReg(hdac, DAC80501M_REG_GAIN, &config);
	if(status != HAL_OK)
	{
		return status;
	}
	if(Dac80501_Buff_Gain == DAC80501M_BUFF_GAIN_1)
	{
		config &= ~(0x1U << __DAC80501M_BUFF_GAIN_POS);
	}
	else
	{
		config |= (0x1U << __DAC80501M_BUFF_GAIN_POS);
	}
	status = DAC80501M_WriteReg(hdac, DAC80501M_REG_GAIN, config);

	return status;
}
/**
 * @brief  Configures the reference divider setting of the DAC80501M over I2C.
 * @details Reads the current contents of the GAIN register (04h) to preserve
 *          other configuration bits (such as the buffer gain), updates the
 *          REF-DIV bit according to the requested mode, and writes the
 *          updated configuration back to the device over I2C.
 *
 * @param[in,out] hdac             Pointer to the DAC80501M handle structure containing
 *                                 the I2C peripheral instance and target device address.
 * @param[in]     Dac80501_Ref_Div Reference divider mode (e.g., ::DAC80501M_REF_DIV_1
 *                                 for undivided reference or ::DAC80501M_REF_DIV_2
 *                                 to divide the reference by 2).
 *
 * @retval HAL_OK       Reference divider setting successfully configured over I2C.
 * @retval HAL_ERROR    A pointer argument is NULL, the target I2C address is 0,
 *                      the requested divider mode exceeds DAC80501M_REF_DIV_2,
 *                      or an I2C communication error occurred during read/write.
 * @retval HAL_BUSY     The I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  The I2C read or write operation timed out.
 */
HAL_StatusTypeDef DAC80501M_Set_Ref_Div(DAC80501M_Handle_t *hdac, uint8_t Dac80501_Ref_Div)
{
	if(hdac == NULL || hdac->hi2c == NULL || hdac->Init.dev_i2c_addr == 0 || Dac80501_Ref_Div > DAC80501M_REF_DIV_2)
	{
		return HAL_ERROR;
	}
	HAL_StatusTypeDef status;
	uint16_t config = 0;

	status = DAC80501M_ReadReg(hdac, DAC80501M_REG_GAIN, &config);
	if(status != HAL_OK)
	{
		return status;
	}
	if(Dac80501_Ref_Div == DAC80501M_REF_DIV_2)
	{
		config |= (0x1U << __DAC80501M_REF_DIV_POS);
	}
	else
	{
		config &= ~(0x1U << __DAC80501M_REF_DIV_POS);
	}
	status = DAC80501M_WriteReg(hdac, DAC80501M_REG_GAIN, config);

	return status;
}
/**
 * @brief  Triggers an asynchronous LDAC update on the DAC80501M over I2C.
 * @details Formats the LDAC trigger command payload and writes it to the
 *          TRIGGER register (05h) via I2C to synchronously latch and update
 *          the DAC analog output with the contents of the DAC data buffer.
 *
 * @param[in,out] hdac Pointer to the DAC80501M handle structure containing the
 *                     I2C peripheral instance and target device address.
 *
 * @retval HAL_OK       LDAC command successfully transmitted over I2C.
 * @retval HAL_ERROR    A pointer argument is NULL, the I2C address is 0, or
 *                      an I2C communication error occurred.
 * @retval HAL_BUSY     The I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  The I2C write operation timed out.
 */
HAL_StatusTypeDef DAC80501M_Trigger_LADC(DAC80501M_Handle_t *hdac)
{
	uint16_t trgg = __DAC80501_TRIGGER_LDAC << __DAC80501M_LDAC_POS;

	return DAC80501M_WriteReg(hdac, DAC80501M_REG_TRIGGER, trgg);
}
/**
 * @brief  Writes a 16-bit digital code to the DAC80501M output register over I2C.
 * @details Validates the handle pointers and device address, then packs the 16-bit
 *          digital code (MSB first) and transmits it to the DAC_DATA register (08h)
 *          using HAL_I2C_Mem_Write. If DAC_SYNC_EN is disabled (default), the analog
 *          output updates immediately upon transaction completion.
 *
 * @param[in,out] hdac Pointer to the DAC80501M handle structure containing the
 *                     I2C peripheral instance and target device address.
 * @param[in]     Data 16-bit digital code (0 to 65535) to set the analog output voltage.
 *
 * @retval HAL_OK       Output data successfully written to the DAC register over I2C.
 * @retval HAL_ERROR    A pointer argument is NULL, the I2C address is 0, or
 *                      an I2C communication error occurred.
 * @retval HAL_BUSY     The I2C peripheral is currently busy.
 * @retval HAL_TIMEOUT  The I2C transmission timed out.
 */
HAL_StatusTypeDef DAC80501M_I2C_Write_DAC(DAC80501M_Handle_t *hdac,uint16_t Data)
{
	if(hdac == NULL || hdac->hi2c == NULL || hdac->Init.dev_i2c_addr == 0)
	{
		return HAL_ERROR;
	}
	uint8_t tx_buff[2];
	tx_buff[0] = (uint8_t)(Data>>8);
	tx_buff[1] = (uint8_t)Data;
	return HAL_I2C_Mem_Write(hdac->hi2c, hdac->Init.dev_i2c_addr, DAC80501M_REG_DAC_DATA, 1, tx_buff, 2, HAL_MAX_DELAY);
}
