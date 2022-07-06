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
//int read_mode = 0; //  mode value
// int flag = 1;
// int newCompValue = 0;
// int prevCompValue = 0;
int g_mode = 1;


void display_init();
void Serial_Init(void);
unsigned char USART_Receive(void); //checks when the UDR0 has received all 8bits = 1 byte, then returns it
bool receiveGGA(); //checks the
int mode_selector(void); //mode selector, increases up to 4 when the button is pressed
void print_msg(char final_message[], int mode);
int interrupt_selector(void);

int main()
{
    display_init();
    Serial_Init();
    //PORTB = 0x02; //set the pul up register to portb1 
        
    PORTD = (1 << PORTD2); // pull up resistor 
    EICRA = (1 << ISC01); //falling egde
    EIMSK = (1 << INT0); // interrupt on INT0
    sei();
   
 
    while(1)
    {
       // int mode = mode_selector(); // mode selector -> simple button
       // int mode = interrupt_selector(); // interrupt_selector -> external interrupt with debouncer
        int mode = g_mode;

        if(receiveGGA(mode)==true)
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

bool receiveGGA(int mode)
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
                    if(strncmp(scanned_message,"$GPGGA",6)==0)  // appears only once
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

int mode_selector(void)
{
    static int read_mode = 0;

    static bool is_button_pressed = false;

    if (!(PINB & 0x02))  // 00000010 and 00000010 = 00000010   PRESSED
    {
        

        if(is_button_pressed == false)
        {
            read_mode = read_mode + 1;    

            if(read_mode > 4)
            {
            read_mode = 1;                  
            } 


            OLED_SetCursor(3,0);
            OLED_Printf("Mode: %d",read_mode);
            

        }
        is_button_pressed = true;

    }
    else
    {
        is_button_pressed = false;
    }


    return read_mode;       
}

void print_msg(char final_message[],int mode)
{
    OLED_SetCursor(3,0);
    OLED_Printf("Mode: %d", mode);

    
    OLED_SetCursor(4,0);
    OLED_Printf("Data: %s", final_message);
}

// int interrupt_selector(void)
// {
   
//     sei();
//     if (prevCompValue == newCompValue)
//     {
//         flag = flag;               
//     }
//     else
//     {
//         flag++;
//         _delay_ms(500);

//         if(flag > 4)
//         {
//             flag = 1;
//         }
//         prevCompValue=newCompValue;    
//         }

//     OLED_SetCursor(1,0);
//     OLED_Printf("flag: %d",flag);  // Flag Value Control
//     return flag;
// }

ISR(INT0_vect)
{
    //newCompValue ++;  

    EIMSK &= ~(1 << INT0); // disable interrupt on INT0
    unsigned char pin_state_at_start = PIND & 0b000000100;
    _delay_us(200); // TODO: tuning required
    unsigned char pin_state_at_end = PIND & 0b00000100;
    if (pin_state_at_start == pin_state_at_end) {
        g_mode++;
        // pin state changed
        if(g_mode>4)
        {
            g_mode=1;
        }
    }
    EIMSK |= (1 << INT0); // enable interrupt on INT0
}



