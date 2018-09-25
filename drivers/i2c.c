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
    I2C_SDA_OUT(HIGH);     //set_gpio_value(SDA, 1);     //SCL�ߵ�ƽʱ��SDA�ɵͱ��  
}


/*   
I2C��ȡACK�ź�(д����ʱʹ��)  
����ֵ ��0��ʾACK�ź���Ч����0��ʾACK�ź���Ч  
*/  
unsigned char i2c_read_ack(void)
{  
    unsigned char r;  
    I2C_SDA_CFG_IN();      //set_gpio_direction(SDA, INP);       //����SDA����Ϊ����  
    I2C_SCL_OUT(LOW);      //set_gpio_value(SCL,0);              // SCL���  
    r = I2C_SDA_GET();     //r = get_gpio_value(SDA);            //��ȡACK�ź�  
    delay();  
    I2C_SCL_OUT(HIGH);     //set_gpio_value(SCL,1);              // SCL���  
    delay();  
    return r;  
} 


/* I2C����ACK�ź�(������ʱʹ��) */  
void i2c_send_ack(void)
{  
    I2C_SDA_CFG_OUT();     //set_gpio_direction(SDA, OUTP);      //����SDA����Ϊ���  
    I2C_SCL_OUT(LOW);      //set_gpio_value(SCL,0);              // SCL���  
    I2C_SDA_OUT(LOW);      //set_gpio_value(SDA, 0);             //����ACK�ź�  
    delay();  
    I2C_SCL_OUT(HIGH);     //set_gpio_value(SCL,1);              // SCL���  
    delay();  
}  


/* I2C�ֽ�д */  
void i2c_write_byte(unsigned char b)
{  
    int i;  
    I2C_SDA_CFG_OUT();              //set_gpio_direction(SDA, OUTP);          //����SDA����Ϊ���  
    for (i=7; i>=0; i--) 
    {  
        I2C_SCL_OUT(LOW);           //set_gpio_value(SCL, 0);             // SCL���  
        delay();  
        I2C_SDA_OUT( b & (1<<i) );  //set_gpio_value(SDA, b & (1<<i));        //�Ӹ�λ����λ����׼�����ݽ��з���  
        I2C_SCL_OUT(HIGH);          //set_gpio_value(SCL, 1);             // SCL���  
        delay();  
    }  
    i2c_read_ack();                 //check the target's ACK signal  
}  


/* I2C�ֽڶ� */  
unsigned char i2c_read_byte(void)
{  
    int i;  
    unsigned char r = 0;  
    I2C_SDA_CFG_IN();                       //set_gpio_direction(SDA, INP);       //����SDA����Ϊ����  
    for (i=7; i>=0; i--) 
    {  
        I2C_SCL_OUT(LOW);                   //set_gpio_value(SCL, 0);             // SCL���  
        delay();  
        r = (r <<1) | I2C_SDA_GET();        //get_gpio_value(SDA);  //�Ӹ�λ����λ����׼�����ݽ��ж�ȡ  
        I2C_SCL_OUT(HIGH);                  //set_gpio_value(SCL, 1);             // SCL���  
        delay();  
    }  
    i2c_send_ack();                         //send ACK signal to target
    return r; 
}


/*  
I2C������  
addr��Ŀ���豸��ַ  
buf����������  
len�������ֽڵĳ���  
*/  
void i2c_read(unsigned char addr, unsigned char* buf, int len)
{  
    int i;  
    unsigned char t;  

    i2c_start();                   //start to communication  

    //���͵�ַ�����ݶ�д����  
    t = (addr << 1) | 1;           //low bit"1" represents read  
    i2c_write_byte(t);  

    //��������  
    for (i=0; i<len; i++)  
        buf[i] = i2c_read_byte();  
    i2c_stop();                    //stop communication  
} 


/*  
I2Cд����  
addr��Ŀ���豸��ַ  
buf��д������  
len��д���ֽڵĳ���  
*/  
void i2c_write(unsigned char addr, unsigned char* buf, int len)
{  
    int i;  
    unsigned char t;  

    i2c_start();                 //start to communication  

    //���͵�ַ�����ݶ�д����  
    t = (addr << 1) | 0;         //low bit"0" represents write  
    i2c_write_byte(t);
    
    //д������  
    for (i=0; i<len; i++)  
        i2c_write_byte(buf[i]);  

    i2c_stop();                  //stop communication 
} 



