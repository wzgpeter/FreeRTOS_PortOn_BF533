//*************************************************************************************
//*************************************************************************************
//File Name: i2c.c
//Notes:
//Author: wu, zhigang
//Date:   May-6-2017
//*************************************************************************************
//*************************************************************************************


#include "stdint.h"
#include "freeRTOS.h"
#include "i2c.h"

void delay(void)
{
    asm("nop; nop; nop; nop; nop;");
    asm("nop; nop; nop; nop; nop;");
    asm("nop; nop; nop; nop; nop;");
    asm("nop; nop; nop; nop; nop;");
    asm("nop; nop; nop; nop; nop;");
}



#define HIGH            (1)
#define LOW             (0)
#define bI2C_SDA        (0)
#define bI2C_SCL        (1)       
#define pGpio_Dirct         (pFIO_DIR)
#define pGpio_Flag_Data     (pFIO_FLAG_D)
#define pGpio_Flag_Set      (pFIO_FLAG_S)
#define pGpio_Flag_Clear    (pFIO_FLAG_C)


#define I2C_SDA_CFG_OUT()   mmr_write16(pGpio_Dirct, mmr_read16(pGpio_Dirct) | (1<<bI2C_SDA))
#define I2C_SCL_CFG_OUT()   mmr_write16(pGpio_Dirct, mmr_read16(pGpio_Dirct) | (1<<bI2C_SCL))
#define I2C_SDA_CFG_IN()    mmr_write16(pGpio_Dirct, mmr_read16(pGpio_Dirct) & (~(1<<bI2C_SDA)))
#define I2C_SCL_CFG_IN()    mmr_write16(pGpio_Dirct, mmr_read16(pGpio_Dirct) & (~(1<<bI2C_SCL)))


#define I2C_SDA_SET_HIGH()  mmr_write16(pGpio_Flag_Set, mmr_read16(pGpio_Flag_Set) | (1<<bI2C_SDA))
#define I2C_SCL_SET_HIGH()  mmr_write16(pGpio_Flag_Set, mmr_read16(pGpio_Flag_Set) | (1<<bI2C_SCL))
#define I2C_SDA_SET_LOW()   mmr_write16(pGpio_Flag_Clear, mmr_read16(pGpio_Flag_Clear) | (1<<bI2C_SDA))
#define I2C_SCL_SET_LOW()   mmr_write16(pGpio_Flag_Clear, mmr_read16(pGpio_Flag_Clear) | (1<<bI2C_SCL))

#define I2C_SDA_OUT(o)      if( (o)==1 ) I2C_SDA_SET_HIGH(); \
                            else     I2C_SDA_SET_LOW();

#define I2C_SCL_OUT(o)      if( (o)==1 ) I2C_SCL_SET_HIGH(); \
                            else     I2C_SCL_SET_LOW();


#define I2C_SDA_GET()       (mmr_read16(pGpio_Flag_Data) & (1<<bI2C_SDA))

void i2c_start(void)
{
    //init I2C port  
    I2C_SDA_CFG_OUT();     //set SDA as output  
    I2C_SCL_CFG_OUT();     //set SCL as output  
    I2C_SDA_OUT(HIGH);     //set SDA pin as high level  
    I2C_SCL_OUT(HIGH);     //set SCL pin as high level  
    delay();
    
    //start conditon  
    I2C_SDA_SET_LOW();     //when SCL is High, switch SDA from High to Low
    delay();
} 


void i2c_stop(void)
{  
    I2C_SCL_OUT(HIGH);     //set_gpio_value(SCL, 1);  
    I2C_SDA_CFG_OUT();     //set_gpio_direction(SDA, OUTP);  
    I2C_SDA_OUT(LOW);      //set_gpio_value(SDA, 0);  
    delay();  
    I2C_SDA_OUT(HIGH);     //set_gpio_value(SDA, 1);     //SCL高电平时，SDA由低变高  
}


