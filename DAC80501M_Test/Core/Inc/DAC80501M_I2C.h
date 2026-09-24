/*
 *  @file DAC80501M_I2C.h
 *  @date 09 Sep 2026
 * 	@author: SRIVATHSJAYAN
 */

#ifndef INC_DAC80501M_I2C_H_
#define INC_DAC80501M_I2C_H_

#include "main.h"

typedef enum
{
	DAC80501M_REG_NOOP     = 0x00U,
	DAC80501M_REG_DEVID    = 0x01U,
	DAC80501M_REG_SYNC     = 0x02U,
	DAC80501M_REG_CONFIG   = 0x03U,
	DAC80501M_REG_GAIN     = 0x04U,
	DAC80501M_REG_TRIGGER  = 0x05U,
	DAC80501M_REG_STATUS   = 0x07U,
	DAC80501M_REG_DAC_DATA = 0x08U

} DAC80501M_Register_t;

typedef struct
{
	uint8_t dac_sync;
	uint8_t int_ref;
	uint16_t dev_i2c_addr;

} DAC80501M_InitTypeDef_t;

typedef struct
{
	I2C_HandleTypeDef *hi2c;

	DAC80501M_InitTypeDef_t Init;

} DAC80501M_Handle_t;

//@dac_sync
#define DAC80501M_SYNC_DI 0U
#define DAC80501M_SYNC_EN 1U

//@int_ref
#define DAC80501M_INT_REF_EN 0U
#define DAC80501M_INT_REF_DI 1U

//
#define DAC80501M_BUFF_GAIN_1 0U
#define DAC80501M_BUFF_GAIN_2 1U

#define DAC80501M_REF_DIV_1 0U
#define DAC80501M_REF_DIV_2 1U

#define __DAC80501M_SYNC_EN_POS            0x00U
#define __DAC80501M_REF_PWDWN_POS          0x08U
#define __DAC80501M_BUFF_GAIN_POS          0x00U
#define __DAC80501M_REF_DIV_POS            0x08U
#define __DAC80501M_LDAC_POS       0x04U
#define __DAC80501M_SOFTRESET_POS  0x00U

#define __DAC80501_TRIGGER_SOFTWARE_RESET    0x000AU
#define __DAC80501_TRIGGER_LDAC              0x0001U

HAL_StatusTypeDef DAC80501M_Init(DAC80501M_Handle_t *hdac);
uint8_t DAC80501M_Get_Ref_Alarm(DAC80501M_Handle_t *hdac);
HAL_StatusTypeDef DAC80501M_Soft_Reset(DAC80501M_Handle_t *hdac);
HAL_StatusTypeDef DAC80501M_Set_Buff_Gain(DAC80501M_Handle_t *hdac, uint8_t Dac80501_Buff_Gain);
HAL_StatusTypeDef DAC80501M_Set_Ref_Div(DAC80501M_Handle_t *hdac, uint8_t Dac80501_Ref_Div);
HAL_StatusTypeDef DAC80501M_ReadReg(DAC80501M_Handle_t *hdac,DAC80501M_Register_t Reg,uint16_t *pData);
HAL_StatusTypeDef DAC80501M_Trigger_LADC(DAC80501M_Handle_t *hdac);
HAL_StatusTypeDef DAC80501M_I2C_Write_DAC(DAC80501M_Handle_t *hdac,uint16_t Data);
#endif /* INC_DAC80501M_I2C_H_ */
