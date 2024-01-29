#define _XTAL_FREQ 16000000
#include <stdio.h>
#define TMR2PRESCALE 4
#include <xc.h>
#include <conio.h>
#include <string.h>
#include <stdint.h>


#define RS RD2
#define EN RD3
#define D4 RD4
#define D5 RD5
#define D6 RD6
#define D7 RD7

#define C1 PORTBbits.RB4
#define C2 PORTBbits.RB5
#define C3 PORTBbits.RB6
#define C4 PORTBbits.RB7
#define R1 PORTBbits.RB0
#define R2 PORTBbits.RB1
#define R3 PORTBbits.RB2
#define R4 PORTBbits.RB3


#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)
//LCD Functions Developed by Circuit Digest.



int blu_stat=0;

#define buzzer PORTEbits.RE0  // buzzer
long PWM_freq = 5000; //PWM frequency
#define forword_PIN PORTEbits.RE1 //motor PIN1
#define reverce_PIN PORTCbits.RC5 //motor PIN2

unsigned int dorStat=0;
int pass_stat=0;//pass_change_in_process
int n_pass_stat=0;//New_pass_set
unsigned int count=1; //attempt count
unsigned int inCount=0;
unsigned int lock=0;// system locked
unsigned int lock_2=0;//b is pressed
unsigned int lock_3=0;
unsigned int pass_stat_2=0;

/////External EEprom signal lines/////
#define EEp_OUT_1 PORTCbits.RC1
#define EEp_OUT_2 PORTEbits.RE2

//////physical Switches////////
#define lim1 PORTDbits.RD0
#define lim2 PORTDbits.RD1
#define inBut PORTCbits.RC3

////Bluetooth function////////
#define stat_fk PORTCbits.RC7 // blu_input_1
#define stat_fk_out PORTCbits.RC6 // blu_out_1
#define stat_fk_in_2  PORTCbits.RC0  // blu_in_2
 
/////////////////////////////////////////////////////////
__EEPROM_DATA('1', '2', '3', '4', '5', '6', 0x00, 0x00);

PWM_initialize(){
    PR2 =(_XTAL_FREQ/(PWM_freq*4*TMR2PRESCALE))-1; //Setting the PR2 using formula
    CCP1M3 = 1; CCP1M2 = 1; //Configure the CCP1 module as PWM
    T2CKPS0 = 1; T2CKPS1 = 0; //Configure the timer 2 prescaler as 4
    TMR2ON = 1; //Turn ON Timer 2
    TRISC2 = 0; //Make pin RC2/CCP1 as output
    
}
PWM_Duty(unsigned int duty){
    duty = ((float)duty/1023)*PR2*4;
    CCP1X = duty & 2;
    CCP1Y = duty & 1;
    CCPR1L = duty >> 2;
}
void ADC_initialize(){
    ADCON0 = 0b01000001  ; //ADC ON and Fosc/16 is selected
    ADCON1 = 0b11000000; // internal reference voltage is selected
    
}
unsigned int ADC_Read(unsigned char channel){
    ADCON0 &= 0x22000101;
    ADCON0 |= channel<<3;
    __delay_ms(2);
    GO_nDONE = 1;
    while(GO_nDONE);
    return((ADRESH<<8)+ADRESL);
}

