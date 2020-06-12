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
int timeHour, timeMin, timeSec;

/* These must be of int type if we want to use Particle.variable function */
int backupVariable;

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
    Particle.variable("TimeFormat", backupVariable);
    
    /* Expose function to set 12h or 24H mode from the APP or Web Console */
    Particle.function("WatchMode", watchMode);
    
    /* Start the timer and initialize the display */
    timer.start();
    display.begin(SSD1306_SWITCHCAPVCC, DISPLAY_I2C_ADDR);
    
    /* Get data from the EEPROM and load into the structure */
    EEPROM.get(EEPROM_ADDR, backupVariable);
    
    /* if the value from the EEPROM is unexpected then update the EEPROM */
    if((backupVariable != 1) && (backupVariable != 0))
    {
        backupVariable = 0;
        EEPROM.put(EEPROM_ADDR, backupVariable);
    }
    
    /* Set the time zone of India */
    setTimeZoneIndia();
    
    /* Get the time in 12H or 24H format */
	getTime(backupVariable);
	
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
    int now = Time.now();
    
    if(timeFormat)
    {
        /* Time in 24H format */
        timeHour = Time.hour(now);
    }
    
    else
    {
        /* Time in 12H format */
        timeHour = Time.hourFormat12(now);
    }
    
    timeMin = Time.minute(now);
    timeSec = Time.second(now);
}

/* Update the display */
void updateDisplay(void)
{
    char str[8];
    
    display.clearDisplay();
    display.setTextSize(FONT_SIZE);
    display.setTextColor(WHITE);
    display.setCursor(0, 15);
    
    snprintf(str, sizeof(str), "%02d", timeHour);
    display.println(str);
    
    display.setCursor(67, 15);
    
    snprintf(str, sizeof(str), "%02d", timeMin);
    display.println(str);
    
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
    if(backupVariable == HR_FORMAT)
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
            getTime(backupVariable);
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
        backupVariable = AM_PM_FORMAT;
        EEPROM.put(EEPROM_ADDR, backupVariable);
        
        /* Get the time in new format and update it on display */
        getTime(AM_PM_FORMAT);
        updateDisplay();
        return 12;
    }
    
    else if((cmd == "24H") || (cmd == "24h") || (cmd == "24"))
    {
        backupVariable = HR_FORMAT;
        EEPROM.put(EEPROM_ADDR, backupVariable);
        getTime(HR_FORMAT);
        updateDisplay();
        return 24;
    }
    
    else
    {
        return 0;
    }
}