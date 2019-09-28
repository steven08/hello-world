#include "modbus_rtu.h"
/**************************************************************************
**  名称: modbus_rtu.c文件
*   日期: 2019年09月17日
*   作者: Steven Zhang
*   描述: 协议rtu类实现文件
*   修改记录: 新建文件
*
*
*   修改记录:
***************************************************************************/
#include "modbus_rtu.h"
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "winsock2.h"

modbus_rtu::modbus_rtu()
{

}

modbus_rtu::~modbus_rtu()
{

}
/**************************************************************************
    ** 宏定义
    **************************************************************************/
#define M_MODBUS_MAX_REGISTER_CNT	(127)		/* 读写寄存器最大个数 */

/**************************************************************************
    ** 结构体声明
    **************************************************************************/

/**************************************************************************
    ** 全局变量声明
    **************************************************************************/

/**************************************************************************
    ** 函数声明
    **************************************************************************/
/*************************************************************************
    * 函  数: int32_t 	modbus_rtu_pack_read_reg(modbus_read_reg_info_st *reg_info, uint8_t *data, uint32_t *datalen)
    * 描  述: 按Modbus协议，组包读寄存器
    * 举  例: 按照以下协议组包公共部分
      typedef struct _modbus_rtu_read_reg_st
      {
     uint8_t slave_addr;
     uint8_t func_code;
     uint16_t register_addr;
     uint16_t register_cnt;
     uint16_t crc;
      } __attribute__((packed)) modbus_rtu_read_reg_st;
      根据输入的寄存器信息，组包数据部分
    * 入  参: modbus_read_reg_info_st *reg_info : 输入读写寄存器信息
    * 出  参: uint8_t *data : 按modbus协议组包的数据流
      uint32_t *datalen : 数据流长度
    * 返回值: int32_t : M_MODBUS_OK  - 成功
    M_MODBUS_ERR - 失败
    **************************************************************************/
int32_t modbus_rtu::modbus_rtu_pack_read_reg
(
                modbus_read_reg_info_st *reg_info,
                uint8_t *data,
                uint32_t *datalen
                )
{
    int32_t ret = M_MODBUS_ERR;
    modbus_rtu_read_reg_st read_reg;

    M_MODBUS_TRACE_IN();

    do
    {
        /* 入参校验 */
        if ((nullptr == reg_info) || (nullptr == data) || (nullptr == datalen))
        {
            M_MODBUS_LOG_ERROR("input param is error, null point");
            break;
        }

        /* 按modbus协议组包 */
        (void)memset(&read_reg, 0, sizeof(read_reg));
        read_reg.slave_addr = reg_info->slave_addr;
        read_reg.func_code = static_cast<uint8_t>(reg_info->func_code);

        /* 组包寄存器地址 */
        read_reg.register_addr = reg_info->register_addr;

        /* 寄存器数目 */
        if ((0 == reg_info->register_cnt) || (M_MODBUS_MAX_REGISTER_CNT < reg_info->register_cnt))
        {
            M_MODBUS_LOG_ERROR("register count[%d] is over out of range", reg_info->register_cnt);
            break;
        }
        read_reg.register_cnt = reg_info->register_cnt;

        /* 大小端转换 */
        read_reg.register_addr = htons(read_reg.register_addr);
        read_reg.register_cnt = htons(read_reg.register_cnt);

        /* crc校验 */
        ret = modbus_calc_crc((uint8_t *)&read_reg, sizeof(read_reg) - sizeof(uint16_t), &read_reg.crc);
        if (M_MODBUS_OK != ret)
        {
            M_MODBUS_LOG_ERROR("calc crc is failed, ret = %d", ret);
            break;
        }

        (void)memcpy(data, (uint8_t *)&read_reg, sizeof(modbus_rtu_read_reg_st));
        *datalen = sizeof(modbus_rtu_read_reg_st);

        /* 打印组包数据 */
        modbus_log_hex_print(data, *datalen);
        M_MODBUS_LOG_DEBUG("package read status register data ok");

        ret = M_MODBUS_OK;
    }
    while (0);

    M_MODBUS_TRACE_OUT();

    return ret;
}

