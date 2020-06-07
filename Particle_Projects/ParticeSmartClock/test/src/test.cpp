/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "c:/Users/rahul/Desktop/Particle/test/src/test.ino"
/*
* Project test: 
* Description:
* Author:
* Date:
*/

void setup();
void loop();
#line 8 "c:/Users/rahul/Desktop/Particle/test/src/test.ino"
#define ACTIVE						        0x01
#define INTENSITY					        0x00
#define OFF_TM1638                0x00
#define GREEN_TM1638 				      0x01
#define RED_TM1638 					      0x02
#define YELLOW_TM1638             0x03
#define DATA_WRITE_INCR_ADDR 		  0x40
#define DATA_WRITE_FIX_ADDR 		  0x44
#define DATA_READ_KEY_SCAN_MODE 	0x42
#define ADDRSET 					        0xC0
#define DISP_ON 					        0x8F
#define DISP_OFF 					        0x80
#define AM_PM_FORMAT              0x00
#define HR_FORMAT                 0x01
#define HYPHEN                    0x40
#define EEPROM_ADDR               0x00

#define MY_SCL                    D0
#define MY_SDA                    D1
#define LED                       D7

const uint8_t Num[]= 
{	//Code table of numbers
	0x3F,					//0
	0x06,					//1
	0x5B,					//2
	0x4F,					//3
	0x66,					//4
	0x6D,					//5
	0x7D,					//6
	0x07,					//7
	0x7F,					//8
	0x6F,					//9
	0x77,					//A
	0x7C,					//B
	0x39,					//C
	0x5E,					//D
	0x79,					//E
	0x71					//F
};

const uint8_t ERROR_DATA[] = 
{
	0x79, 		// E
	0x50, 		// r
	0x50, 		// r
	0x5C, 		// o
	0x50, 		// r
	0,
	0,
	0
};

uint8_t MY_SSEL = D2; //for some reason MY_SSEL has to be an int

uint8_t time_hour, time_min, time_sec, temp_number = 0, counter = 0;
bool flag = FALSE, WiFiConnected = FALSE, flagProcess = FALSE, oldWiFiStatus = FALSE, ledFlag = FALSE;

struct values //these must be of int type if we want to use Particle.variable function
{
    int intensity; 
    int timeFormat;
}backupVariables;

void SPI_SendData(uint8_t dat);
void TM1638_Init(void);
void TM1638_DisplayClean(void);
void TM1638_SetupDisplay(uint8_t active, uint8_t intensity);
void TM1638_SendCommand(uint8_t cmd);
void TM1638_SendData(uint8_t address, uint8_t dat);
void TM1638_ShowDig(uint8_t position, uint8_t data, uint8_t Dot);
void TM1638_ClearDig(uint8_t position, uint8_t dot);
void TM1638_ShowError(void);
void TM1638_ShowDecNumber(unsigned long number, uint8_t dots, uint8_t startingPos);
void TM1638_ShowSignedDecNumber(signed long number, uint8_t dots);
void TM1638_ShowHexNumber(unsigned long number, uint8_t dots);
void TM1638_ShowLed(uint8_t number, uint8_t color);
void TM1638_ShowLeds(uint8_t color);
void TM1638_LedBinaryCounter(uint8_t number, uint8_t color);
void Get_Time(uint8_t time_format);
void Set_TimeZone_India(void);
void Set_TimeZone_Global(uint8_t value);
void Show_Time(void);
void Update_Time(void);
void do_every_second(void);
void process(void);
int watchMode(String cmd);
int displayCtrl(String cmd);
uint8_t TM1638_GetKey(void);

Timer timer(1000, do_every_second);
Timer timerProcess(100, process);

void setup() 
{
    Particle.variable("Hour",  time_hour);
    Particle.variable("Minute", time_min);
    Particle.variable("Second", time_sec);
    Particle.variable("Intensity", backupVariables.intensity);
    Particle.variable("Time Format", backupVariables.timeFormat);
    Particle.variable("Time Mismatch", temp_number);
    Particle.function("Watch Mode", watchMode);
    Particle.function("Display Intensity", displayCtrl);

    timer.start();
    timerProcess.start();
    
    pinMode(MY_SCL,  OUTPUT);
    pinMode(MY_SDA,  OUTPUT);
    pinMode(MY_SSEL, OUTPUT);
    pinMode(LED, OUTPUT);
    
    EEPROM.get(EEPROM_ADDR, backupVariables);
    
    if(backupVariables.timeFormat > 1)
    {
        backupVariables.intensity = 0;
        backupVariables.timeFormat = 0;
        EEPROM.put(EEPROM_ADDR, backupVariables);
    }
    
    TM1638_Init();
	TM1638_SetupDisplay(ACTIVE, backupVariables.intensity);
	TM1638_SendData((1 << 1), HYPHEN);
	TM1638_SendData((2 << 1), HYPHEN);
	TM1638_SendData((3 << 1), HYPHEN);
	TM1638_SendData((4 << 1), HYPHEN);
	TM1638_SendData((5 << 1), HYPHEN);
	TM1638_SendData((6 << 1), HYPHEN);
	TM1638_SendData((7 << 1), HYPHEN);
	TM1638_SendData((8 << 1), HYPHEN);
    Set_TimeZone_India();
	Get_Time(backupVariables.timeFormat);
	Show_Time();  
}

