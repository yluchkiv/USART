#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include "i2c.h"
#include "SSD1306.h"
#include <stdbool.h>

#define BUFFER_SIZE 256
#define BUAD 9600
#define BRC ((F_CPU/16/BUAD)-1)  // = 103.16 // 0110 0111

char scanned_message[BUFFER_SIZE+1]={'\0'}; // fill the buffer with null 
int read_mode = 0; //  mode value

void display_init();
void Serial_Init(void);
unsigned char USART_Receive(void); //checks when the UDR0 has received all 8bits = 1 byte, then returns it
bool receiveGGA(); //checks the
int mode_selector(void); //mode selector, increases up to 4 when the button is pressed
void print_msg(char final_message[]);

int main()
{
    display_init();
    Serial_Init();
    PORTB = 0x02; //set the pul up register to portb1 
 
    while(1)
    {
        mode_selector();
        if(receiveGGA(read_mode)==true)
        {
            print_msg(scanned_message);
        }

    }
    return 0;

}

void display_init()
{
    OLED_Init();  //initialize the OLED
    OLED_SetCursor(0, 0);        //set the cursor position to (0 - move down, 0-move left)
    OLED_Printf("Initialized");
}

void Serial_Init(void)
{
    UBRR0H = (BRC >> 8); // we need to shift 8 bits of 103 (0110 0111) to fill the lower register
    UBRR0L = BRC; //lower register
    UCSR0B = (1 << RXEN0); // Rx enable
    UCSR0C = (1 << UCSZ00)|(1 << UCSZ01)|(1 << USBS0); // 8 bit data format
}

unsigned char USART_Receive(void)
{
    while ( !(UCSR0A & (1 << RXC0)) ); //Wait for data to be received 
    return UDR0; // Get and return received data from buffer 
}

bool receiveGGA(int a)
{
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        scanned_message[i] = USART_Receive(); // data transfered from UDR0 reg to the buffer
        if((i > 0)&&(scanned_message[i-1]=='\r') && (scanned_message[i]=='\n')) //detecting the end of NMEA message
        {
            scanned_message[i-1]='\0'; // ends the message 
            switch(a)
            {
                case 1:
                    if((a == 1)&&(strncmp(scanned_message,"$GPGGA",6)==0))
                    {
                        OLED_SetCursor(3,0);
                        OLED_Printf("Mode: %d",a);
                        return true;
                    } 
                
                case 2:
                    if((a == 2)&&(strncmp(scanned_message,"$GPRMC",6)==0))
                    {
                        OLED_SetCursor(3,0);
                        OLED_Printf("Mode: %d",a);
                        return true;
                    } 
            }
            return false;
        }
    }
    return false;
}     

int mode_selector(void)
{
    if (!(PINB & 0x02))  // 00000010 and 00000010 = 00000010   PRESSED
    {
        read_mode = read_mode + 1;    
        _delay_ms(300);

        if(read_mode > 2)
        {
            read_mode = 0;                  
        } 
    }
    return read_mode;       
}

void print_msg(char final_message[])
{
    OLED_SetCursor(4,0);
    OLED_Printf("Data: %s", final_message);
}