/**************************************************************************
* 函  数: int32_t modbus_rtu_unpack_read_status_reg(modbus_read_reg_info_st *reg_info,
    modbus_rtu_ack_data_rw_st *data_ack, modbus_rtu_reg_data_val_st *data_val)
* 描  述: 按Modbus协议，解析读取状态寄存器的响应数据
* 举  例: 按照以下协议组包进行解析，输出正常响应的数据值
      typedef struct _modbus_rtu_ack_data_rw_st
      {
     uint8_t slave_addr;
     uint8_t func_code;
     uint8_t datalen;
     uint8_t *data;
      } __attribute__((packed)) modbus_rtu_ack_data_rw_st;
* 入  参: modbus_read_reg_info_st *reg_info : 输入读写寄存器信息
      modbus_rtu_ack_data_rw_st *data_ack : 接收的状态寄存器的信息
* 出  参: modbus_rtu_reg_data_val_st *data_val : 输出读保存/状态寄存器的数据值信息
* 返回值: int32_t : M_MODBUS_OK  - 成功
        M_MODBUS_ERR - 失败
**************************************************************************/
int32_t modbus_rtu::modbus_rtu_unpack_read_status_reg (
                modbus_read_reg_info_st *reg_info,
                modbus_rtu_ack_data_rw_st *data_ack,
                modbus_rtu_reg_data_val_st *data_val
                )
{
    int32_t ret = M_MODBUS_ERR;

    M_MODBUS_TRACE_IN();

    do
    {
        if ((nullptr == reg_info) || (nullptr == data_ack) || (nullptr == data_val))
        {
            M_MODBUS_LOG_ERROR("input param is error, null point");
            break;
        }

        /* 读状态寄存器 */
        if (reg_info->register_cnt != static_cast<uint16_t>(data_ack->datalen))
        {
            M_MODBUS_LOG_ERROR("read status register num[%x] not equal recv datalen[%x]",
                               reg_info->register_cnt, data_ack->datalen);
            break;
        }

        /* 保存到结构体中 */
        data_val->read_reg_type = E_MODBUS_RTU_READ_STATUS_REG;
        data_val->start_register_addr = reg_info->register_addr;
        data_val->read_register_cnt = reg_info->register_cnt / 8 + (reg_info->register_cnt % 8) ? 1 : 0;
        data_val->reg_data.status_reg_value = static_cast<uint8_t *>(malloc(data_val->read_register_cnt));
        //data_val->reg_data.status_reg_value = new(sizeof(uint8_t)*data_val->read_register_cnt);
        if (nullptr == data_val->reg_data.status_reg_value)
        {
            M_MODBUS_LOG_ERROR("malloc read status register[%d] space is failed", data_val->read_register_cnt);
            break;
        }

        (void)memset(data_val->reg_data.status_reg_value, 0, data_val->read_register_cnt);
        (void)memcpy(data_val->reg_data.status_reg_value, data_ack->data, data_val->read_register_cnt);

        M_MODBUS_LOG_DEBUG("recv read status register data ok");

        ret = M_MODBUS_OK;
    }
    while (0);

    M_MODBUS_TRACE_OUT();

    return ret;

}
/**************************************************************************
* 函  数: int32_t modbus_rtu_unpack_read_storage_reg(modbus_read_reg_info_st *reg_info,
    modbus_rtu_ack_data_rw_st *data_ack, modbus_rtu_reg_data_val_st *data_val)
* 描  述: 按Modbus协议，解析读取存储寄存器的响应数据
* 举  例: 按照以下协议组包进行解析，输出正常响应的数据值
      typedef struct _modbus_rtu_ack_data_rw_st
      {
     uint8_t slave_addr;
     uint8_t func_code;
     uint8_t datalen;
     uint8_t *data;
      } __attribute__((packed)) modbus_rtu_ack_data_rw_st;
* 入  参: modbus_read_reg_info_st *reg_info : 输入读写寄存器信息
      modbus_rtu_ack_data_rw_st *data_ack : 接收的状态寄存器的信息
* 出  参: modbus_rtu_reg_data_val_st *data_val : 输出读保存/状态寄存器的数据值信息
* 返回值: int32_t : M_MODBUS_OK  - 成功
        M_MODBUS_ERR - 失败
**************************************************************************/
int32_t modbus_rtu::modbus_rtu_unpack_read_storage_reg
(
                modbus_read_reg_info_st *reg_info,
                modbus_rtu_ack_data_rw_st *data_ack,
                modbus_rtu_reg_data_val_st *data_val
                )
{
    uint16_t index = 0;
    int32_t ret = M_MODBUS_ERR;

    M_MODBUS_TRACE_IN();

    do
    {
        if ((nullptr == reg_info) || (nullptr == data_ack) || (nullptr == data_val))
        {
            M_MODBUS_LOG_ERROR("input param is error, null point");
            break;
        }

        /* 读保存寄存器 */
        if ((reg_info->register_cnt * 2) != static_cast<uint16_t>(data_ack->datalen))
        {
            M_MODBUS_LOG_ERROR("read register num[%x *２] not equal recv datalen[%x]", reg_info->register_cnt, data_ack->datalen);
            break;
        }
        for(int i = 0; i < data_ack->datalen; i++) {
           M_MODBUS_LOG_DEBUG("0x%x ",data_ack->data[i]);
        }
        /* 解析获取到结构体中 */
        data_val->read_reg_type = E_MODBUS_RTU_READ_STORAGE_REG;
        data_val->start_register_addr = reg_info->register_addr;
        data_val->read_register_cnt = data_ack->datalen / 2;
        data_val->reg_data.storage_reg_value = static_cast<uint16_t *>(malloc(data_val->read_register_cnt));
        if (nullptr == data_val->reg_data.storage_reg_value)
        {
            M_MODBUS_LOG_ERROR("malloc read storage register[%d] space is failed", data_val->read_register_cnt);
            break;
        }

        (void)memset(data_val->reg_data.storage_reg_value, 0, data_val->read_register_cnt);
        for (index = 0; index < reg_info->register_cnt; index++)

        {
            data_val->reg_data.storage_reg_value[index] = data_ack->data[index * 2];
            data_val->reg_data.storage_reg_value[index] <<= 8;
            data_val->reg_data.storage_reg_value[index] |= data_ack->data[index * 2 + 1];
        }

        M_MODBUS_LOG_DEBUG("recv read storage register data ok");
        ret = M_MODBUS_OK;
    }
    while (0);

    M_MODBUS_TRACE_OUT();

    return ret;

}
/**************************************************************************
* 函  数: int32_t modbus_rtu_unpack_read_register(modbus_read_reg_info_st *reg_info,
          uint8_t *data, uint32_t datalen, uint16_t *pOutBuf)
* 描  述: 按Modbus协议，解析响应的数据
* 举  例: 按照以下协议组包进行解析，输出正常响应的数据值
      typedef struct _modbus_rtu_ack_data_rw_st
      {
     uint8_t slave_addr;
     uint8_t func_code;
     uint8_t datalen;
     uint8_t *data;
      } __attribute__((packed)) modbus_rtu_ack_data_rw_st;
* 入  参: modbus_read_reg_info_st *reg_info : 输入读写寄存器信息
      uint8_t *data : 按modbus协议组包的数据流
      uint32_t datalen : 数据流长度
* 出  参: modbus_rtu_reg_data_val_st *data_val : 输出读保存/状态寄存器的数据值信息，
      在调用后，成功获取完寄存器的数据后，需要释放申请的寄存器的值
* 返回值: int32_t : M_MODBUS_OK  - 成功
        M_MODBUS_ERR - 失败
**************************************************************************/
int32_t modbus_rtu::modbus_rtu_unpack_read_register
(
                modbus_read_reg_info_st *reg_info,
                uint8_t* data,
                uint32_t datalen,
                modbus_rtu_reg_data_val_st *data_val
                )
{
    int32_t ret = M_MODBUS_ERR;
    uint16_t crc_val = 0;
    uint16_t recv_crc_val = 0;
    modbus_rtu_ack_data_rw_st *data_ack = nullptr;
    modbus_rtu_fail_st *fail_info = nullptr;
    MODBUS_ERROR_CODE_E err_code = E_CODE_NO_ERR;

    M_MODBUS_TRACE_IN();

    do
    {
        /* 入参校验 */
        if ((nullptr == reg_info) || (nullptr == data) || (nullptr == data_val) || (3 > datalen))
        {
            M_MODBUS_LOG_ERROR("input param is error, null point");
            break;
        }
        //data_ack = (modbus_rtu_ack_data_rw_st*)malloc(sizeof(modbus_rtu_ack_data_rw_st));
        data_ack = static_cast<modbus_rtu_ack_data_rw_st*>(malloc(sizeof(modbus_rtu_ack_data_rw_st)));
        if (data_ack == nullptr) {
            M_MODBUS_LOG_DEBUG("malloc for data_ack failed\n");
        }
        memset(data_ack, 0, sizeof(modbus_rtu_ack_data_rw_st));
        memcpy(data_ack, data, sizeof(modbus_rtu_ack_data_rw_st));
        data_ack->data = static_cast<uint8_t*>(malloc(data_ack->datalen));
        memset(data_ack->data, 0, data_ack->datalen);
        memcpy(data_ack->data, &data[3], data_ack->datalen);

        /* 从地址校验 */
        if (reg_info->slave_addr != data_ack->slave_addr)
        {
            M_MODBUS_LOG_ERROR("pack read_reg[%x] not equal recv read_reg[%x]",
                               reg_info->slave_addr, data_ack->slave_addr);
            break;
        }

        /* CRC校验 */
        ret = modbus_calc_crc(data, datalen - 2, &crc_val);
        if (M_MODBUS_OK != ret)
        {
            M_MODBUS_LOG_ERROR("recv read_reg ack calc crc failed");
            break;
        }

        //recv_crc_val = (uint16_t)(((uint16_t)data[datalen - 2] << 8) | (uint16_t)data[datalen - 1]);
        recv_crc_val = static_cast<uint16_t>(static_cast<uint16_t>(data[datalen - 1] << 8) \
                | static_cast<uint16_t>(data[datalen - 2]));
        if (crc_val != recv_crc_val)
        {
            M_MODBUS_LOG_ERROR("recv read_reg(%x) not equal calc crc(%x)", recv_crc_val, crc_val);
            ret = M_MODBUS_ERR;
            break;
        }

        /* 错误功能码解析 */
        if (static_cast<uint8_t>(reg_info->func_code != data_ack->func_code))
        {
            if ((static_cast<uint8_t>(reg_info->func_code | 0x80) == data_ack->func_code))
            {
                fail_info = (modbus_rtu_fail_st *)data;

                /* 返回的是错误码 */
                switch (fail_info->except_code)
                {
                case 0x01:
                case 0x02:
                case 0x03:
                case 0x04:
                case 0x05:
                {
                    err_code = static_cast<MODBUS_ERROR_CODE_E>(fail_info->except_code);
                    break;
                }
                default:
                {
                    err_code = E_CODE_OTHER_ERR;
                    break;
                }
                }
            }
            else
            {
                err_code = E_CODE_OTHER_ERR;
            }

            M_MODBUS_LOG_WARN("read register data, func code[%x] is fail ack value[%x]",
                              data_ack->func_code, fail_info->except_code);

            ret = M_MODBUS_OK;
            break;
        }

        switch (reg_info->func_code)
        {
        case E_FUNC_CODE_READ_COILS:
        case E_FUNC_CODE_READ_DISCRETE_INPUTS:
        {
            /* 读状态寄存器 */
            ret = modbus_rtu_unpack_read_status_reg(reg_info, data_ack, data_val);
            break;
        }
        case E_FUNC_CODE_READ_HOLDING_REGISTERS:
        case E_FUNC_CODE_READ_INPUT_REGISTERS:
        {
            /* 读存储寄存器 */
            ret = modbus_rtu_unpack_read_storage_reg(reg_info, data_ack, data_val);
            break;
        }
        default:
        {
            /* 其他 */
            M_MODBUS_LOG_WARN("not recv read register data, func code[%x]", reg_info->func_code);
            ret = M_MODBUS_ERR;
            break;
        }
        }

        if (nullptr != data_ack) {
            if(nullptr !=data_ack->data) {
                free(data_ack->data);
            }
            free(data_ack);
        }
    }
    while (0);

    M_MODBUS_TRACE_OUT();

    return ret;
}

