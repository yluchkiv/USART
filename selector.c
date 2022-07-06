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
int g_mode = 1;

void display_init();
void Serial_Init(void);
void interrupt_Init(void);
unsigned char USART_Receive(void); //checks when the UDR0 has received all 8bits = 1 byte, then returns it
bool receive_NMEA(); //checks the
void print_msg(char final_message[], int mode);

int main()
{
    display_init();
    Serial_Init();
    interrupt_Init();
        
    sei();
 
    while(1)
    {
        int mode = g_mode;
        if(receive_NMEA(mode)==true)
        {
            print_msg(scanned_message, mode);
        }
    }
    return 0;
}

void display_init()
{
    OLED_Init();  //initialize the OLED
    OLED_SetCursor(0, 0);        //set the cursor position to (0 - move down, 0-move left)
    OLED_Printf("NMEA Scan Initialized");
}

void Serial_Init(void)
{
    UBRR0H = (BRC >> 8); // we need to shift 8 bits of 103 (0110 0111) to fill the lower register
    UBRR0L = BRC; //lower register
    UCSR0B = (1 << RXEN0); // Rx enable
    UCSR0C = (1 << UCSZ00)|(1 << UCSZ01)|(1 << USBS0); // 8 bit data format
}
void interrupt_Init(void)
{
    PORTD = (1 << PORTD2); // pull up resistor 
    EICRA = (1 << ISC01); //falling egde
    EIMSK = (1 << INT0); // interrupt on INT0
}

unsigned char USART_Receive(void)
{
    while ( !(UCSR0A & (1 << RXC0)) ); //Wait for data to be received 
    return UDR0; // Get and return received data from buffer 
}

bool receive_NMEA(int mode)
{
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        scanned_message[i] = USART_Receive(); // data transfered from UDR0 reg to the buffer
        if((i > 0)&&(scanned_message[i-1]=='\r') && (scanned_message[i]=='\n')) //detecting the end of NMEA message
        {
            scanned_message[i-1]='\0'; // ends the message 

            switch(mode)
            {
                case 1:
                    if(strncmp(scanned_message,"$GPGGA",6)==0)  
                    {
                        return true;
                    } 
                    break;
                    
                case 2:
                    if(strncmp(scanned_message,"$GPRMC",6)==0)
                    {
                        return true;
                    } 
                    break;

                case 3:
                    if(strncmp(scanned_message,"$GPGSA",6)==0)
                    {
                        return true;
                    } 
                    break;

                case 4:
                    if(strncmp(scanned_message,"$GPGSV",6)==0)
                    {
                        return true;
                    } 
                    break;
            }
            return false;
        }
    }
    return false;
}     

void print_msg(char final_message[],int mode)
{
    switch(mode)
    {
        case 1:
        OLED_SetCursor(2,0);
        OLED_Printf("Message selected: GGA");
        break;

        case 2:
        OLED_SetCursor(2,0);
        OLED_Printf("Message selected: RMC");
        break;

        case 3:
        OLED_SetCursor(2,0);
        OLED_Printf("Message selected: GSA");
        break;

        case 4:
        OLED_SetCursor(2,0);
        OLED_Printf("Message selected: GSV");
        break;

    } 

    OLED_SetCursor(4,0);
    OLED_Printf("Data: %s", final_message);
}

ISR(INT0_vect)
{
    EIMSK &= ~(1 << INT0); // disable interrupt on INT0 to prevent button bounce-counting
    unsigned char pin_state_at_start = PIND & 0b000000100;
    _delay_us(500); // TODO: tuning required
    unsigned char pin_state_at_end = PIND & 0b00000100;
    if (pin_state_at_start == pin_state_at_end) {
        g_mode++;
      
        if(g_mode>4)
        {
            g_mode=1;
        }
    }
    EIMSK |= (1 << INT0); // enable interrupt on INT0
}



