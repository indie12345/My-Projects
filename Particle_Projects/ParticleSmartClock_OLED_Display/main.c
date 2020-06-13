/*********************************************************************
Thanks to Adafruit for their GFX and SSD1306 libraries
*********************************************************************/

#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"

Adafruit_SSD1306 display(-1);

#define FONT_SIZE           4
#define EEPROM_ADDR         0x00
#define AM_PM_FORMAT        0x00
#define HR_FORMAT           0x01
#define DISPLAY_I2C_ADDR    0x3C

bool flag = FALSE;
int timeHour, timeMin, timeSec, timeNow, timeInPreviousReading, timeFormat;

/* These must be of int type if we want to use Particle.variable function */
int backupVariable;

int watchMode(String cmd);
void setTimeZoneIndia(void);
void setTimeZoneGlobal(uint8_t value);

void setup()   
{   
    /* Expose internal variables */
    Particle.variable("Hour",  timeHour);
    Particle.variable("Minute", timeMin);
    Particle.variable("Second", timeSec);
    Particle.variable("TimeFormat", backupVariable);
    
    /* Expose function to set 12h or 24H mode from the APP or Web Console */
    Particle.function("WatchMode", watchMode);
    
    /* Initialize the display */
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
	
	/* Disable the RGB LED's default function and control manually */
	RGB.control(true);
    RGB.color(0,0,20);
}

void loop()
{
    timeNow = Time.now();
    
    if(timeNow != timeInPreviousReading)
    {
        timeInPreviousReading = timeNow;
        timeSec = Time.second(timeNow);
        
        /* Draw lines to indicate seconds */
        display.drawLine(4, 1, ((timeSec << 1) + 4), 1, WHITE);
        display.drawLine(4, 3, ((timeSec << 1) + 4), 3, WHITE);
        display.drawLine((124 - (timeSec << 1)), 61, 123, 61, WHITE);
        display.drawLine((124 - (timeSec << 1)), 63, 123, 63, WHITE);
        display.display();
        
    }
    
    if(timeMin != Time.minute(timeNow))
    {
        timeMin = Time.minute(timeNow);
        timeHour = Time.hour(timeNow);
        display.clearDisplay();
        display.setTextSize(FONT_SIZE);
        display.setTextColor(WHITE);
        display.setCursor(0, 15);
        display.println(Time.format("%H:%M"));
        display.display();
    }
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

/* This function is called when the time mode 
    is change and the function is called */
int watchMode(String cmd)
{
    /* Fill timeMin with some unexpected value to that 
    doesn't match with the value from Time.minute function 
    and therefore get's updated and printed on the display */
    timeMin = 0xFF;    
    
    if((cmd == "12H") || (cmd == "12h") || (cmd == "12"))
    {
        /* Update the variable and store it in EEPROM */
        backupVariable = AM_PM_FORMAT;
        EEPROM.put(EEPROM_ADDR, backupVariable);
        
        return 12;
    }
    
    else if((cmd == "24H") || (cmd == "24h") || (cmd == "24"))
    {
        backupVariable = HR_FORMAT;
        EEPROM.put(EEPROM_ADDR, backupVariable);
        
        return 24;
    }
    
    else
    {
        return 0;
    }
}