///////////////////////////////////////////////
void eeprom_write_string(int address, char *data) {
    while (*data != '\0') {
        eeprom_write(address, *data);
        address++;
        data++;
        __delay_ms(100);
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
Lcd_Clear()
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

////////////////////////////////////////////////////////////////////
void motorControl(unsigned int val1, unsigned int val2){
    forword_PIN = val1;
    reverce_PIN = val2;

}
void lockingSequence(){
    if(lim1==0){
       __delay_ms(20);
        while(lim1==0){
            motorControl(1,0);
        }
    }
    motorControl(0,0);
    __delay_ms(250);
    Lcd_Clear();
    Lcd_Set_Cursor(1,5);
    Lcd_Print_String("LOCKED!!");
    __delay_ms(1000);
}

void unLockingSequence(){
    if(lim2==0){
       __delay_ms(20);
        while(lim2==0){
            motorControl(0,1);
        }
    }
    motorControl(0,0);
}

////////////////////////////////////////////////////////////////////
char enteredPassword[10]; // Assuming a password of up to 10 characters
char restPasswrord[10];
unsigned int minutes,seconds;

void gdSnd(){
    for(int j=0;j<4;j++){
            buzzer=1;//BUZZER ON
            __delay_ms(100);
            
            buzzer=0;//BUZZER OFF
            __delay_ms(100);
            }
}


void enterPassword(){
    Lcd_Clear();
    Lcd_Start();
    Lcd_Set_Cursor(1,1);
    Lcd_Print_String("Password Please:");
    Lcd_Set_Cursor(2,1);
    
}
void closeNote(){
    Lcd_Clear();
    Lcd_Start();
    Lcd_Set_Cursor(1,5);
    Lcd_Print_String("Please!!!");
    Lcd_Set_Cursor(2,2);
    Lcd_Print_String("Lock the Door");
}


void addToEnteredPassword(char c) {
    int len = strlen(enteredPassword);
    if (len < 9) {
        enteredPassword[len] = c;
        enteredPassword[len + 1] = '\0';
    }
}
void addToRestPassword(char c) {
    int len = strlen(restPasswrord);
    if (len < 9) {
        restPasswrord[len] = c;
        restPasswrord[len + 1] = '\0';
    }
}

void validPassword(){
    
    Lcd_Clear();
        Lcd_Set_Cursor(1, 6);
        Lcd_Print_String("Correct");
        Lcd_Set_Cursor(2, 4);
        Lcd_Print_String("Password!!!");
            gdSnd();
            unLockingSequence();
            dorStat=1;
            inCount=0;
        Lcd_Clear();
        Lcd_Set_Cursor(1, 5);
        Lcd_Print_String("Welcome!");
        __delay_ms(1000);
        closeNote();
        EEp_OUT_2=1;
        __delay_ms(2500);
        EEp_OUT_2=0;
}


void invalidPassword(){
   
    Lcd_Clear();
        for(int i=0;i<3;i++){
            Lcd_Set_Cursor(1, 1);
            Lcd_Print_String("Invalid Password");
            Lcd_Set_Cursor(2, 6);
            Lcd_Print_String("Sorry!!!");
            buzzer=1;//BUZZER ON
            __delay_ms(700);
            Lcd_Clear();
            buzzer=0;//BUZZER OFF
            __delay_ms(700);
            
        }
}
void warning(){
    Lcd_Clear();
    Lcd_Start();
    
    for(int i=0;i<2;i++){
        Lcd_Set_Cursor(1,1);
        Lcd_Print_String("System Locked!!");
        buzzer=1;//BUZZER ON
        __delay_ms(1000);
        Lcd_Clear();
        buzzer=0;//BUZZER OFF
        __delay_ms(1000);
    }
    Lcd_Set_Cursor(1,3);
    Lcd_Print_String("Press (B)");
    Lcd_Set_Cursor(2,2);
    Lcd_Print_String("To Reset Key!!");
    __delay_ms(100);
    lock=1;
    __delay_ms(100);
}
void errordelay(unsigned int val){
        char sec[6];
        Lcd_Clear();
        sprintf(sec, "%d", val);
        Lcd_Set_Cursor(1, 2);
        Lcd_Print_String("Wait ");
        Lcd_Set_Cursor(1, 7);
        Lcd_Print_String("for ");
        Lcd_Set_Cursor(1, 11);
        Lcd_Print_String(sec);
        Lcd_Set_Cursor(1, 13);
        Lcd_Print_String("s!");
        
        Lcd_Set_Cursor(2,7);
        for (int i = 0; i < val; i++){
            minutes = i / 60;
            seconds = i % 60;
            char timeString[6]; // Format: MM:SS
            sprintf(timeString, "%02d:%02d", minutes, seconds);
            Lcd_Print_String(timeString);
            __delay_ms(1000);
            Lcd_Set_Cursor(2,7);
        } 
}
void forthAttempt(){
    char password[10]; // Adjust the size based on your maximum password length
    eeprom_read_string(0, password);
    if (strlen(enteredPassword)==strlen(password)) {
            if(strcmp(enteredPassword,password)==0){
                validPassword();
                count=1;
                memset(enteredPassword, 0, sizeof(enteredPassword));
            }
            else {
                invalidPassword();
                count++;
                memset(enteredPassword, 0, sizeof(enteredPassword));
                if(count<4){
                    enterPassword();
                }
                else{
                    errordelay(60);
                    enterPassword();
                }
            }
        } 
}

void fifthAttempt(){
    char password[10]; // Adjust the size based on your maximum password length
    eeprom_read_string(0, password);
     
    if (strlen(enteredPassword)==strlen(password)) {
            if(strcmp(enteredPassword,password)==0){
                validPassword();
                count=1;
                memset(enteredPassword, 0, sizeof(enteredPassword));
            }
            else {
                invalidPassword();
                count++;
                memset(enteredPassword, 0, sizeof(enteredPassword));
                warning();
            }
        } 
}

void first3Attempts() {
    char password[10]; // Adjust the size based on your maximum password length
    eeprom_read_string(0, password);
        if (strlen(enteredPassword)==strlen(password)) {
            if(strcmp(enteredPassword,password)==0){
                memset(enteredPassword, 0, sizeof(enteredPassword));
                validPassword();
                count=1;
                
            }
            else {
                invalidPassword();
                count++;
                memset(enteredPassword, 0, sizeof(enteredPassword));
                if(count<4){
                    enterPassword();
                }
                else{
                    errordelay(30);
                    enterPassword();
                }
            }
        }    
}
void pass_chng(){
    Lcd_Clear();
    Lcd_Start();
    Lcd_Set_Cursor(1,3);
    Lcd_Print_String("Please Enter");
    Lcd_Set_Cursor(2,2);
    Lcd_Print_String("Current Pass!!");
    __delay_ms(1500);
    Lcd_Clear();
    Lcd_Set_Cursor(1,1);
    Lcd_Print_String("Current Pass:");
    Lcd_Set_Cursor(2,1);
}
////////////////////////////////////////////////////////////////////////////////

void keypad(){
    char key = '\0';
    
    C1 =1; C2=0; C3=0; C4=0;
    if(R1==1){
        if(lock==0){
        key='1';
        Lcd_Print_Char('1');
        }
        while(R1==1);
    }
    if(R2==1){
        if(lock==0){
        key='4';
        Lcd_Print_Char('4');
        }
        while(R2==1);
        
    }
    if(R3==1){
        if(lock==0){
        key='7';
        Lcd_Print_Char('7');
        }
        while(R3==1);
    }
    if(R4==1){//"*"
        if(lock==0){
            if(dorStat==1){ 
                gdSnd();
                lockingSequence();
                enterPassword();
                dorStat=0;
                inCount=0;
            }
        }    
        while(R4==1);
    }
    
    C1 =0; C2=1; C3=0; C4=0;
    if(R1==1){
        if(lock==0){
        key='2';
        Lcd_Print_Char('2');
        }
        while(R1==1);
    }
    if(R2==1){
        if(lock==0){
        key='5';
        Lcd_Print_Char('5');
        }
        while(R2==1);
    }
    if(R3==1){
        if(lock==0){
        key='8';
        Lcd_Print_Char('8');
        }
        while(R3==1);
    }
    if(R4==1){
        if(lock==0){
        key='0';
        Lcd_Print_Char('0');
        }
        while(R4==1);
    }
    
    C1 =0; C2=0; C3=1; C4=0;
    if(R1==1){
        if(lock==0){
        key='3';
        Lcd_Print_Char('3');
        }
        while(R1==1);
    }
    if(R2==1){
        if(lock==0){
        key='6';
        Lcd_Print_Char('6');
        }
        while(R2==1);
    }
    if(R3==1){
        if(lock==0){
        key='9';
        Lcd_Print_Char('9');
        }
        while(R3==1);
    }
    if(R4==1){//"#"
        if (dorStat==0 && n_pass_stat==0 && pass_stat==0 && lock==0){
            memset(enteredPassword, 0, sizeof(enteredPassword));
            enterPassword();
        }
        else if(pass_stat==1 && n_pass_stat==0){
            memset(enteredPassword, 0, sizeof(enteredPassword));
            Lcd_Clear();
            Lcd_Set_Cursor(1,1);
            Lcd_Print_String("Current Pass:");
            Lcd_Set_Cursor(2,1);
        }
        else if(pass_stat==1 && n_pass_stat==1){
            
            memset(restPasswrord, 0, sizeof(restPasswrord));
            Lcd_Clear();
                Lcd_Set_Cursor(1,1);
                Lcd_Print_String("New Password:");
                Lcd_Set_Cursor(2,1);
        }
        else if(pass_stat==2){
            
            memset(restPasswrord, 0, sizeof(restPasswrord));
            Lcd_Clear();
                Lcd_Set_Cursor(1,1);
                Lcd_Print_String("New Password:");
                Lcd_Set_Cursor(2,1);
        }
        while(R4==1);
    }
    C1 =0; C2=0; C3=0; C4=1;
    if(R1==1){//"A"
        if(lock==0){
            memset(enteredPassword, 0, sizeof(enteredPassword));
            memset(restPasswrord, 0, sizeof(restPasswrord));
            pass_chng();
            pass_stat=1;
        }
        while(R1==1);
    }
    if (R2 == 1) { //"B"
     if (lock == 1) {
         Lcd_Clear();
         Lcd_Set_Cursor(1, 2);
         Lcd_Print_String("Connect Phone");
         Lcd_Set_Cursor(2, 3);
         Lcd_Print_String("& Press (C)!!");
         __delay_ms(100);
         lock_2=1;
     }
    while (R2 == 1);
}
    if(R3==1){//C
        if(lock==0 && lock_3==0){
            Lcd_Clear();
            Lcd_Set_Cursor(1, 2);
            Lcd_Print_String("Connect Phone");
            Lcd_Set_Cursor(2, 3);
            Lcd_Print_String("& Press (D)!!");
            __delay_ms(100);
            lock_3=1;
        }
        else if(lock==1 && lock_2==1){
            Lcd_Clear();
            Lcd_Set_Cursor(1, 2);
            Lcd_Print_String("Connect Phone");
            blu_stat=1;
            __delay_ms(500);
            Lcd_Clear();
            Lcd_Set_Cursor(1, 1);
            Lcd_Print_String("Please Complete");
            Lcd_Set_Cursor(2, 3);
            Lcd_Print_String("The Process!!!");
            stat_fk_out=1;
            __delay_ms(3000);
            stat_fk_out=0;
            
        }
        while(R3==1);
    }
    if(R4==1){//D
        if(lock_3==1){
            Lcd_Clear();
            Lcd_Set_Cursor(1, 2);
            Lcd_Print_String("Connect Phone");
            lock=2;
            blu_stat=2;
            __delay_ms(500);
            Lcd_Clear();
            Lcd_Set_Cursor(1, 1);
            Lcd_Print_String("Please Complete");
            Lcd_Set_Cursor(2, 3);
            Lcd_Print_String("The Process!!!");
            stat_fk_out=1;
            __delay_ms(3000);
            stat_fk_out=0;
            
        }
        while(R4==1);
    }
    if(stat_fk==1){
        if(lock==1 && lock_2==1 && blu_stat==1){
            Lcd_Clear();
            Lcd_Set_Cursor(1, 4);
            Lcd_Print_String("System!!!");
            Lcd_Set_Cursor(2, 5);
            Lcd_Print_String("Unlocked");
            lock=0;
            lock_2=0;
            blu_stat=0;
            count=1;
            __delay_ms(1000);
                Lcd_Clear();
                Lcd_Start();
                Lcd_Set_Cursor(1,3);
                Lcd_Print_String("Please Enter!!");
                Lcd_Set_Cursor(2,1);
                Lcd_Print_String("The new Password");
                __delay_ms(1500);
                Lcd_Clear();
                Lcd_Set_Cursor(1,1);
                Lcd_Print_String("New Password:");
                Lcd_Set_Cursor(2,1);
                pass_stat=2;
                
        }
        
        while(stat_fk==1);
    }
    
    if(stat_fk_in_2==1){
       if(lock==2 && lock_3==1 && blu_stat==2){
            Lcd_Clear();
            Lcd_Set_Cursor(1, 1);
            Lcd_Print_String("Answer Changed");
            Lcd_Set_Cursor(2,1);
            Lcd_Print_String("Successfully!!");
            __delay_ms(2500);   
            lock_3=0;
            lock=0;
            blu_stat=0;
            if(dorStat==0){
                 enterPassword();
            }
            else{
                 closeNote();
            }
       }
        
        while(stat_fk_in_2==1);
    }
    if (dorStat==0 && pass_stat==0){
        if (key != '\0') {

            addToEnteredPassword(key);
            __delay_ms(100); 
            if(count==1||count==2||count==3){
                first3Attempts(); // Check the password after each key press
            }
            else if(count==4){
                forthAttempt(); // Check the password after each key press
            }
            else if(count==5){
                fifthAttempt(); // Check the password after each key press
            }
        
        }
    }
    else if(pass_stat==1){
        if (key != '\0') {

            addToEnteredPassword(key);
            __delay_ms(100);
            char password[10]; // Adjust the size based on your maximum password length
            eeprom_read_string(0, password);
            if (strlen(enteredPassword)==strlen(password) && n_pass_stat==0 ) {
                if(strcmp(enteredPassword,password)==0){
                memset(enteredPassword, 0, sizeof(enteredPassword));
                Lcd_Clear();
                Lcd_Start();
                Lcd_Set_Cursor(1,3);
                Lcd_Print_String("Please Enter!!");
                Lcd_Set_Cursor(2,1);
                Lcd_Print_String("The new Password");
                __delay_ms(1500);
                Lcd_Clear();
                Lcd_Set_Cursor(1,1);
                Lcd_Print_String("New Password:");
                Lcd_Set_Cursor(2,1);
                n_pass_stat=1;
                }
                else {
                invalidPassword();
                memset(enteredPassword, 0, sizeof(enteredPassword));
                    if(dorStat==0){
                        enterPassword();
                    }
                    else{
                        closeNote();
                    }
                    pass_stat=0;
                }
            }
            else if(n_pass_stat==1){
                if (key != '\0') {

                addToRestPassword(key);
                if(strlen(restPasswrord)==6){
                    eeprom_write_string(0, restPasswrord);
                    memset(restPasswrord, 0, sizeof(restPasswrord));
                    memset(enteredPassword, 0, sizeof(enteredPassword));
                    n_pass_stat=0;
                    pass_stat=0;
                    Lcd_Clear();
                    Lcd_Start();
                    Lcd_Set_Cursor(1,3);
                    Lcd_Print_String("Please Wait!!");
                    __delay_ms(5000);
                    Lcd_Clear();
                    Lcd_Set_Cursor(1,3);
                    Lcd_Print_String("New Password");
                    Lcd_Set_Cursor(2,4);
                    Lcd_Print_String("Is Set!!");
                    __delay_ms(3000);
                    gdSnd();
                    if(dorStat==0){
                        enterPassword();
                    }
                    else{
                        closeNote();
                    }
                }
                
                }
            }
        }
    }
    else if(pass_stat==2){
        if (key != '\0') {

                addToRestPassword(key);
                if(strlen(restPasswrord)==6){
                    eeprom_write_string(0, restPasswrord);
                    memset(restPasswrord, 0, sizeof(restPasswrord));
                    memset(enteredPassword, 0, sizeof(enteredPassword));
                    n_pass_stat=0;
                    pass_stat=0;
                    Lcd_Clear();
                    Lcd_Start();
                    Lcd_Set_Cursor(1,3);
                    Lcd_Print_String("Please Wait!!");
                    __delay_ms(5000);
                    Lcd_Clear();
                    Lcd_Set_Cursor(1,3);
                    Lcd_Print_String("New Password");
                    Lcd_Set_Cursor(2,4);
                    Lcd_Print_String("Is Set!!");
                    __delay_ms(3000);
                    gdSnd();
                    if(dorStat==0){
                        enterPassword();
                    }
                    else{
                        closeNote();
                    }
                }
                
                }            
                    
    }
    
}


void main()
{  
    
    blu_stat=0;
    lock_3=0;
    lock=0;
    int adc_value;
//    Initialize_Bluetooth();
     
    TRISCbits.TRISC1=0;// PWM port
    TRISAbits.TRISA0 = 1; //adc port
    ADC_initialize();
    PWM_initialize();
    
    TRISCbits.TRISC3=1;
    TRISDbits.TRISD1=1;//lim Sw 1
    TRISDbits.TRISD0=1;//lim Sw 2
    

    TRISDbits.TRISD7=0;
    TRISDbits.TRISD6=0;
    TRISDbits.TRISD5=0;
    TRISDbits.TRISD4=0;
    TRISDbits.TRISD3=0;
    TRISDbits.TRISD2=0;
    
    
    //Keypad bits
    TRISBbits.TRISB7=0;
    TRISBbits.TRISB6=0;
    TRISBbits.TRISB5=0;
    TRISBbits.TRISB4=0;
    TRISBbits.TRISB3=1;
    TRISBbits.TRISB2=1;
    TRISBbits.TRISB1=1;
    TRISBbits.TRISB0=1;

    TRISEbits.TRISE0=0; //Buzzer pin
    TRISEbits.TRISE1=0; //forword_pin
    TRISCbits.TRISC5=0; //reverce_pin
    
    TRISCbits.TRISC7=1; //blu_input_1
    TRISCbits.TRISC6=0; //blu_out_1
    TRISCbits.TRISC0=1; //blu_in_2
    
    TRISCbits.TRISC1=0; //EEp_OUT_1
    TRISEbits.TRISE2=0; //EEp_OUT_2
    
    if(dorStat==0){
        enterPassword();
    }else{closeNote();}
    
    
    motorControl(0,0);
    
    __delay_ms(50);
     adc_value = ADC_Read(0);
        PWM_Duty(adc_value);
        __delay_ms(50);
    if(lim1==0){
       __delay_ms(20);
        while(lim1==0){
            motorControl(1,0);
        }
    }
    motorControl(0,0);
    
    while(1)
    {
        
        if(inBut==0 ){
            __delay_ms(50);
            while(inBut==0){
                keypad();
                __delay_ms(100);
            } 
        }
        else if(inCount==0 && dorStat==0 ){
            Lcd_Clear();
            Lcd_Set_Cursor(1, 5);
            Lcd_Print_String("Welcome!");
            __delay_ms(200);
            gdSnd();
            unLockingSequence();
            inCount=1;
            dorStat=1;
            count=1;
            lock=0;
            lock_2=0;
            blu_stat=0;
            lock_3=0;
            EEp_OUT_1=1;
            __delay_ms(2500);
            EEp_OUT_1=0;
            Lcd_Clear();
            Lcd_Set_Cursor(1, 5);
            Lcd_Print_String("Welcome!");
            __delay_ms(800);
            closeNote();
        }
        else if(dorStat==1 ){
            gdSnd();
            lockingSequence();
            inCount=0;
            enterPassword();
            dorStat=0;
            count=1;
            blu_stat=0;
        }

        
        adc_value = ADC_Read(0);
           PWM_Duty(adc_value);
           __delay_ms(50);
    }
}