/**************************************************************************
* 函  数: int32_t modbus_rtu_pack_write_reg(modbus_rtu_write_reg_info_st *reg_info,
        uint8_t *data, uint32_t *datalen)
* 描  述: 按Modbus协议，组包写寄存器的值
* 举  例:
    对于单个状态/存储寄存器来说,组包格式,按字节的方式:
    发送->slave_addr(1) + func(1) + reg_addr(2) + reg_val(2) + crc(2)
    应答->slave_addr(1) + func(1) + reg_addr(2) + reg_val(2) + crc(2)
    对于多个状态寄存器来说,组包格式,按bit的方式:
    发送->slave_addr(1) + func(1) + reg_addr(2) + reg_cnt(2) + reg_val_len(1) + reg_val(n) + crc(2)
    应答->slave_addr(1) + func(1) + reg_addr(2) + reg_cnt(2) + reg_val_len(1) + crcc(2)
    对于多个存储寄存器来说，组包格式，按字节的方式:
    发送->slave_addr(1) + func(1) + reg_addr(2) + reg_cnt(2) + reg_val_len(1) + reg_val(n) + crc(2)
    应答->slave_addr(1) + func(1) + reg_addr(2) + reg_cnt(2) + crcc(2)
* 入  参: modbus_register_info_st *reg_info : 输入写寄存器信息
* 出  参: uint8_t *data : 按modbus协议组包的数据流
      uint32_t *datalen : 数据流长度
* 返回值: int32_t : M_MODBUS_OK  - 成功
        M_MODBUS_ERR - 失败
**************************************************************************/
int32_t modbus_rtu::modbus_rtu_pack_write_reg
(
                modbus_rtu_write_reg_info_st *reg_info,
                uint8_t *data,
                uint32_t *datalen
                )
{
    int32_t ret = M_MODBUS_ERR;
    uint16_t crc_val = 0;
    uint8_t index = 0;
    uint8_t templen = 0;
    uint8_t register_val_len = 0;

    M_MODBUS_TRACE_IN();

    do
    {
        /* 入参校验 */
        if ((nullptr == reg_info) || (nullptr == data) || (nullptr == datalen))
        {
            M_MODBUS_LOG_ERROR("input param is error, null point");
            break;
        }

        /* 寄存器个数为0，直接退出 */
        if (0x00 == reg_info->register_cnt)
        {
            M_MODBUS_LOG_ERROR("input param write register count is 0x00, exit.");
            break;
        }

        /* 按modbus协议组包 */
        data[templen++] = reg_info->slave_addr;
        data[templen++] = static_cast<uint8_t>(reg_info->func_code);

        /* 组包寄存器地址 */
        data[templen++] = static_cast<uint8_t>((reg_info->register_addr >> 8) & 0xff);
        data[templen++] = static_cast<uint8_t>(reg_info->register_addr & 0xff);

        if (0x01 == reg_info->register_cnt)
        {
            /* 单个状态/保存寄存器 */
            data[templen++] = static_cast<uint8_t>((reg_info->reg_val.register_val[0] >> 8) & 0xff);
            data[templen++] = static_cast<uint8_t>(reg_info->reg_val.register_val[0] & 0xff);
        }
        else
        {
            /* 写寄存器个数 */
            data[templen++] = 0x00;
            data[templen++] = reg_info->register_cnt;

            if (E_FUNC_CODE_WRITE_MULTIPLE_COILS == reg_info->func_code)
            {
                /* 写多个线圈的字节数长度 */
                register_val_len = reg_info->register_cnt / 8 + (reg_info->register_cnt % 8 ? 1 : 0);
                data[templen++] = register_val_len;

                /* 多个线圈状态值 */
                (void)memcpy(&data[templen], reg_info->reg_val.multiple_status_reg_val, register_val_len);
                templen += register_val_len;
            }
            else if (E_FUNC_CODE_WRITE_MULTIPLE_REGISTERS == reg_info->func_code)
            {
                /* 写多个寄存器字节长度 */
                data[templen++] = reg_info->register_cnt * 2;

                for (index = 0; index < reg_info->register_cnt; index++)
                {
                    data[templen++] = static_cast<uint8_t>((reg_info->reg_val.register_val[index] >> 8) & 0xff);
                    data[templen++] = static_cast<uint8_t>(reg_info->reg_val.register_val[index] & 0xff);
                }
            }
            else
            {
                M_MODBUS_LOG_ERROR("write multiple register, func code[%d] error", reg_info->func_code);
                break;
            }
        }

        /* crc校验 */
        ret = modbus_calc_crc(data, templen, &crc_val);
        if (M_MODBUS_OK != ret)
        {
            M_MODBUS_LOG_ERROR("calc crc is failed, ret = %d", ret);
            break;
        }

        data[templen++] = static_cast<uint8_t>((crc_val >> 8) & 0xff);
        data[templen++] = static_cast<uint8_t>(crc_val & 0xff);

        *datalen = templen;

        /* 打印组包数据 */
        modbus_log_hex_print(data, *datalen);
        M_MODBUS_LOG_DEBUG("package write register data ok");
    }
    while (0);

    M_MODBUS_TRACE_OUT();

    return ret;

}

