#include <stdio.h>
#include<xc.h>
#include <conio.h>
#include <string.h>
#include <stdint.h>


#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF     


#define MAX_STRING_SIZE 10 // Adjust the size based on your requirements

#define but_prss PORTBbits.RB7
#define but_out PORTBbits.RB6
#define but_out_2 PORTBbits.RB5
__EEPROM_DATA('1', '2', '3', '4', '5', '6', 0x00, 0x00);
char receivedString[MAX_STRING_SIZE];

#define _XTAL_FREQ 16000000

unsigned char a;
unsigned char byte;
int block_count = 0;
int address_count = 0;
unsigned long msb = 0x000000;
unsigned long lsb = 0x000001;

unsigned int stat_1=0;
unsigned int stat_a=0;
unsigned int stat_b=0;
unsigned int stat_c=0;
unsigned int stat_d=0;
unsigned int stat_a_1 =0;


#define RS RD2
#define EN RD3
#define D4 RD4
#define D5 RD5
#define D6 RD6
#define D7 RD7



void spi_write(unsigned long value);
unsigned char spi_read();
void write_to_external_eeprom(unsigned long* msb, unsigned long* lsb, unsigned char data, int* block_count, int* address_count);
void read_from_external_eeprom(unsigned long* msb, unsigned long* lsb, unsigned char written_data);
void ReadByteFromEEPROM(const unsigned char address);
void WriteByteToEEPROM(unsigned char data, const unsigned char address);




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


void Lcd_Clear()

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

  for(long int i=1065244; i<=0; i--)  NOP();  

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

   RS = 1;             

   Lcd_SetBit(Upper_Nibble>>4);             //Send upper half by shifting by 4

   EN = 1;

   for(long int i=2130483; i<=0; i--)  NOP(); 

   EN = 0;

   Lcd_SetBit(Lower_Nibble); //Send Lower half

   EN = 1;

   for(long int i=2130483; i<=0; i--)  NOP();

   EN = 0;

}


void Lcd_Print_String(char *a)

{

    int i;

    for(i=0;a[i]!='\0';i++)

       Lcd_Print_Char(a[i]);  //Split the string using pointers and call the Char function 

}

void spi_write(unsigned long value){


      SSPBUF = (unsigned char)value;

    while (!SSPSTATbits.BF);

}

unsigned char spi_read(){

    SSPBUF = 0x00;
    while (!PIR1bits.SSPIF);
    return (SSPBUF);

    

}


void write_to_external_eeprom(unsigned long* msb, unsigned long* lsb, unsigned char data, int* block_count, int* address_count){

        
    if(*block_count < 64 && *address_count < 255){

    PORTBbits.RB0 = 0;

    spi_write(0x02);
                
    SSPBUF = (unsigned char)(*msb >> 16);  // Send the high byte of the address
    while (!SSPSTATbits.BF);  // Wait for buffer to be full

    SSPBUF = (unsigned char)(*msb >> 8);   // Send the middle byte of the address
    while (!SSPSTATbits.BF);  // Wait for buffer to be full

    SSPBUF = (unsigned char)*msb;          // Send the low byte of the address
    while (!SSPSTATbits.BF);  // Wait for buffer to be full                
                
                
    SSPBUF = (unsigned char)(*lsb >> 16);  // Send the high byte of the address
    while (!SSPSTATbits.BF);  // Wait for buffer to be full

    SSPBUF = (unsigned char)(*lsb >> 8);   // Send the middle byte of the address
    while (!SSPSTATbits.BF);  // Wait for buffer to be full

    SSPBUF = (unsigned char)*lsb;          // Send the low byte of the address
    while (!SSPSTATbits.BF);  // Wait for buffer to be full                
                    
    
    spi_write(data);

    PORTBbits.RB0 = 1;

    *msb += 1;
    *lsb += 1;
    
    *address_count += 1; // comment or not

    }

    else if (*block_count < 64 && *address_count == 255){


        unsigned long last_two_digits;
        unsigned long first_two_digits;
        last_two_digits = *msb & 0xFF;

        first_two_digits = (*msb >> 16) & 0xFF;
        first_two_digits += 1;
        first_two_digits = first_two_digits << 16;


        *msb = first_two_digits;
        *address_count += 1;     
        *block_count += 1;

        
    PORTBbits.RB0 = 0;

    spi_write(0x02);
                
    SSPBUF = (unsigned char)(*msb >> 16);  // Send the high byte of the address
    while (!SSPSTATbits.BF);  // Wait for buffer to be full

    SSPBUF = (unsigned char)(*msb >> 8);   // Send the middle byte of the address
    while (!SSPSTATbits.BF);  // Wait for buffer to be full

    SSPBUF = (unsigned char)*msb;          // Send the low byte of the address
    while (!SSPSTATbits.BF);  // Wait for buffer to be full                         
      
    
    SSPBUF = (unsigned char)(*lsb >> 16);  // Send the high byte of the address
    while (!SSPSTATbits.BF);  // Wait for buffer to be full

    SSPBUF = (unsigned char)(*lsb >> 8);   // Send the middle byte of the address
    while (!SSPSTATbits.BF);  // Wait for buffer to be full

    SSPBUF = (unsigned char)*lsb;          // Send the low byte of the address
    while (!SSPSTATbits.BF)
        ;  // Wait for buffer to be full                
                    
    
 
    spi_write(data);

    PORTBbits.RB0 = 1;
             


    }


    if (*block_count == 64){
    }


    Lcd_Clear();
    Lcd_Start();
    
    Lcd_Set_Cursor(1,3);

    Lcd_Print_String("Reading from");
    
    }


