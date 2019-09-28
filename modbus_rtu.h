#ifndef MODBUS_RTU_H
#define MODBUS_RTU_H

/**************************************************************************
**  名称: modbus_rtu.h头文件
*   日期: 2019年09月17日
*   作者: Steven Zhang
*   描述: 协议rtu类头文件
*   修改记录: 新建文件
***************************************************************************/

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include "modbus_common.h"
/**************************************************************************
** 宏定义
**************************************************************************/

/**************************************************************************
** 结构体声明
**************************************************************************/
/* 返回失败功能码应答结构体 */
typedef struct _modbus_rtu_fail_st
{
    uint8_t slave_addr;
    uint8_t err_func_code;
    uint8_t except_code;
    uint16_t crc16;
} __attribute__((packed)) modbus_rtu_fail_st;

/* modbus rtu 读写寄存器响应协议结构 */
typedef struct _modbus_rtu_ack_data_rw_st
{
    uint8_t slave_addr;			/* 从设备地址 */
    uint8_t func_code; 			/* 功能码 */
    uint8_t datalen;			/* 响应数长的 */
    uint8_t *data;				/* 响应数据 */
} __attribute__((packed)) modbus_rtu_ack_data_rw_st;

/****************************************************************************
** 读寄存器结构体
*****************************************************************************/
/* 读从机寄存器的结构体(支持1-126) */
typedef struct _modbus_read_reg_info_st
{
     uint8_t slave_addr;							/* 从设备地址 */
     MODBUS_FUNC_CODE_TYPE_E func_code; 			/* 功能码 */
     uint16_t register_addr;						/* 寄存器首地址 */
     uint16_t register_cnt; 						/* 寄存器数目 */
} modbus_read_reg_info_st;

/* modbus rtu 读寄存器协议结构 */
typedef struct _modbus_rtu_read_reg_st
{
    uint8_t slave_addr;			/* 从设备地址 */
    uint8_t func_code; 			/* 功能码 */
    uint16_t register_addr; 		/* 读寄存器地址 */
    uint16_t register_cnt;		/* 寄存器数目 */
    uint16_t crc;			/* crc校验 */
} __attribute__((packed)) modbus_rtu_read_reg_st;

/* 用于区分读状态寄存器，还是读保存寄存器 */
typedef enum _MODBUS_RTU_READ_REG_TYPE_E
{
    E_MODBUS_RTU_READ_STATUS_REG = 0,
    E_MODBUS_RTU_READ_STORAGE_REG,
} MODBUS_RTU_READ_REG_TYPE_E;

/* 用于保存读状态寄存器和保存寄存器的数据值信息 */
typedef union _modbus_rtu_reg_data_u
{
    uint16_t *storage_reg_value;
    uint8_t *status_reg_value;
} modbus_rtu_reg_data_u;

typedef struct _modbus_rtu_reg_data_val_st
{
    MODBUS_RTU_READ_REG_TYPE_E read_reg_type;	/* 状态寄存器，还是存储寄存器 */
    uint16_t start_register_addr;				/* 寄存器起始地址 */
    uint8_t read_register_cnt;					/* 寄存器个数 */
    modbus_rtu_reg_data_u reg_data;				/* 保存的寄存器数据值 */
} modbus_rtu_reg_data_val_st;

/****************************************************************************
** 写寄存器结构体
****************************************************************************/
/* 用于写寄存器的值 */
typedef union _modbus_rtu_write_reg_val_st
{
    uint16_t *register_val; 					/* 写寄存器的值 */
    uint8_t *multiple_status_reg_val;			/* 多个状态寄存器的值 */
} modbus_rtu_write_reg_val_st;

/* 用于写状态/存储寄存器 */
typedef struct _modbus_rtu_write_reg_info_st
{
    uint8_t slave_addr; 						/* 从设备地址 */
    MODBUS_FUNC_CODE_TYPE_E func_code; 			/* 功能码 */
    uint16_t register_addr;						/* 寄存器首地址 */
    uint8_t register_cnt; 						/* 写寄存器个数 */
    modbus_rtu_write_reg_val_st reg_val;		/* 寄存器的值 */
} modbus_rtu_write_reg_info_st;