/**************************************************************************
* 函  数: int32_t modbus_rtu_unpack_write_signal_reg(modbus_read_reg_info_st *reg_info,
    uint8_t *data, uint32_t datalen)
* 描  述: 按Modbus协议，解析读取单个状态/存储寄存器的响应数据
* 举  例: 对于单个状态/存储寄存器来说,组包格式,按字节的方式:
      发送->slave_addr(1) + func(1) + reg_addr(2) + reg_val(2) + crc(2)
      应答->slave_addr(1) + func(1) + reg_addr(2) + reg_val(2) + crc(2)
* 入  参: modbus_read_reg_info_st *reg_info : 输入读写寄存器信息
      uint8_t *data : 按modbus协议接收的数据流
      uint32_t datalen : 数据流长度
* 出  参: void
* 返回值: int32_t : M_MODBUS_OK  - 成功
        M_MODBUS_ERR - 失败
**************************************************************************/
int32_t modbus_rtu::modbus_rtu_unpack_write_signal_reg
(
                modbus_rtu_write_reg_info_st *reg_info,
                uint8_t *data,
                uint32_t datalen
                )
{
    int32_t ret = M_MODBUS_ERR;
    modbus_rtu_ack_write_signal_reg_st *write_signal_reg = nullptr;

    M_MODBUS_TRACE_IN();

    do
    {
        /* 入参判断 */
        if ((nullptr == reg_info) || (nullptr == data) || (3 >= datalen))
        {
            M_MODBUS_LOG_ERROR("input param is error, null point");
            break;
        }

        write_signal_reg = (modbus_rtu_ack_write_signal_reg_st *)data;

        /* 寄存器地址和寄存器值的校验 */
        write_signal_reg->register_addr = ntohs(write_signal_reg->register_addr);
        write_signal_reg->register_data = ntohs(write_signal_reg->register_data);

        if (reg_info->register_addr != write_signal_reg->register_addr)
        {
            M_MODBUS_LOG_ERROR("recv write_signal_reg addr(%x) != recv write signal addr(%x)",
                               reg_info->register_addr, write_signal_reg->register_addr);
            break;
        }

        if (reg_info->reg_val.register_val[0] != write_signal_reg->register_data)
        {
            M_MODBUS_LOG_ERROR("recv write_signal_reg val(%x) != recv write signal val(%x)",
                               reg_info->reg_val.register_val[0], write_signal_reg->register_data);
            break;
        }

        M_MODBUS_LOG_DEBUG("recv write signal register data ok");
        ret = M_MODBUS_OK;
        break;
    }
    while (0);

    M_MODBUS_TRACE_OUT();

    return ret;

}