void display(unsigned long reading_address, unsigned long ending_address, int unlock_type){

    
        char string_1[20]; 
        char string_2[20];
        sprintf(string_1, "%lx", reading_address);
        sprintf(string_2, "%lx", ending_address);


    Lcd_Clear();
    Lcd_Start();
    
    
    if (unlock_type == 1){
    

        __delay_ms(500);
        
        Lcd_Set_Cursor(1,3);

        Lcd_Print_String("Reading from");

        Lcd_Set_Cursor(2,6);
        
        Lcd_Print_String(string_1);

        Lcd_Set_Cursor(2,8);
        
        Lcd_Print_String("to");
        
        Lcd_Set_Cursor(2,11);
        
        Lcd_Print_String(string_2);

        __delay_ms(2000);

        Lcd_Clear();
    
        Lcd_Set_Cursor(2,3);

        Lcd_Print_String("PIN Unlocked!");
          
        __delay_ms(3000);

          
    }

    
    else if (unlock_type == 2){

        __delay_ms(500);
        
        Lcd_Set_Cursor(1,3);

        Lcd_Print_String("Reading from");

        Lcd_Set_Cursor(2,6);
        
        Lcd_Print_String(string_1);
        
        Lcd_Set_Cursor(2,8);
        
        Lcd_Print_String("to");
        
        Lcd_Set_Cursor(2,11);
        
        Lcd_Print_String(string_2);
        
        __delay_ms(2000);
        
        Lcd_Clear();

        Lcd_Set_Cursor(1,3);

        Lcd_Print_String("Opened From");
          
        Lcd_Set_Cursor(2,6);

        Lcd_Print_String("Inside!");
          


    }
    

}

void read_from_external_eeprom(unsigned long* msb, unsigned long* lsb, unsigned char written_data){
    
    
    PORTBbits.RB0 = 0;

    spi_write(0x03);
                     
    SSPBUF = (unsigned char)((*msb-1) >> 16);  // Send the high byte of the address
    while (!SSPSTATbits.BF);  // Wait for buffer to be full

    SSPBUF = (unsigned char)((*msb-1) >> 8);   // Send the middle byte of the address
    while (!SSPSTATbits.BF);  // Wait for buffer to be full

    SSPBUF = (unsigned char)(*msb-1);          // Send the low byte of the address
    while (!SSPSTATbits.BF);  // Wait for buffer to be full                
                
                
      
    
    SSPBUF = (unsigned char)((*lsb-1) >> 16);  // Send the high byte of the address
    while (!SSPSTATbits.BF);  // Wait for buffer to be full

    SSPBUF = (unsigned char)((*lsb-1) >> 8);   // Send the middle byte of the address
    while (!SSPSTATbits.BF);  // Wait for buffer to be full

    SSPBUF = (unsigned char)(*lsb-1);          // Send the low byte of the address
    while (!SSPSTATbits.BF)
        ;                
                
                      
    a = (spi_read());

    PORTBbits.RB0 = 1;

    
    if (written_data == 0x01){

        RA0 = 0;
        RA1 = 1;

        display(*msb-1,*lsb, 1);


    }
    
    else if (written_data == 0x02){



        RA1 = 0;
        RA0 = 1;
        
        display(*msb-1,*lsb, 1);



        
    }
    
}








