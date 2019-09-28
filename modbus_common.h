/**************************************************************************
**  名称: modbus_common.h头文件
*   日期: 2019年09月17日
*   作者: Steven Zhang
*   描述: 协议通用头文件
*   修改记录: 新建文件
***************************************************************************/
#ifndef MODBUS_COMMON_H
#define MODBUS_COMMON_H

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */
#include <stdarg.h>
#include <stdio.h>
/**************************************************************************
** 宏定义
**************************************************************************/
/* 简化类型定义 */
typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned int uint32_t;
typedef signed int int32_t;
typedef unsigned long long int uint64_t;
typedef signed long long int int64_t;


#define M_MODBUS_OK		(0)			/* 成功 */
#define M_MODBUS_ERR	(-1)		/* 失败 */

#define MODBUS_FUNC_CODE_BASE_NUM            1000    /*本协议规定基准数是1000*/
#if 0
#ifndef bool
typedef unsigned char bool;
#endif

#ifndef false
#define false			(uint8_t)(0)
#endif

#ifndef true
#define true			(uint8_t)(1)
#endif
#endif
#define DEBUG			(1)

/* 打印重定义 */
#ifdef DEBUG
#define M_MODBUS_LOG_DEBUG(format,...) printf("[%s][%d] debug: " #format "\n", __func__, __LINE__, ##__VA_ARGS__); \
    fflush(stdout);
#define M_MODBUS_LOG_WARN(format,...)  printf("[%s][%d] warn: " #format "\n", __func__, __LINE__, ##__VA_ARGS__); \
    fflush(stdout);
#define M_MODBUS_LOG_ERROR(format,...) printf("[%s][%d] error: " #format "\n", __func__, __LINE__, ##__VA_ARGS__); \
    fflush(stdout);