/**************************************************************************
* 函  数: int32_t modbus_rtu_unpack_write_multiple_state_reg(
       modbus_read_reg_info_st *reg_info, uint8_t *data, uint32_t datalen)
* 描  述: 按Modbus协议，解析读取多个状态寄存器的响应数据
* 举  例: 对于多个状态寄存器来说,组包格式,按bit的方式:
      发送->slave_addr(1) + func(1) + reg_addr(2) + reg_cnt(2) + reg_val_len(1) + reg_val(n) + crc(2)
      应答->slave_addr(1) + func(1) + reg_addr(2) + reg_cnt(2) + reg_val_len(1) + crcc(2)
* 入  参: modbus_read_reg_info_st *reg_info : 输入读写寄存器信息
      uint8_t *data : 按modbus协议接收的数据流
      uint32_t datalen : 数据流长度
* 出  参: void
* 返回值: int32_t : M_MODBUS_OK  - 成功
        M_MODBUS_ERR - 失败
**************************************************************************/
int32_t modbus_rtu::modbus_rtu_unpack_write_multiple_state_reg
(
                modbus_rtu_write_reg_info_st *reg_info,
                uint8_t *data,
                uint32_t datalen
                )
{
    int32_t ret = M_MODBUS_ERR;
    uint8_t write_data_len = 0;
    modbus_rtu_ack_write_multiple_state_reg_st *write_state_reg = nullptr;

    M_MODBUS_TRACE_IN();

    do
    {
        /* 入参判断 */
        if ((nullptr == reg_info) || (nullptr == data) || (3 >= datalen))
        {
            M_MODBUS_LOG_ERROR("input param is error, null point");
            break;
        }

        write_state_reg = (modbus_rtu_ack_write_multiple_state_reg_st *)data;

        /* 寄存器地址和寄存器个数，寄存器长度进行校验*/
        write_state_reg->register_addr = ntohs(write_state_reg->register_addr);
        write_state_reg->register_cnt = ntohs(write_state_reg->register_cnt);

        if (reg_info->register_addr != write_state_reg->register_addr)
        {
            M_MODBUS_LOG_ERROR("write_multiple_status_reg addr(%x) != recv write multiple_status addr(%x)",
                               reg_info->register_addr, write_state_reg->register_addr);
            break;
        }

        if (static_cast<uint16_t>(reg_info->register_cnt != write_state_reg->register_cnt))
        {
            M_MODBUS_LOG_ERROR("write_multiple_status_cnt(%x) != recv write multiple_status cnt(%x)",
                               reg_info->register_cnt, write_state_reg->register_cnt);
            break;
        }

        write_data_len = reg_info->register_cnt / 8 + (reg_info->register_cnt % 8 ? 1 : 0);
        if (write_data_len != write_state_reg->register_data_len)
        {
            M_MODBUS_LOG_ERROR("write_multiple_status_data_len(%x) != recv write multiple_status data len(%x)",
                               write_data_len, write_state_reg->register_data_len);
            break;
        }

        M_MODBUS_LOG_DEBUG("recv write multiple_status data ok");
        ret = M_MODBUS_OK;
    }
    while (0);

    M_MODBUS_TRACE_OUT();

    return ret;
}