/*   
I2C读取ACK信号(写数据时使用)  
返回值 ：0表示ACK信号有效；非0表示ACK信号无效  
*/  
unsigned char i2c_read_ack(void)
{  
    unsigned char r;  
    I2C_SDA_CFG_IN();      //set_gpio_direction(SDA, INP);       //设置SDA方向为输入  
    I2C_SCL_OUT(LOW);      //set_gpio_value(SCL,0);              // SCL变低  
    r = I2C_SDA_GET();     //r = get_gpio_value(SDA);            //读取ACK信号  
    delay();  
    I2C_SCL_OUT(HIGH);     //set_gpio_value(SCL,1);              // SCL变高  
    delay();  
    return r;  
} 


/* I2C发出ACK信号(读数据时使用) */  
void i2c_send_ack(void)
{  
    I2C_SDA_CFG_OUT();     //set_gpio_direction(SDA, OUTP);      //设置SDA方向为输出  
    I2C_SCL_OUT(LOW);      //set_gpio_value(SCL,0);              // SCL变低  
    I2C_SDA_OUT(LOW);      //set_gpio_value(SDA, 0);             //发出ACK信号  
    delay();  
    I2C_SCL_OUT(HIGH);     //set_gpio_value(SCL,1);              // SCL变高  
    delay();  
}  


/* I2C字节写 */  
void i2c_write_byte(unsigned char b)
{  
    int i;  
    I2C_SDA_CFG_OUT();              //set_gpio_direction(SDA, OUTP);          //设置SDA方向为输出  
    for (i=7; i>=0; i--) 
    {  
        I2C_SCL_OUT(LOW);           //set_gpio_value(SCL, 0);             // SCL变低  
        delay();  
        I2C_SDA_OUT( b & (1<<i) );  //set_gpio_value(SDA, b & (1<<i));        //从高位到低位依次准备数据进行发送  
        I2C_SCL_OUT(HIGH);          //set_gpio_value(SCL, 1);             // SCL变高  
        delay();  
    }  
    i2c_read_ack();                 //check the target's ACK signal  
}  


/* I2C字节读 */  
unsigned char i2c_read_byte(void)
{  
    int i;  
    unsigned char r = 0;  
    I2C_SDA_CFG_IN();                       //set_gpio_direction(SDA, INP);       //设置SDA方向为输入  
    for (i=7; i>=0; i--) 
    {  
        I2C_SCL_OUT(LOW);                   //set_gpio_value(SCL, 0);             // SCL变低  
        delay();  
        r = (r <<1) | I2C_SDA_GET();        //get_gpio_value(SDA);  //从高位到低位依次准备数据进行读取  
        I2C_SCL_OUT(HIGH);                  //set_gpio_value(SCL, 1);             // SCL变高  
        delay();  
    }  
    i2c_send_ack();                         //send ACK signal to target
    return r; 
}


/*  
I2C读操作  
addr：目标设备地址  
buf：读缓冲区  
len：读入字节的长度  
*/  
void i2c_read(unsigned char addr, unsigned char* buf, int len)
{  
    int i;  
    unsigned char t;  

    i2c_start();                   //start to communication  

    //发送地址和数据读写方向  
    t = (addr << 1) | 1;           //low bit"1" represents read  
    i2c_write_byte(t);  

    //读入数据  
    for (i=0; i<len; i++)  
        buf[i] = i2c_read_byte();  
    i2c_stop();                    //stop communication  
} 


/*  
I2C写操作  
addr：目标设备地址  
buf：写缓冲区  
len：写入字节的长度  
*/  
void i2c_write(unsigned char addr, unsigned char* buf, int len)
{  
    int i;  
    unsigned char t;  

    i2c_start();                 //start to communication  

    //发送地址和数据读写方向  
    t = (addr << 1) | 0;         //low bit"0" represents write  
    i2c_write_byte(t);
    
    //写入数据  
    for (i=0; i<len; i++)  
        i2c_write_byte(buf[i]);  

    i2c_stop();                  //stop communication 
} 