void loop() 
{   
    if(flag == TRUE)
    {
        flag = FALSE;
        Update_Time(); // new addition
    }
}

void SPI_SendData(uint8_t dat)
{
	uint8_t index;
	
	for(index = 0; index < 8; index++)
	{
		digitalWrite(SDA, (dat & 0x01));
		dat >>= 1;
		delayMicroseconds(1);
		pinResetFast(SCL);
		delayMicroseconds(1);
		pinSetFast(SCL);
	}
	
	pinResetFast(SDA);
}

void TM1638_Init(void)
{
	TM1638_DisplayClean();							//Clean display
	TM1638_SendCommand(DISP_OFF);					//Display off
	TM1638_SendCommand(DATA_WRITE_FIX_ADDR);		//Set address mode
	TM1638_SendCommand(ADDRSET);
	TM1638_SetupDisplay(ACTIVE, INTENSITY);
}

void TM1638_DisplayClean(void) 
{						
	uint8_t index;
	
	TM1638_SendCommand(DATA_WRITE_INCR_ADDR);		//Set address mode
	
	for (index = 0; index < 16; index++) 
	{
		TM1638_SendData(index, 0x00);
	}
}

void TM1638_SetupDisplay(uint8_t active, uint8_t intensity)
{
    TM1638_SendCommand (0x80 | (active ? 8 : 0) | intensity);
}

void TM1638_SendCommand(uint8_t cmd) 
{	
	pinResetFast(MY_SSEL);  //Set STROBE = "0"
	SPI_SendData(cmd);
	pinSetFast(MY_SSEL);    //Set STROBE = "1"
}

void TM1638_SendData(uint8_t address, uint8_t dat)
{   
	TM1638_SendCommand(DATA_WRITE_FIX_ADDR);
	pinResetFast(MY_SSEL);          //Set STROBE = "0"
	SPI_SendData(0xC0 | address);	//Set first address
	SPI_SendData(dat);				//Send data
	pinSetFast(MY_SSEL);            //Set STROBE = "1"
}

void TM1638_ShowDig(uint8_t position, uint8_t data, uint8_t Dot)			
{
	TM1638_SendData(position << 1, Num [data] | (Dot ? 0x80 : 0));
}

void TM1638_ClearDig(uint8_t position, uint8_t dot)
{
	TM1638_SendData(position << 1, 0x00 | (dot ? 0x80 : 0));
}

void TM1638_ShowError(void) 
{
	uint8_t index;
	
	for(index = 0; index < 8; index++) 
	{
		TM1638_SendData(index << 1, ERROR_DATA[index]);
	}
}

void TM1638_ShowDecNumber(unsigned long number, uint8_t dots, uint8_t startingPos) 
{
 	uint8_t index;
	
 	if(number > 99999999) 
 	{
    	TM1638_ShowError();
  	} 
	
 	else 
 	{
    	for(index = 0; index < 8 - startingPos; index++) 
		{
        	TM1638_ShowDig(7 - index, number % 10, (dots & (1 << index)) != 0);
        	number /= 10;
    	}
  	}
}

void TM1638_ShowSignedDecNumber(signed long number, uint8_t dots)
{
	if(number >= 0) 
	{
		TM1638_ShowDecNumber(number, dots, 0);
	}
	
	else 
	{
		if(-number > 9999999) 
		{
		    TM1638_ShowError();
		}
		
		else 
		{
			TM1638_ShowDecNumber(-number, dots, 1);
			TM1638_SendData(0, HYPHEN);
		}
	}
}

void TM1638_ShowHexNumber(unsigned long number, uint8_t dots) 
{
	int index;
	
	for (index = 0; index < 8; index++) 
	{
		TM1638_ShowDig(7 - index, number & 0xF, (dots & (1 << index)) != 0);
		number >>= 4;
    }
}