/**************************************************************************
* 函  数: int32_t modbus_rtu_unpack_write_multiple_storage_reg(
       modbus_read_reg_info_st *reg_info, uint8_t *data, uint32_t datalen)
* 描  述: 按Modbus协议，解析读取多个存储寄存器的响应数据
* 举  例: 对于多个存储寄存器来说，组包格式，按字节的方式:
      发送->slave_addr(1) + func(1) + reg_addr(2) + reg_cnt(2) + reg_val_len(1) + reg_val(n) + crc(2)
      应答->slave_addr(1) + func(1) + reg_addr(2) + reg_cnt(2) + crcc(2)
* 入  参: modbus_read_reg_info_st *reg_info : 输入读写寄存器信息
      uint8_t *data : 按modbus协议接收的数据流
      uint32_t datalen : 数据流长度
* 出  参: void
* 返回值: int32_t : M_MODBUS_OK  - 成功
        M_MODBUS_ERR - 失败
**************************************************************************/
int32_t modbus_rtu::modbus_rtu_unpack_write_multiple_storage_reg
(
                modbus_rtu_write_reg_info_st *reg_info,
                uint8_t *data,
                uint32_t datalen
                )
{
    int32_t ret = M_MODBUS_ERR;
    modbus_rtu_ack_write_multiple_storage_reg_st *write_storage_reg = nullptr;

    M_MODBUS_TRACE_IN();

    do
    {
        /* 入参判断 */
        if ((nullptr == reg_info) || (nullptr == data) || (3 >= datalen))
        {
            M_MODBUS_LOG_ERROR("input param is error, null point");
            break;
        }

        write_storage_reg = (modbus_rtu_ack_write_multiple_storage_reg_st *)data;

        /* 寄存器地址和寄存器个数，寄存器长度进行校验*/
        write_storage_reg->register_addr = ntohs(write_storage_reg->register_addr);
        write_storage_reg->register_cnt = ntohs(write_storage_reg->register_cnt);

        if (reg_info->register_addr != write_storage_reg->register_addr)
        {
            M_MODBUS_LOG_ERROR("write_multiple_storage_reg addr(%x) != recv write multiple_storage addr(%x)",
                               reg_info->register_addr, write_storage_reg->register_addr);
            break;
        }

        if (static_cast<uint16_t>(reg_info->register_cnt != write_storage_reg->register_cnt))
        {
            M_MODBUS_LOG_ERROR("write_multiple_storage_cnt(%x) != recv write multiple_storage cnt(%x)",
                               reg_info->register_cnt, write_storage_reg->register_cnt);
            break;
        }

        M_MODBUS_LOG_DEBUG("recv write multiple_storage data ok");
        ret = M_MODBUS_OK;
    }
    while (0);

    M_MODBUS_TRACE_OUT();

    return ret;
}

