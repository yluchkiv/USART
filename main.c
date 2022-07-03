#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include "i2c.h"
#include "SSD1306.h"

#define BUFFER_SIZE 256
#define BUAD 9600
#define BRC ((F_CPU/16/BUAD)-1)  // = 103.16 // 0110 0111

char scanned_message[BUFFER_SIZE+1]={'\0'}; // fill the buffer with null 

void Serial_Init(void);
unsigned char USART_Receive(void);

void receiveGGA();
void selectGGA(char imput_message[]);



int main()
{
   
    OLED_Init();  //initialize the OLED
    OLED_SetCursor(0, 0);        //set the cursor position to (0 - move down, 0-move left)
    OLED_Printf("Initialized");

    Serial_Init();
 

    while(1)
    {
        
        receiveGGA();

    }

    return 0;

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

void receiveGGA()
{
 
    for (int i = 0; i< BUFFER_SIZE; i++)
    {
        scanned_message[i] = USART_Receive(); // data transfered from UDR0 reg to the buffer
        if((scanned_message[i]=='\r')|(scanned_message[i]=='\n')) //detecting the end of NMEA message
        {
            scanned_message[i]='\0'; // ends the message 
            break;
        }
            
    }

     if(strncmp(scanned_message,"$GPGGA",6)==0) //!!!! - parser GGA
     {
        //OLED_Clear(); // only for display clear
        OLED_SetCursor(0, 0);  
        OLED_Printf("Data: %s",scanned_message);

     }


}