#define M_MODBUS_LOG_INFO(format,...)  	\
    printf("[%s][%d] info: " #format "\n", __func__, __LINE__, ##__VA_ARGS__); \
    fflush(stdout);
#define M_MODBUS_TRACE_IN()				\
    printf("[%s][%s][%d] trace in\n", __FILE__, __func__, __LINE__); \
    fflush(stdout);
#define M_MODBUS_TRACE_OUT()			\
    printf("[%s][%s][%d] trace out\n", __FILE__, __func__, __LINE__); \
    fflush(stdout);

#else
#define M_MODBUS_LOG_DEBUG(format,...)
#define M_MODBUS_LOG_WARN(format,...)
#define M_MODBUS_LOG_ERROR(format,...)
#define M_MODBUS_LOG_INFO(format,...)
#define M_MODBUS_TRACE_IN()
#define M_MODBUS_TRACE_OUT()
#endif

/**************************************************************************
** 结构体声明
**************************************************************************/
/* 寄存器错误码应答 */
typedef enum _MODBUS_ERROR_CODE_E
{
    E_CODE_NO_ERR = 0,
    E_CODE_ILLEGAL_FUNC_ERR,						/* 非法功能错误 */
    E_CODE_ILLEGAL_REG_ADDR_ERR,					/* 非法寄存器数据地址 */
    E_CODE_ILLEGAL_REG_VAL_ERR,						/* 非法寄存器数据值 */
    E_CODE_SLAVER_FAULT_ERR,						/* 从设备故障 */
    E_CODE_DEALING_CONFIRM_ERR,						/* 正在确认 */
    E_CODE_OTHER_ERR,								/* 其他错误 */
} MODBUS_ERROR_CODE_E;

/* 寄存器类型 */
typedef enum _MODBUS_FUNC_CODE_TYPE_E
{
    E_FUNC_CODE_READ_COILS = 0x01,					/* 读线圈状态 */
    E_FUNC_CODE_READ_DISCRETE_INPUTS = 0x02,		/* 读离散输入状态 */
    E_FUNC_CODE_READ_HOLDING_REGISTERS = 0x03,		/* 读保持寄存器 */
    E_FUNC_CODE_READ_INPUT_REGISTERS = 0x04,		/* 读输入寄存器 */
    E_FUNC_CODE_WRITE_SINGLE_COIL = 0x05,			/* 写单个线圈 */
    E_FUNC_CODE_WRITE_SINGLE_REGISTER = 0x06,		/* 写单个保持寄存器 */
    E_FUNC_CODE_READ_EXCEPTION_STATUS = 0x07,		/* 读异常状态 */
    E_FUNC_CODE_WRITE_MULTIPLE_COILS = 0x0F,		/* 写多个线圈 */
    E_FUNC_CODE_WRITE_MULTIPLE_REGISTERS = 0x10,	/* 写多个保持寄存器 */
    E_FUNC_CODE_REPORT_SLAVE_ID = 0x11,				/* 报告从机标识 */
} MODBUS_FUNC_CODE_TYPE_E;

/* 寄存器地址类型 */
typedef enum _modbus_func_address_st
{
    E_INPUT_POWER						=0,
    E_ACTIVE_POWER						=1,
    E_REACTIVE_POWER	 				=2,
    E_APPARENT_POWER					=3,
    E_POWER_FACTOR						=4,
    E_EFFICIENCY						=5,
    E_OUTPUT_FREQUENCY					=6,
    E_INPUT_VOLTAGE						=7,
    E_INPUT_CURRENT=8,
    E_EMISSION_REDUCTION_LO=9,
    E_EMISSION_REDUCTION_HI=10,
    E_A_PHASE_ACTIVE_POWER=11,
    E_B_PHASE_ACTIVE_POWER=12,
    E_C_PHASE_ACTIVE_POWER=13,
    E_A_PHASE_REACTIVE_POWER=14,
    E_B_PHASE_REACTIVE_POWER=15,
    E_C_PHASE_REACTIVE_POWER=16,
    E_A_PHASE_APPARENT_POWER=17,
    E_B_PHASE_APPARENT_POWER=18,
    E_C_PHASE_APPARENT_POWER=19,
    E_A_PHASE_POWER_FACTOR=20,
    E_B_PHASE_POWER_FACTOR=21,
    E_C_PHASE_POWER_FACTOR=22,
    E_A_PHASE_NET_SIDE_CURRENT=23,
    E_B_PHASE_NET_SIDE_CURRENT=24,
    E_C_PHASE_NET_SIDE_CURRENT=25,
    E_A_PHASE_NET_MODULE_CURRENT=26,
    E_B_PHASE_NET_MODULE_CURRENT=27,
    E_C_PHASE_NET_MODULE_CURRENT=28,
    E_DC_INSULATION_VALUE=29,
    E_POWER_GRID_AB_LINE_VOLTAGE=30,
    E_POWER_GRID_BC_LINE_VOLTAGE=31,
    E_POWER_GRID_CA_LINE_VOLTAGE=32,
    E_MODULAR_AB_LINE_VOLTAGE=33,
    E_MODULAR_BC_LINE_VOLTAGE=34,
    E_MODULAR_CA_LINE_VOLTAGE=35,
    E_MODULAR_1A_PHASE_OUTPUT_CURRENT=36,
    E_MODULAR_1B_PHASE_OUTPUT_CURRENT=37,
    E_MODULAR_1C_PHASE_OUTPUT_CURRENT=38,
    E_MODULAR1_INPUT_VOLTAGE=39,
    E_MODULAR1_INPUT_CURRENT=40,
    E_MODULAR_2A_PHASE_OUTPUT_CURRENT=41,
    E_MODULAR_2B_PHASE_OUTPUT_CURRENT=42,
    E_MODULAR_2C_PHASE_OUTPUT_CURRENT=43,
    E_MODULAR2_INPUT_VOLTAGE=44,
    E_MODULAR2_INPUT_CURRENT=45,
    E_RESERVE1=46,
    E_OMAP_VERSION_NUMBER=47,
    E_RESERVE2=48,
//    E_RESERVE3=49,
    E_RESERVE3=50,
    E_RESERVE4=51,
    E_RESERVE5=52,
    E_RESERVE6=53,
    E_RESERVE7=54,
    E_RESERVE8=55,
    E_RESERVE9=56,
    E_RESERVE10=57,
    E_RESERVE11=58,
    E_RESERVE12=59,
    E_RESERVE13=60,
    E_DSP_VERSION_NUMBER_A=61,
    E_DSP_VERSION_NUMBER_B=62,
    E_DSP_VERSION_NUMBER_C=63,
    E_TOTAL_POWER_LO=64,
    E_TOTAL_POWER_HI=65,
    E_TOTAL_DAILY_POWER_LO=66,
    E_TOTAL_DAILY_POWER_HI=67,
    E_POWER_OUTAGE_DAY=68,
    E_POWER_OUTAGE_MON=69,
    E_POWER_OUTAGE_YEAR=70,
    E_TOTAL_POWER_GENERATION_TIME_LO=71,
    E_TOTAL_POWER_GENERATION_TIME_HI=72,
    E_DAY_POWER_GENERATION_TIME_LO=73,
    E_DAY_POWER_GENERATION_TIME_HI=74,
//   E_RESERVE(13~47)=(75~109),
//    ..........
    E_INSIDE_CABINET_TEMPERATURE=110,
    E_MODULAR1_TEMPERATURE=111,
    E_RESERVE48=112,
    E_MODULAR2_TEMPERATURE=113,
//    E_RESERVE(48~83)=(114~149),
} MODBUS_FUNC_ADDRESS;


/* 支持三种协议方式, modbus ASCII, RTU, RTU_TCP */
typedef enum _MODBUS_PROTOCOL_TYPE_E
{
    E_START_PROTOCOL_TYPE = 0,
    E_ASCII_PROTOCOL_TYPE,
    E_RTU_PROTOCOL_TYPE,
    E_RTU_TCP_PROTOCOL_TYPE,
} MODBUS_PROTOCOL_TYPE_E;

/* 波特率 */
typedef enum _MODBUS_BAUD_E
{
    E_BAUD_2400BPS = 2400,
    E_BAUD_4800BPS = 4800,
    E_BAUD_9600BPS = 9600,
    E_BAUD_14400BPS = 14400,
    E_BAUD_19200BPS = 19200,
    E_BAUD_28800BPS = 28800,
    E_BAUD_38400BPS = 38400,
    E_BAUD_57600BPS = 56700,
    E_BAUD_115200BPS = 115200,
    E_BAUD_128000BPS = 128000,
    E_BAUD_256000BPS = 256000,
} MODBUS_BAUD_E;

/* 数据位 */
typedef enum _MODBUS_DATA_BIT_E
{
    E_DATA_4BITS = 4,
    E_DATA_5BITS = 5,
    E_DATA_6BITS = 6,
    E_DATA_7BITS = 7,
    E_DATA_8BITS = 8,
} MODBUS_DATA_BIT_E;

/* 停止位 */
typedef enum _MODBUS_STOP_BIT_E
{
    E_STOP_1V0BIT = 0,
    E_STOP_1V5BITS,
    E_STOP_2V0BITS,
} MODBUS_STOP_BIT_E;

/* 校验符 */
typedef enum _MODBUS_CHECK_E
{
    E_CHECK_NONE = 0,
    E_CHECK_EVEN,
    E_CHECK_ODD,
    E_CHECK_MARK,
    E_CHECK_SPACK,
} MODBUS_CHECK_E;

/* modbus的驱动口的相关参数 */
typedef struct _modbus_com_params_st
{
    char *device;				/* 硬件设备 */
    int32_t baud;				/* 波特率 */
    uint8_t data_bit;			/* 数据位 */
    uint8_t stop_bit;			/* 停止位 */
    char parity;				/* 校验符 */
} modbus_com_params_st;

/**************************************************************************
** 函数声明
**************************************************************************/
/**************************************************************************
* 函  数: void modbus_log_hex_print(uint8_t *data, uint32_t datalen)
* 描  述: 打印hex数据
* 入  参: uint8_t *data : 需要打印的数据流
          uint32_t datalen : 数据长度
* 出  参: void
* 返回值: void
**************************************************************************/
void modbus_log_hex_print(uint8_t *data, uint32_t datalen);

/**************************************************************************
* 函  数: int32_t modbus_calc_crc(uint8_t *data, uint32_t datalen, uint16_t *crc)
* 描  述: 计算crc校验函数
* 入  参: uint8_t *data : 计算crc校验的数据
          uint32_t datalen : 计算crc校验的数据长度
* 出  参: uint16_t *crc : 计算crc校验值
* 返回值: int32_t : M_MODBUS_OK  - 成功
                    M_MODBUS_ERR - 失败
**************************************************************************/
int32_t modbus_calc_crc(uint8_t *data, uint32_t datalen, uint16_t *crc);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif //MODBUS_COMMON_H
