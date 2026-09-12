/*
 * DAC80501M_SPI.h
 *
 *  Created on: Sep 9, 2026
 *      Author: SRIVATHSJAYAN
 */

#ifndef INC_DAC80501M_SPI_H_
#define INC_DAC80501M_SPI_H_

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
	uint8_t buff_gain;
	uint8_t ref_div;

} DAC80501M_InitTypeDef_t;

typedef struct
{
	SPI_HandleTypeDef *hspi;

	GPIO_TypeDef *CS_Port;
	uint16_t CS_Pin;

	DAC80501M_InitTypeDef_t Init;

} DAC80501M_Handle_t;

//SYNC Reg
#define DAC80501M_SYNC_EN 1
#define DAC80501M_SYNC_DI 0
//CONFIG Reg
#define DAC80501M_INT_REF_EN 0
#define DAC80501M_INT_REF_DI 1

#define DAC80501M_PWUP 0
#define DAC80501M_PWDWN 1
//GAIN Reg
#define DAC80501M_BUFF_GAIN_1 0
#define DAC80501M_BUFF_GAIN_2 1

#define DAC80501M_REF_DIV_1 0
#define DAC80501M_REF_DIV_2 1

#define __DAC80501M_SYNC_EN_POS            0x00U
#define __DAC80501M_REF_PWDWN_POS          0x08U

#define __DAC80501M_BUFF_GAIN_POS          0x00U
#define __DAC80501M_REF_DIV_POS            0x08U
#define __DAC80501M_TRIGGER_LDAC_POS       0x04U
#define __DAC80501M_TRIGGER_SOFTRESET_POS  0x00U

#define __DAC80501_TRIGGER_SOFTWARE_RESET    0x000AU
#define __DAC80501_TRIGGER_LDAC              0x0001U

HAL_StatusTypeDef DAC80501M_Init(DAC80501M_Handle_t *hdac);
HAL_StatusTypeDef DAC80501M_Soft_Reset(DAC80501M_Handle_t *hdac);
HAL_StatusTypeDef DAC80501M_SPI_Write_DAC(DAC80501M_Handle_t *hdac,uint16_t Data);
HAL_StatusTypeDef DAC80501M_Trigger_LADC(DAC80501M_Handle_t *hdac);
#endif /* INC_DAC80501M_SPI_H_ */