/**************************************************************************
* 函  数: int32_t modbus_rtu_unpack_write_reg(modbus_read_reg_info_st *reg_info,
    uint8_t *data, uint32_t datalen)
* 描  述: 按Modbus协议，解析读取寄存器的响应数据
* 举  例: 对于单个状态/存储寄存器来说,组包格式,按字节的方式:
      发送->slave_addr(1) + func(1) + reg_addr(2) + reg_val(2) + crc(2)
      应答->slave_addr(1) + func(1) + reg_addr(2) + reg_val(2) + crc(2)
      对于多个状态寄存器来说,组包格式,按bit的方式:
      发送->slave_addr(1) + func(1) + reg_addr(2) + reg_cnt(2) + reg_val_len(1) + reg_val(n) + crc(2)
      应答->slave_addr(1) + func(1) + reg_addr(2) + reg_cnt(2) + reg_val_len(1) + crcc(2)
      对于多个存储寄存器来说，组包格式，按字节的方式:
      发送->slave_addr(1) + func(1) + reg_addr(2) + reg_cnt(2) + reg_val_len(1) + reg_val(n) + crc(2)
      应答->slave_addr(1) + func(1) + reg_addr(2) + reg_cnt(2) + crcc(2)
* 入  参: modbus_read_reg_info_st *reg_info : 输入读写寄存器信息
      uint8_t *data : 按modbus协议接收的数据流
      uint32_t datalen : 数据流长度
* 出  参: void
* 返回值: int32_t : M_MODBUS_OK  - 成功
        M_MODBUS_ERR - 失败
**************************************************************************/
int32_t modbus_rtu::modbus_rtu_unpack_write_reg
(
                modbus_rtu_write_reg_info_st *reg_info,
                uint8_t *data,
                uint32_t datalen
                )
{
    int32_t ret = M_MODBUS_ERR;
    uint16_t crc_val = 0;
    uint16_t recv_crc_val = 0;
    MODBUS_ERROR_CODE_E err_code = E_CODE_NO_ERR;
    modbus_rtu_fail_st *fail_info = nullptr;
    modbus_rtu_ack_data_rw_st *data_ack = nullptr;

    M_MODBUS_TRACE_IN();

    do
    {
        /* 入参判断 */
        if ((nullptr == reg_info) || (nullptr == data) || (3 >= datalen))
        {
            M_MODBUS_LOG_ERROR("input param is error, null point");
            break;
        }

        data_ack = (modbus_rtu_ack_data_rw_st *)(data);

        /* 地址是否一致 */
        if (reg_info->slave_addr != data_ack->slave_addr)
        {
            M_MODBUS_LOG_ERROR("write register slave addr[%x] != recv data ack slave addr[%x]",
                               reg_info->slave_addr, data_ack->slave_addr);
            break;
        }

        /* crc校验 */
        ret = modbus_calc_crc(data, datalen - 2, &crc_val);
        if (M_MODBUS_OK != ret)
        {
            M_MODBUS_LOG_ERROR("recv write_signal_reg ack calc crc failed");
            break;
        }

        recv_crc_val = static_cast<uint16_t>(((static_cast<uint16_t>(data[datalen - 2] << 8) \
                                              | static_cast<uint16_t>(data[datalen - 1]))));
        if (crc_val != recv_crc_val)
        {
            M_MODBUS_LOG_ERROR("recv write_signal_reg_crc(%x) not equal calc crc(%x)", recv_crc_val, crc_val);
            ret = M_MODBUS_ERR;
            break;
        }

        /* 错误功能码解析 */
        if (static_cast<uint8_t>(reg_info->func_code != data_ack->func_code))
        {
            if ((static_cast<uint8_t>(reg_info->func_code | 0x80) == data_ack->func_code))
            {
                fail_info = (modbus_rtu_fail_st *)data;

                /* 返回的是错误码 */
                switch (fail_info->except_code)
                {
                case 0x01:
                case 0x02:
                case 0x03:
                case 0x04:
                case 0x05:
                {
                    err_code = static_cast<MODBUS_ERROR_CODE_E>(fail_info->except_code);
                    break;
                }
                default:
                {
                    err_code = E_CODE_OTHER_ERR;
                    break;
                }
                }
            }
            else
            {
                err_code = E_CODE_OTHER_ERR;
            }

            M_MODBUS_LOG_WARN("write signal register data, func code[%x] is fail ack value[%x]",
                              reg_info->func_code, data_ack->func_code);

            ret = M_MODBUS_ERR;
            break;
        }

        switch (reg_info->func_code)
        {
        case E_FUNC_CODE_WRITE_SINGLE_COIL:
        case E_FUNC_CODE_WRITE_SINGLE_REGISTER:
        {
            ret = modbus_rtu_unpack_write_signal_reg(reg_info, data, datalen);
        }
            break;
        case E_FUNC_CODE_WRITE_MULTIPLE_COILS:
        {
            ret = modbus_rtu_unpack_write_multiple_state_reg(reg_info, data, datalen);
        }
            break;
        case E_FUNC_CODE_WRITE_MULTIPLE_REGISTERS:
        {
            ret = modbus_rtu_unpack_write_multiple_storage_reg(reg_info, data, datalen);
        }
            break;
        default:
        {
            /* 其他 */
            M_MODBUS_LOG_WARN("not recv write register data, func code[%x]", reg_info->func_code);
            ret = M_MODBUS_ERR;
        }
        }
    }
    while (0);

    M_MODBUS_TRACE_OUT();

    return ret;
}

