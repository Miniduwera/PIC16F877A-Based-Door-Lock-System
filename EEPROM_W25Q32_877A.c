// CONFIG

#define RS RD2
#define EN RD3
#define D4 RD4
#define D5 RD5
#define D6 RD6
#define D7 RD7

#pragma config FOSC = HS       // Oscillator Selection bits (HS oscillator)

#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)

#pragma config PWRTE = OFF       // Power-up Timer Enable bit (PWRT enabled)

#pragma config BOREN = OFF        // Brown-out Reset Enable bit (BOR enabled)

#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)

#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)

#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)

#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

//End of CONFIG registers


#define _XTAL_FREQ 16000000
#include <stdio.h>
#include<xc.h>
#include <conio.h>
#include <string.h>
#include <stdint.h>
#define MAX_STRING_SIZE 10 // Adjust the size based on your requirements

#define but_prss PORTBbits.RB7
#define but_out PORTBbits.RB6
#define but_out_2 PORTBbits.RB5
__EEPROM_DATA('1', '2', '3', '4', '5', '6', 0x00, 0x00);
char receivedString[MAX_STRING_SIZE];
//////////////////////////////////////////////////
void eeprom_write_string(int address, char *data) {
    while (*data != '\0') {
        eeprom_write(address, *data);
        address++;
        data++;
       // __delay_ms(100);
    }
    // Write the null terminator to indicate the end of the string
    eeprom_write(address, '\0');
}

void eeprom_read_string(int address, char *buffer) {
    char data;
    do {
        data = (char)eeprom_read(address);
        *buffer = data;
        buffer++;
        address++;
    } while (data != '\0');
}
//*****Initialize Bluetooth using USART*******//
void Lcd_SetBit(char data_bit) //Based on the Hex value Set the Bits of the Data Lines
{
    if(data_bit& 1) 
        D4 = 1;
    else
        D4 = 0;
    if(data_bit& 2)
        D5 = 1;
    else
        D5 = 0;
    if(data_bit& 4)
        D6 = 1;
    else
        D6 = 0;
    if(data_bit& 8) 
        D7 = 1;
    else
        D7 = 0;
}
void Lcd_Cmd(char a)
{
    RS = 0;           
    Lcd_SetBit(a); //Incoming Hex value
    EN  = 1;         
        __delay_ms(4);
        EN  = 0;         
}
Lcd_Clear(void)
{
    Lcd_Cmd(0); //Clear the LCD
    Lcd_Cmd(1); //Move the curser to first position
}
void Lcd_Set_Cursor(char a, char b)
{
    char temp,z,y;
    if(a== 1)
    {
      temp = 0x80 + b - 1; //80H is used to move the curser
        z = temp>>4; //Lower 8-bits
        y = temp & 0x0F; //Upper 8-bits
        Lcd_Cmd(z); //Set Row
        Lcd_Cmd(y); //Set Column
    }
    else if(a== 2)
    {
        temp = 0xC0 + b - 1;
        z = temp>>4; //Lower 8-bits
        y = temp & 0x0F; //Upper 8-bits
        Lcd_Cmd(z); //Set Row
        Lcd_Cmd(y); //Set Column
    }
}
void Lcd_Start()
{
  Lcd_SetBit(0x00);
  for(int i=1065244; i<=0; i--)  NOP();  
  Lcd_Cmd(0x03);
    __delay_ms(5);
  Lcd_Cmd(0x03);
    __delay_ms(11);
  Lcd_Cmd(0x03); 
  Lcd_Cmd(0x02); //02H is used for Return home -> Clears the RAM and initializes the LCD
  Lcd_Cmd(0x02); //02H is used for Return home -> Clears the RAM and initializes the LCD
  Lcd_Cmd(0x08); //Select Row 1
  Lcd_Cmd(0x00); //Clear Row 1 Display
  Lcd_Cmd(0x0C); //Select Row 2
  Lcd_Cmd(0x00); //Clear Row 2 Display
  Lcd_Cmd(0x06);
}
void Lcd_Print_Char(char data)  //Send 8-bits through 4-bit mode
{
   char Lower_Nibble,Upper_Nibble;
   Lower_Nibble = data&0x0F;
   Upper_Nibble = data&0xF0;
   RS = 1;             // => RS = 1
   Lcd_SetBit(Upper_Nibble>>4);             //Send upper half by shifting by 4
   EN = 1;
   for(int i=2130483; i<=0; i--)  NOP(); 
   EN = 0;
   Lcd_SetBit(Lower_Nibble); //Send Lower half
   EN = 1;
   for(int i=2130483; i<=0; i--)  NOP();
   EN = 0;
}

