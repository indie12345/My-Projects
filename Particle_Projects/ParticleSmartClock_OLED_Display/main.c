/*********************************************************************
Thanks to Adafruit for their GFX and SSD1306 libraries
*********************************************************************/

#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"

Adafruit_SSD1306 display(-1);

#define FONT_SIZE           5
#define EEPROM_ADDR         0x00
#define AM_PM_FORMAT        0x00
#define HR_FORMAT           0x01
#define DISPLAY_I2C_ADDR    0x3C

bool flag = FALSE;
uint8_t timeHour, timeMin, timeSec;

/* These must be of int type if we want to use Particle.variable function */
struct values 
{
    int intensity; 
    int timeFormat;
}backupVariables;

void updateTime(void);
void updateDisplay(void);
void doEverySecond(void);
int watchMode(String cmd);
void setTimeZoneIndia(void);
void getTime(uint8_t timeFormat);
void setTimeZoneGlobal(uint8_t value);

/* Timer for 1 second interrupt, it will call 
    "doEverySecond" function every second */
Timer timer(1000, doEverySecond);

void setup()   
{   
    /* Expose internal variables */
    Particle.variable("Hour",  timeHour);
    Particle.variable("Minute", timeMin);
    Particle.variable("Second", timeSec);
    Particle.variable("TimeFormat", backupVariables.timeFormat);
    
    /* Expose function to set 12h or 24H mode from the APP or Web Console */
    Particle.function("WatchMode", watchMode);
    
    /* Start the timer and initialize the display */
    timer.start();
    display.begin(SSD1306_SWITCHCAPVCC, DISPLAY_I2C_ADDR);
    
    /* Get data from the EEPROM and load into the structure */
    EEPROM.get(EEPROM_ADDR, backupVariables);
    
    /* if the values from the EEPROM are unexpected then update the EEPROM */
    if(backupVariables.timeFormat > 1)
    {
        backupVariables.intensity = 0;
        backupVariables.timeFormat = 0;
        EEPROM.put(EEPROM_ADDR, backupVariables);
    }
    
    /* Set the time zone of India */
    setTimeZoneIndia();
    
    /* Get the time in 12H or 24H format */
	getTime(backupVariables.timeFormat);
	
	/* Update the display with the time */
	updateDisplay();
	
	/* Disable the RGB LED's default function and control manually */
	RGB.control(true);
    RGB.color(0,0,20);
}

void loop()
{
    /* Update the time if flag is set */
    if(flag)
    {
        flag = false;
        updateTime();
    }
}

/* Timer interrupt handler */
void doEverySecond(void)
{
    flag = true;    
}

/* Set to Indian time zone */
void setTimeZoneIndia(void)
{
    /* India is +5.5Hrs from the UTC */
    Time.zone(5.5);
}

/* Set to any other global time zone */
void setTimeZoneGlobal(uint8_t value)
{
     Time.zone(value);
}

/* Get time from the servers in specific format */
void getTime(uint8_t timeFormat)
{
    if(timeFormat)
    {
        /* Time in 24H format */
        timeHour = Time.hour();
    }
    
    else
    {
        /* Time in 12H format */
        timeHour = Time.hourFormat12();
    }
    
    timeMin = Time.minute();
    timeSec = Time.second();
}

/* Update the display */
void updateDisplay(void)
{
    display.clearDisplay();
    display.setTextSize(FONT_SIZE);
    display.setTextColor(WHITE);
    display.setCursor(0, 15);
    
    /* The function only prints single digit if number is less than 10 so 
        using this case to print numbers below 10 as strings to keep the 
        formatting intact, once number is 10 or more its printed directly */
    switch (timeHour)
    {
        case 0:
            display.println("00");
            break;
        case 1:
            display.println("01");
            break;
        case 2:
            display.println("02");
            break;
            case 3:
            display.println("03");
            break;
        case 4:
            display.println("04");
            break;
        case 5:
            display.println("05");
            break;
        case 6:
            display.println("06");
            break;
        case 7:
            display.println("07");
            break;
        case 8:
            display.println("08");
            break;
        case 9:
            display.println("09"); 
            break;
        default:
            display.println(timeHour);
            break;
    }
    
    display.setCursor(67, 15);
    
    switch (timeMin)
    {
        case 0:
            display.println("00");
            break;
        case 1:
            display.println("01");
            break;
        case 2:
            display.println("02");
            break;
            case 3:
            display.println("03");
            break;
        case 4:
            display.println("04");
            break;
        case 5:
            display.println("05");
            break;
        case 6:
            display.println("06");
            break;
        case 7:
            display.println("07");
            break;
        case 8:
            display.println("08");
            break;
        case 9:
            display.println("09"); 
            break;
        default:
            display.println(timeMin);
            break;
    }
    
    /* Print the colon between minute and hour */
    display.setTextSize(FONT_SIZE - 1);
    display.setCursor(52, 20);
    display.println(":");
}

void updateTime(void)
{
    uint8_t tempTimeSec = 0;
    
    /* Increment second */
    timeSec++;
    
    /* Make seconds '0' when it reaches 60, increase the minute */
    if(timeSec > 59)
    {
        timeSec = 0;
        timeMin++;
    }
    
    /* Make minute '0' when it reaches 60 and increase the hour */
    if(timeMin > 59)
    {
        timeMin = 0;
        timeHour++;
    }
    
    /* Update the hour, depending on the time format */
    if(backupVariables.timeFormat == HR_FORMAT)
    {
        if(timeHour == 24)
        {
            timeHour = 0;
        }
    }
    
    else
    {
        if(timeHour == 13)
        {
            timeHour = 1;
        }
    }
    
    /* When second is '30' that is midway the minute then update the time if 
    internal time is off by more than 2 seconds. We are doing this here as if 
    we do when second is '0' then there might be cases where the used will see 
    the time going back by a minute if the internal time is ahead of actual time.*/
    if(timeSec == 30)
    {
        tempTimeSec = Time.second();
        if((tempTimeSec > 32) || (tempTimeSec < 28))
        {
            getTime(backupVariables.timeFormat);
        }
    }
    
    /* Update the time whenever seconds is '0' */
    if(!timeSec)
    {
        updateDisplay();
    }
    
    /* Draw lines to indicate seconds */
    display.drawLine(4, 1, ((timeSec << 1) + 4), 1, WHITE);
    display.drawLine(4, 3, ((timeSec << 1) + 4), 3, WHITE);
    display.drawLine((124 - (timeSec << 1)), 61, 123, 61, WHITE);
    display.drawLine((124 - (timeSec << 1)), 63, 123, 63, WHITE);
    display.display();
}

/* This function is called when the time mode 
    is change and the function is called */
int watchMode(String cmd)
{
    if((cmd == "12H") || (cmd == "12h") || (cmd == "12"))
    {
        /* Update the variable and store it in EEPROM */
        backupVariables.timeFormat = AM_PM_FORMAT;
        EEPROM.put(EEPROM_ADDR, backupVariables);
        
        /* Get the time in new format and update it on display */
        getTime(AM_PM_FORMAT);
        updateDisplay();
        return 12;
    }
    
    else if((cmd == "24H") || (cmd == "24h") || (cmd == "24"))
    {
        backupVariables.timeFormat = HR_FORMAT;
        EEPROM.put(EEPROM_ADDR, backupVariables);
        getTime(HR_FORMAT);
        updateDisplay();
        return 24;
    }
    
    else
    {
        return 0;
    }
}