void TM1638_ShowLed(uint8_t number, uint8_t color)
{
	if(number) 
	{
		TM1638_SendData((number << 1) - 1, color);
	}
}

void TM1638_ShowLeds(uint8_t color)
{
	uint8_t index;
	
	for(index = 1; index < 9; index++) 
	{
		TM1638_SendData((index << 1)-1, color);
	}
}

void TM1638_LedBinaryCounter(uint8_t number, uint8_t color)
{
    if(number & 1)
    {
        TM1638_ShowLed(8, color);
    }
    
    else
    {
        TM1638_ShowLed(8, OFF_TM1638);    
    }
    
    if(number & 2)
    {
        TM1638_ShowLed(7, color);
    }
    
    else
    {
        TM1638_ShowLed(7, OFF_TM1638);    
    }
    
    if(number & 4)
    {
        TM1638_ShowLed(6, color);
    }
    
    else
    {
        TM1638_ShowLed(6, OFF_TM1638);    
    }
    
    if(number & 8)
    {
        TM1638_ShowLed(5, color);
    }
    
    else
    {
        TM1638_ShowLed(5, OFF_TM1638);    
    }
    
    if(number & 16)
    {
        TM1638_ShowLed(4, color);
    }
    
    else
    {
        TM1638_ShowLed(4, OFF_TM1638);    
    }
    
    if(number & 32)
    {
        TM1638_ShowLed(3, color);
    }
    
    else
    {
        TM1638_ShowLed(3, OFF_TM1638);    
    }
    
    if(number & 64)
    {
        TM1638_ShowLed(2, color);
    }
    
    else
    {
        TM1638_ShowLed(2, OFF_TM1638);    
    }
    
    if(number & 128)
    {
        TM1638_ShowLed(1, color);
    }
    
    else
    {
        TM1638_ShowLed(1, OFF_TM1638);    
    }
}

void Get_Time(uint8_t time_format)
{
    if(time_format)
    {
        time_hour = Time.hour();
    }
    
    else
    {
        time_hour = Time.hourFormat12();
    }
    
    time_min = Time.minute();
    time_sec = Time.second();
}

void Set_TimeZone_India(void)
{
    Time.zone(5.5); //India is +5.5Hrs from the UTC
}

void Set_TimeZone_Global(uint8_t value)
{
     Time.zone(value);
}

void Show_Time(void)
{
    //Print Time
    TM1638_ShowDig(0, (time_hour / 10),  0);
    TM1638_ShowDig(1, (time_hour % 10),  0);
    
    TM1638_ShowDig(3, (time_min  / 10),  0);
    TM1638_ShowDig(4, (time_min  % 10),  0); 
    
    TM1638_ShowDig(6, (time_sec  / 10),  0); 
    TM1638_ShowDig(7, (time_sec  % 10),  0); 
    
    //Print hyphens (-)
    TM1638_SendData(0x04, HYPHEN);
    TM1638_SendData(0x0A, HYPHEN);
}

void Update_Time(void)
{
    bool update_min = FALSE;
    bool update_hour = FALSE;
    uint8_t temp_time_sec = 0;
    
    if(time_sec > 59)
    {
        time_sec = 0;
        time_min++;
        update_min = TRUE;
    }
    
    if(time_min == 60)
    {
        time_min = 0;
        time_hour++;
        update_hour = TRUE;
    }
    
    if(backupVariables.timeFormat == HR_FORMAT)
    {
        if(time_hour == 24)
        {
            time_hour = 0;
        }
    }
    
    else
    {
        if(time_hour == 13)
        {
            time_hour = 1;
        }
    }
    
    TM1638_ShowDig(6, (time_sec / 10), 0); 
    TM1638_ShowDig(7, (time_sec % 10), 0);

    if(update_min == TRUE)
    {
        update_min = FALSE;
        TM1638_ShowDig(3, (time_min / 10), 0);
        TM1638_ShowDig(4, (time_min % 10), 0);
    }
    
    if(update_hour == TRUE)
    {
        update_hour = FALSE;
        TM1638_ShowDig(0, (time_hour / 10),  0);
        TM1638_ShowDig(1, (time_hour % 10),  0);
    }
    
    if(time_sec == 30)
    {
        temp_time_sec = Time.second();

        if((temp_time_sec > 32) || (temp_time_sec < 28))
        {
            TM1638_LedBinaryCounter(++temp_number, GREEN_TM1638);
            Get_Time(backupVariables.timeFormat);
            Show_Time();
        }
    }
}

void do_every_second(void)
{
    flag = TRUE;
    time_sec++;
}