void Lcd_Print_String(char *a)
{
    int i;
    for(i=0;a[i]!='\0';i++)
       Lcd_Print_Char(a[i]);  //Split the string using pointers and call the Char function 
}
void Initialize_Bluetooth()
{
   //Set the pins of RX and TX//
    TRISC6=1;
    TRISC7=1;    
  //Set the baud rate using the look up table in datasheet(pg114)//
    BRGH=1;      //Always use high speed baud rate with Bluetooth else it wont work
    SPBRG  =103;
    //Turn on Asyc. Serial Port//
    SYNC=0;
    SPEN=1;
    //Set 8-bit reception and transmission
    RX9=0;
    TX9=0;
   //Enable transmission and reception//
    TXEN=1; 
    CREN=1; 
    //Enable global and ph. interrupts//
    GIE = 1;

    PEIE= 1;
    //Enable interrupts for Tx. and Rx.//
    RCIE=1;
    TXIE=1;
}
//__________BT initialized____________//
//Function to load the Bluetooth Rx. buffer with one char.//

void BT_load_char(char byte)  

{
    TXREG = byte;
    while(!TXIF);  
    while(!TRMT);
}
//End of function//

//Function to Load Bluetooth Rx. buffer with string//
void BT_load_string(char* string)
{
    while(*string)
    BT_load_char(*string++);
}
//End of function//
//Function to broadcast data from RX. buffer//
void broadcast_BT()
{
  TXREG = 13;  
  __delay_ms(500);
}
//End of function//
//Function to get a char from Rx.buffer of BT//
char BT_get_char(void)   
{
    if(OERR) // check for over run error 
    {
        CREN = 0;
        CREN = 1; //Reset CREN
    }    
    if(RCIF==1) //if the user has sent a char return the char (ASCII value)
    {
    while(!RCIF);  
    return RCREG;
    }
    else //if user has sent no message return 0
        return 0;
}
void BT_get_string(void)   
{
    int i = 0;
    
    if (OERR) // Check for over run error 
    {
        CREN = 0;
        CREN = 1; // Reset CREN
    }
    
    // Read characters until newline '\n' is encountered or buffer is full
    while (i < MAX_STRING_SIZE - 1) 
    {
        if (RCIF == 1) // Check if a character is available
        {
            char receivedChar = RCREG;
            
            if (receivedChar == '\n' || receivedChar == '\0') 
            {
                // End of string, null terminate and exit loop
                receivedString[i] = '\0';
                break;
            }
            
            receivedString[i] = receivedChar;
            i++;
        }
    }
    
    // Null terminate the string if buffer is full
    receivedString[MAX_STRING_SIZE - 1] = '\0';
}
unsigned int stat_1=0;
unsigned int stat_a=0;
unsigned int stat_b=0;
unsigned int stat_c=0;
unsigned int stat_d=0;
unsigned int stat_a_1 =0;

