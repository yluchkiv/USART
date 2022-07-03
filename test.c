#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include "i2c.h"
#include "SSD1306.h"

int counter = 0;

int checker(void);


void prt_reg(void)
{
   // DDRB = 0x01; //portb0 output
    PORTB = 0x02; //set the pul up register to portb1 
}

void result(int res)
{
    OLED_SetCursor(1,0);
    OLED_Printf("Result %d ",res);
}


int checker(void)
    {
        if (!(PINB & 0x02))  // 00000010 and 00000010 = 00000010  NOT PRESSED
        {
            counter = counter + 1;
                
            _delay_ms(500);
                


        }
        return counter;

    }


int main()
{
    prt_reg();
    OLED_Init();

    while(1)
    {
        

        //PORTB ^= 0x01;
        OLED_SetCursor(0,0);
        OLED_Printf("Init %d ",counter);
        checker();

        

    

        result(counter);

        OLED_SetCursor(3,0);
        OLED_Printf("Escape");
        


    }




}
