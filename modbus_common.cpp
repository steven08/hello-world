/**************************************************************************
**  名称: modbus_common.c文件
*   日期: 2019年09月17日
*   作者: Steven Zhang
*   描述: 协议通用实现文件
*   修改记录: 新建文件
***************************************************************************/
#include "modbus_common.h"
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
//#include <netinet/in.h>

/**************************************************************************
** 宏定义
**************************************************************************/
#define M_MODBUS_POLY_VALUE			(0xa001)	/*CRC16-MODBUS 定义的多项式值 */
#define M_MODBUS_CRC_SEED_VALUE		(0xFFFF)	/* 预置值 */

/**************************************************************************
** 结构体声明
**************************************************************************/

/**************************************************************************
** 函数声明
**************************************************************************/

/**************************************************************************
** 函数定义
**************************************************************************/
/**************************************************************************
* 函  数: void modbus_log_hex_print(uint8_t *data, uint32_t datalen)
* 描  述: 打印hex数据
* 举  例: 输入data: 0x12 0x34 0x56 0x00 0x78 0x90; datalen: 6
          打印结果: "hex_data[6]: 12 34 56 00 78 90"
* 入  参: uint8_t *data : 需要打印的数据流
          uint32_t datalen : 数据长度
* 出  参: void
* 返回值: void
**************************************************************************/
void modbus_log_hex_print(uint8_t *data, uint32_t datalen)
{
    uint32_t index = 0;
    uint32_t temp_data_len = datalen * 3 + 1;
    uint32_t templen = 0;
    char *temp_data = nullptr;

    if ((nullptr == data) || (0 == datalen))
    {
        M_MODBUS_LOG_ERROR("print hex data log failed, input param is error.");
        return;
    }

    temp_data = static_cast<char*>(malloc(temp_data_len));
    if (nullptr == temp_data)
    {
        M_MODBUS_LOG_ERROR("print hex data log, malloc data[%d] space failed.", temp_data_len);
        return;
    }

    (void)memset(temp_data, 0, temp_data_len);
    for (index = 0; index < datalen; index++)
    {
        templen += sprintf(&temp_data[templen], "%02x ", data[index]);
    }

    printf("hex_data[%d]: %s\n", datalen, temp_data);

    free(temp_data);
    temp_data = nullptr;
}

/**************************************************************************
* 函  数: int32_t modbus_calc_crc(uint8_t *data, uint32_t datalen, uint16_t *crc)
* 描  述: 计算crc校验函数
* 举  例: 输入data: 0x12 0x23 0x46 0x99; datalen = 4;
          计算得到两个字节的crc校验码: XXXX
* 入  参: uint8_t *data : 计算crc校验的数据
          uint32_t datalen : 计算crc校验的数据长度
* 出  参: uint16_t *crc : 计算crc校验值
* 返回值: int32_t : M_MODBUS_OK  - 成功
                    M_MODBUS_ERR - 失败
**************************************************************************/
int32_t modbus_calc_crc(uint8_t *data, uint32_t datalen, uint16_t *crc)
{
    uint16_t crc_val = M_MODBUS_CRC_SEED_VALUE;
    uint32_t i = 0;
    uint32_t j = 0;

    if ((nullptr == data) || (0 == datalen))
    {
        M_MODBUS_LOG_ERROR("input param is error");
        return M_MODBUS_ERR;
    }
    for (i = 0; i < datalen; i++) {
        crc_val = static_cast<uint16_t>(crc_val ^ data[i]);
        for (j = 0; j < 8; j++) {
            if (crc_val & 0x01) {
                crc_val = crc_val  >> 1;
                crc_val = crc_val ^ M_MODBUS_POLY_VALUE;
            } else {
                crc_val = crc_val >> 1;
            }
        }
    }
    *crc = crc_val;

    return M_MODBUS_OK;
}