void main(void)
{   
    but_out=0;
    but_out_2=0;

    Lcd_Clear();
    Lcd_Start();
    Initialize_Bluetooth();
    TRISBbits.TRISB7 = 1;
    TRISBbits.TRISB6 = 0;
    TRISBbits.TRISB5 = 0;
    TRISD = 0x00;
    
    
    while (1) {
        // Debounce the button press
        
        
        
        if (but_prss == 1 && stat_1 == 0) {
            __delay_ms(50);
            if (but_prss == 1 && stat_1 == 0) {
                // Button is stable, proceed with the initialization
                memset(receivedString, 0, sizeof(receivedString));            
                 
                BT_load_string("Bluetooth Initialized and Ready");
                broadcast_BT();
                BT_load_string("Enter R to Reset Password");
                broadcast_BT();
                BT_load_string("Enter S to Change Answers");
                broadcast_BT();
                BT_load_string("Enter M to Change Answers");
                broadcast_BT();
                __delay_ms(500);
                stat_1 = 1;

                Lcd_Clear();
                Lcd_Start();
                Lcd_Set_Cursor(1, 1);
                Lcd_Print_String("LED BLUETOOTH:");
            }
        }

        if (stat_1 == 1) {
                memset(receivedString, 0, sizeof(receivedString));

                BT_get_string();

                if(strstr(receivedString, "S") != NULL){
                     BT_load_string(receivedString);
                     broadcast_BT();
                     memset(receivedString, 0, sizeof(receivedString));
                     stat_a=1;
                     stat_1 = 2;
                }
                
                else if(strstr(receivedString, "R") != NULL){
                     BT_load_string(receivedString);
                     broadcast_BT();
                     memset(receivedString, 0, sizeof(receivedString));
                     stat_b=1;
                     stat_1 = 2;
                }
                
                else if(strstr(receivedString, "M") != NULL){
                    
                    BT_load_string("Enter R to Reset Password");
                    broadcast_BT();
                    BT_load_string("Enter S to Change Answers");
                    broadcast_BT();
                    BT_load_string("Enter M to Change Answers");
                    broadcast_BT();            
                    memset(receivedString, 0, sizeof(receivedString));
                    stat_1 = 1;
                }
                
        }else if(stat_1 ==2){
            while(stat_a==1) // Set Questions
            {   
                    BT_load_string("What is your Home town?");
                    broadcast_BT();
                    BT_get_string();
                    __delay_ms(500);
                    char qst1[10]; 
                    eeprom_read_string(0, qst1);
                    if((strstr(receivedString, qst1) != NULL)  ){                         
                            BT_load_string("Answers Matched!!");
                            broadcast_BT();
                            memset(receivedString, 0, sizeof(receivedString));
                            BT_load_string("Enter your New answer!");
                            broadcast_BT();
                            BT_get_string();
                            __delay_ms(200);
                            eeprom_write_string(0, receivedString);
                            BT_load_string("Done!!");
                            stat_1 =1;
                            stat_a=0;
                            memset(receivedString, 0, sizeof(receivedString));  
                            but_out=1;
                            __delay_ms(3000);
                            but_out=0;
                            
                    }
                    else{
                        BT_load_string("Invalid Answer!!!");
                        broadcast_BT();  
                    }
                memset(receivedString, 0, sizeof(receivedString));
                __delay_ms(100);
            }
            while(stat_b==1) // Unlock System
            {   
                BT_load_string("What is your Home town?");
                broadcast_BT();
                BT_get_string();
                __delay_ms(500);
                
                char qst1[10]; // Adjust the size based on your maximum password length
                eeprom_read_string(0, qst1);
                if((strstr(receivedString, qst1) != NULL)  ){
                        BT_load_string("Answers Matched!!");
                        broadcast_BT();
                        BT_load_string("System Unlocked!!!!");
                        broadcast_BT();
                        memset(receivedString, 0, sizeof(receivedString));
                        stat_1 =1;
                        stat_b=0;
                        but_out_2=1;
                        __delay_ms(3000);
                        but_out_2=0;
                        
                }
                else{
                    BT_load_string("Invalid Answer!!!");
                    broadcast_BT();
                    
                }
                memset(receivedString, 0, sizeof(receivedString));
                __delay_ms(100);
            }
}
}
}