/**************************************************************************
* 函  数: int32_t modbus_rtu_init(modbus_com_params_st *com_param)
* 描  述: 初始化modbus rtu 通信口参数
* 入  参: modbus_com_params_st *com_param : 通信口参数信息
* 出  参: void
* 返回值: int32_t : M_MODBUS_OK  - 成功
        M_MODBUS_ERR - 失败
**************************************************************************/
int32_t modbus_rtu::modbus_rtu_init(modbus_com_params_st *com_param)
{
    int32_t ret = M_MODBUS_ERR;

    M_MODBUS_TRACE_IN();
    (void)com_param;
#if 0
    do
    {
        if (NULL == com_param)
        {
            M_MODBUS_LOG_ERROR("input param is error, null point");
            break;
        }
        /* 参数初始化 */
        (void)memset(&g_modbus_com, 0, sizeof(g_modbus_com));
        if (strlen(com_param->device) > 0)
        {
            (void)memcpy(g_modbus_com.device, com_param->device, strlen(com_param->device));
        }

        /* 波特率 */
        if ((E_BAUD_2400BPS > com_param->baud) || (E_BAUD_256000BPS < com_param->baud))
        {
            g_modbus_com.baud = E_BAUD_9600BPS;
        }
        else
        {
            g_modbus_com.baud = com_param->baud;
        }

        /* 数据位 */
        if ((E_DATA_4BITS > com_param->data_bit) || (E_DATA_8BITS < com_param->data_bit))
        {
            g_modbus_com.data_bit = E_DATA_8BITS;
        }
        else
        {
            g_modbus_com.data_bit = com_param->data_bit;
        }

        /* 停止位 */
        g_modbus_com.stop_bit = com_param->stop_bit;
        g_modbus_com.parity = com_param->parity;
    }
    while (0);
#endif
    M_MODBUS_TRACE_OUT();

    return ret;
}