void process(void)
{
    flagProcess = TRUE;
}

int watchMode(String cmd)
{
    if((cmd == "12H") || (cmd == "12h"))
    {
        backupVariables.timeFormat = AM_PM_FORMAT;
        EEPROM.put(EEPROM_ADDR, backupVariables);
        Get_Time(AM_PM_FORMAT);
        Show_Time();
        return 1;
    }
    
    else if((cmd == "24H") || (cmd == "24h"))
    {
        backupVariables.timeFormat = HR_FORMAT;
        EEPROM.put(EEPROM_ADDR, backupVariables);
        Get_Time(HR_FORMAT);
        Show_Time();
        return 0;
    }
    
    else
    {
        return -1;
    }
}

int displayCtrl(String cmd)
{
    if(cmd == "off")
    {
        TM1638_SetupDisplay(0, 0);
        return 9;
    }
    
    else if(cmd == "0")
    {
        TM1638_SetupDisplay(1, 0);
        backupVariables.intensity = 0;
        EEPROM.put(EEPROM_ADDR, backupVariables);
        return 0;
    }
    
    else if(cmd == "1")
    {
        TM1638_SetupDisplay(1, 1);
        backupVariables.intensity = 1;
        EEPROM.put(EEPROM_ADDR, backupVariables);
        return 1;
    }
    
    else if(cmd == "2")
    {
        TM1638_SetupDisplay(1, 2);
        backupVariables.intensity = 2;
        EEPROM.put(EEPROM_ADDR, backupVariables);
        return 2;
    }
    
    else if(cmd == "3")
    {
        TM1638_SetupDisplay(1, 2);
        backupVariables.intensity = 3;
        EEPROM.put(EEPROM_ADDR, backupVariables);
        return 2;
    }
    
    else if(cmd == "4")
    {
        TM1638_SetupDisplay(1, 4);
        backupVariables.intensity = 4;
        EEPROM.put(EEPROM_ADDR, backupVariables);
        return 4;
    }
    
    else if(cmd == "5")
    {
        TM1638_SetupDisplay(1, 5);
        backupVariables.intensity = 5;
        EEPROM.put(EEPROM_ADDR, backupVariables);
        return 5;
    }
    
    else if(cmd == "6")
    {
        TM1638_SetupDisplay(1, 6);
        backupVariables.intensity = 6;
        EEPROM.put(EEPROM_ADDR, backupVariables);
        return 6;
    }
    
    else if(cmd == "7")
    {
        TM1638_SetupDisplay(1, 7);
        backupVariables.intensity = 7;
        EEPROM.put(EEPROM_ADDR, backupVariables);
        return 7;
    }
    
    else
    {
        return -1;
    }
}

uint8_t TM1638_GetKey(void) 
{
	uint8_t KeyData   = 0;
	uint8_t Status[4] = {0, 0, 0, 0};
	uint8_t index, key_bit;
	
	pinMode(SDA, INPUT);
	pinResetFast(MY_SSEL); //digitalWrite(MY_SSEL, 0);						// Set STROBE = "0"
	SPI_SendData(DATA_READ_KEY_SCAN_MODE);
	
	delayMicroseconds(5);
	
	pinSetFast(MY_SDA); //digitalWrite(SDA, 0x01); //SDA_Write(0x01);
	
	delayMicroseconds(5);	 // wait to scan keys ready
	
	for(index = 0; index < 4; index++) 
	{
		for(key_bit = 0; key_bit < 8; key_bit++)
		{
			pinResetFast(MY_SCL);
			delayMicroseconds(2);
			Status[index] |= (digitalRead(SDA) << key_bit);
			pinSetFast(MY_SCL);
			delayMicroseconds(2);
		}

		delayMicroseconds(5);
	}

	pinSetFast(MY_SSEL);
	delayMicroseconds(10);
	pinMode(SDA, OUTPUT);
	
	if(Status[0] == 0x01)
	{
		KeyData |= 0x01;
	}
	
	if(Status[0] == 0x10)
	{
		KeyData |= 0x05;
	}
	
	if(Status[1] == 0x01)
	{
		KeyData |= 0x02;
	}
	
	if(Status[1] == 0x10)
	{
		KeyData |= 0x06;
	}
	
	if(Status[2] == 0x01)
	{
		KeyData |= 0x03;
	}
	
	if(Status[2] == 0x10)
	{
		KeyData |= 0x07;
	}
	
	if(Status[3] == 0x01)
	{
		KeyData |= 0x04;
	}
	
	if(Status[3] == 0x10)
	{
		KeyData |= 0x08;
	}
	return KeyData;
}