//
//void read_from_external_eeprom(unsigned long* msb, unsigned long* lsb, unsigned char written_data){
//
//    PORTBbits.RB0 = 0;
//
//    spi_write(0x03);
//                     
//    SSPBUF = (unsigned char)((*msb-1) >> 16);  // Send the high byte of the address
//    while (!SSPSTATbits.BF);  // Wait for buffer to be full
//
//    SSPBUF = (unsigned char)((*msb-1) >> 8);   // Send the middle byte of the address
//    while (!SSPSTATbits.BF);  // Wait for buffer to be full
//
//    SSPBUF = (unsigned char)(*msb-1);          // Send the low byte of the address
//    while (!SSPSTATbits.BF);  // Wait for buffer to be full                
//                
//                
//      
//    
//    SSPBUF = (unsigned char)((*lsb-1) >> 16);  // Send the high byte of the address
//    while (!SSPSTATbits.BF);  // Wait for buffer to be full
//
//    SSPBUF = (unsigned char)((*lsb-1) >> 8);   // Send the middle byte of the address
//    while (!SSPSTATbits.BF);  // Wait for buffer to be full
//
//    SSPBUF = (unsigned char)(*lsb-1);          // Send the low byte of the address
//    while (!SSPSTATbits.BF)
//        ;                
//                
//                      
//    a = (spi_read());
//
//    PORTBbits.RB0 = 1;
//
//    
//    if (written_data == 0x01){
//
//        RA0 = 0;
//        RA1 = 1;
//        
//        display(*msb-1,*lsb-1, 1);
//    }
//    
//    else if (written_data == 0x02){
//
//
//
//        RA1 = 0;
//        RA0 = 1;
//        display(*msb-1, *lsb-1, 2);
//
//        
//    }
//    
//}
//


#define RS RD2
#define EN RD3
#define D4 RD4
#define D5 RD5
#define D6 RD6
#define D7 RD7

       // Flash Program Memory Code Protection bit (Code protection off)

//End of CONFIG registers



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
//Initialize Bluetooth using USART//


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
//BT initialized//
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


void main(){

    TRISCbits.TRISC3 = 0;
    TRISCbits.TRISC4 = 1;
    TRISCbits.TRISC5 = 0;
    TRISBbits.TRISB0 = 0;
    
    but_out=0;
    but_out_2=0;


    Initialize_Bluetooth();
    TRISBbits.TRISB7 = 1;
    TRISBbits.TRISB6 = 0;
    TRISBbits.TRISB5 = 0;
    TRISD = 0x00;    
    


    TRISA0 = 0;
    TRISA1 = 0; 
    TRISB1 = 1;   
    TRISB2 = 1; 


    SSPCON = 0x20;
    SSPSTAT = 0x00;

    Lcd_Clear();
    Lcd_Start();

    char string_value[10];
    eeprom_read_string(0x00, string_value);

    if ((int)string_value == 0){

        unsigned long msb = 0x000000;
        unsigned long lsb = 0x000001;
    }
    
    else{
    
        unsigned long msb = (unsigned int)(string_value);
        unsigned long lsb = (unsigned int)(string_value)+1;
    
    }
    while(1){


    if (RB1 == 1){
        
        __delay_ms(2000);

        PORTBbits.RB0 = 0;
        spi_write(0x06);
        PORTBbits.RB0 = 1;

        __delay_ms(10);

        write_to_external_eeprom(&msb, &lsb, 0x01, &block_count, &address_count);
 
        __delay_ms(10);

        PORTBbits.RB0 = 0;
        spi_write(0x04);
        PORTBbits.RB0 = 1;
       
        __delay_ms(10);   

        read_from_external_eeprom(&msb, &lsb, 0x01);
        __delay_ms(10);   



    }
    
    if (RB2 == 1){
        
        __delay_ms(2000);

        PORTBbits.RB0 = 0;
        spi_write(0x06);
        PORTBbits.RB0 = 1;

        __delay_ms(10);

        write_to_external_eeprom(&msb, &lsb, 0x02, &block_count, &address_count);
        
        __delay_ms(10);

        PORTBbits.RB0 = 0;
        spi_write(0x04);
        PORTBbits.RB0 = 1;
       
        __delay_ms(1000);
        
        read_from_external_eeprom(&msb, &lsb, 0x02);
        __delay_ms(10);   

     
    }
    


if (but_prss == 1 && stat_1 == 0) {
            __delay_ms(50);
            // if (but_prss == 1 && stat_1 == 0) {
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
                
        }


if(stat_1 ==2){

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
                            stat_1 =0;
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
                        stat_1 =0;
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