/* 解析写单个寄存器 */
typedef struct _modbus_rtu_ack_write_signal_reg_st
{
    uint8_t slave_addr;							/* 从设备地址 */
    uint8_t func_code;							/* 功能码 */
    uint16_t register_addr;						/* 寄存器首地址 */
    uint16_t register_data;						/* 写寄存器的值 */
    uint16_t crc;								/* crc校验 */
} __attribute__((packed)) modbus_rtu_ack_write_signal_reg_st;

/* 解析写多个状态寄存器应答结构 */
typedef struct _modbus_rtu_ack_write_multiple_status_reg_st
{
    uint8_t slave_addr;							/* 从设备地址 */
    uint8_t func_code;							/* 功能码 */
    uint16_t register_addr;						/* 寄存器首地址 */
    uint16_t register_cnt;						/* 寄存器个数 */
    uint8_t register_data_len;					/* 寄存器数据值长度 */
    uint16_t crc;								/* crc校验 */
} __attribute__((packed)) modbus_rtu_ack_write_multiple_state_reg_st;

/* 解析写多个状态寄存器应答结构 */
typedef struct _modbus_rtu_ack_write_multiple_storage_reg_st
{
    uint8_t slave_addr;							/* 从设备地址 */
    uint8_t func_code;							/* 功能码 */
    uint16_t register_addr;						/* 寄存器首地址 */
    uint16_t register_cnt;						/* 寄存器个数 */
    uint16_t crc;								/* crc校验 */
} __attribute__((packed)) modbus_rtu_ack_write_multiple_storage_reg_st;


/**************************************************************************
** 函数声明
**************************************************************************/

//#include <QByteArray>
class modbus_rtu
{
public:
    modbus_rtu();
    ~modbus_rtu();

    int32_t modbus_rtu_pack_read_reg (
                modbus_read_reg_info_st *reg_info,
                uint8_t *data,
                uint32_t *datalen
                );
    int32_t modbus_rtu_unpack_read_status_reg (
                modbus_read_reg_info_st *reg_info,
                modbus_rtu_ack_data_rw_st *data_ack,
                modbus_rtu_reg_data_val_st *data_val
                );
    int32_t modbus_rtu_unpack_read_storage_reg (
                modbus_read_reg_info_st *reg_info,
                modbus_rtu_ack_data_rw_st *data_ack,
                modbus_rtu_reg_data_val_st *data_val
                );
    int32_t modbus_rtu_unpack_read_register (
                modbus_read_reg_info_st *reg_info,
                uint8_t* data,
                uint32_t datalen,
                modbus_rtu_reg_data_val_st *data_val
                );
    int32_t modbus_rtu_pack_write_reg (
                modbus_rtu_write_reg_info_st *reg_info,
                uint8_t *data,
                uint32_t *datalen
                );
    int32_t modbus_rtu_unpack_write_signal_reg (
                modbus_rtu_write_reg_info_st *reg_info,
                uint8_t *data,
                uint32_t datalen
                );
    int32_t modbus_rtu_unpack_write_multiple_state_reg (
                modbus_rtu_write_reg_info_st *reg_info,
                uint8_t *data,
                uint32_t datalen
                );
   int32_t modbus_rtu_unpack_write_multiple_storage_reg(
                modbus_rtu_write_reg_info_st *reg_info,
                uint8_t *data,
                uint32_t datalen
                );
   int32_t modbus_rtu_unpack_write_reg(
                   modbus_rtu_write_reg_info_st *reg_info,
                   uint8_t *data,
                   uint32_t datalen
                   );
   int32_t modbus_rtu_init(modbus_com_params_st *com_param);
};
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif // MODBUS_RTU_H
