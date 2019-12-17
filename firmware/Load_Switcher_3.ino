// ************************************************ //
// ********** Load Switcher UNIT ****************** //
// ************************************************ //
// By M. Little (m.e.little@lboro.ac.uk)            //
// December 2019                                    //
// ************************************************ //
// Overview
// This program uses a 4 ways relay control unit, Arduino Uno
// and Velleman VMA203 LCD and keypad shield
// It is designed to have 4 seperate 5 or 12V loads attached (up to 10A)
// It will then switch them on and off at different rates
// The rates can be adjusted via the keypad
// The rates are displayed on the LCD
// Datasheet for Velleman LCD and keypad shield
// https://www.velleman.eu/downloads/29/vma203_a4v03.pdf

// General configuration
#include "config.h"

// General Arduino features
#include <Arduino.h>
#include <EEPROM.h>
#include <LiquidCrystal.h>

// select the pins used on the LCD panel
LiquidCrystal lcd(LCD_RS , LCD_E, LCD_DATA1, LCD_DATA2, LCD_DATA3, LCD_DATA4);

// define some values used by the panel and buttons
int lcd_key = 0;
int adc_key_in = 0;

int display_Mode = 0;
int max_display_Mode = 9;

uint32_t  load1_ON_time;
uint32_t  load1_OFF_time;
uint32_t  load1_timer;
bool load1_flag = LOW;

uint32_t  load2_ON_time;
uint32_t  load2_OFF_time;
uint32_t  load2_timer;
bool load2_flag = LOW;

uint32_t  load3_ON_time;
uint32_t  load3_OFF_time;
uint32_t  load3_timer;
bool load3_flag = LOW;

uint32_t  load4_ON_time;
uint32_t  load4_OFF_time;
uint32_t  load4_timer;
bool load4_flag = LOW;

bool button_press_flag = LOW; // Stops flying around in a loop

// read the buttons
int read_LCD_buttons()
{
  adc_key_in = analogRead(0); // read the value from the sensor
  //Serial.println(adc_key_in);
  // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
  // we add approx 50 to those values and check to see if we are close
  if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
  // For V1.1 us this threshold
  if (adc_key_in < 50) return btnRIGHT;
  if (adc_key_in < 200) return btnUP;
  if (adc_key_in < 350) return btnDOWN;
  if (adc_key_in < 500) return btnLEFT;
  if (adc_key_in < 850) return btnSELECT;
  return btnNONE; // when all others fail, return this...
}


void setup()
{
  Serial.begin(9600);

  pinMode(LCD_LED, OUTPUT);
  digitalWrite(LCD_LED, HIGH);

  pinMode(LOAD1_PIN, OUTPUT);
  pinMode(LOAD2_PIN, OUTPUT);
  pinMode(LOAD3_PIN, OUTPUT);
  pinMode(LOAD4_PIN, OUTPUT);
  // Set all relays low
  digitalWrite(LOAD1_PIN, LOW);
  digitalWrite(LOAD2_PIN, LOW);
  digitalWrite(LOAD3_PIN, LOW);
  digitalWrite(LOAD4_PIN, LOW);

  // LCD setup
  lcd.begin(16, 2); // start the library
  lcd.setCursor(0, 0);
  lcd.print(F("LOAD SWITCHER v1.0")); // print a simple message


  // Read in the saved on/off times for each relay
  load1_ON_time = EEPROMReadInt(0);
  load1_OFF_time = EEPROMReadInt(2);
  load2_ON_time = EEPROMReadInt(4);
  load2_OFF_time = EEPROMReadInt(6);
  load3_ON_time = EEPROMReadInt(8);
  load3_OFF_time = EEPROMReadInt(10);
  load4_ON_time = EEPROMReadInt(12);
  load4_OFF_time = EEPROMReadInt(14);

  if (DEBUG == 1)
  {
    Serial.print(F("Load 1 ON:"));
    Serial.print(load1_ON_time);
    Serial.print(F(" OFF:"));
    Serial.println(load1_OFF_time);

    Serial.print(F("Load 2 ON:"));
    Serial.print(load2_ON_time);
    Serial.print(F(" OFF:"));
    Serial.println(load2_OFF_time);

    Serial.print(F("Load 3 ON:"));
    Serial.print(load3_ON_time);
    Serial.print(F(" OFF:"));
    Serial.println(load3_OFF_time);

    Serial.print(F("Load 4 ON:"));
    Serial.print(load4_ON_time);
    Serial.print(F(" OFF:"));
    Serial.println(load4_OFF_time);

  }
  delay(1000);  // Wait to display message

  // Here we want to start the timers: OFF position first
  load1_timer = millis();
  load2_timer = millis();
  load3_timer = millis();
  load4_timer = millis();
  lcd.clear();
}

void loop()
{
  sortDisplay();
  sortLoads();
  if (display_Mode == 0)
  {
    // Show the seconds to time things
    lcd.setCursor(14, 1); // move cursor to second line "1" and 9 spaces over
    int seconds = (millis() / 1000) % 60;
    lcd.print(seconds); // display seconds elapsed since power-up
    if (seconds < 10)
    {
      lcd.print(' ');
    }
  }

  lcd_key = read_LCD_buttons(); // read the buttons
  updateButtons();
  switchLoads();
  delay(50); // Slow things down a bit
}

void sortDisplay()
{
  switch (display_Mode)
  {
    case 0:
      {
        // Normal display
        lcd.setCursor(0, 0);
        lcd.print(F("1: ")); // print a simple message
        if (load1_flag == HIGH)
        {
          lcd.print(F("ON ")); // print a simple message
        }
        else
        {
          lcd.print(F("OFF")); // print a simple message
        }
        lcd.setCursor(7, 0);
        lcd.print(F("2: ")); // print a simple message
        if (load2_flag == HIGH)
        {
          lcd.print(F("ON ")); // print a simple message
        }
        else
        {
          lcd.print(F("OFF")); // print a simple message
        }
        lcd.setCursor(0, 1);
        lcd.print(F("3: ")); // print a simple message
        if (load3_flag == HIGH)
        {
          lcd.print(F("ON ")); // print a simple message
        }
        else
        {
          lcd.print(F("OFF")); // print a simple message
        }
        lcd.setCursor(7, 1);
        lcd.print(F("4: ")); // print a simple message
        if (load4_flag == HIGH)
        {
          lcd.print(F("ON ")); // print a simple message
        }
        else
        {
          lcd.print(F("OFF")); // print a simple message
        }
        break;
      }
    case 1:
      {
        lcd.setCursor(0, 0);
        lcd.print(F("Load 1 ON Time: ")); // print a simple message
        lcd.setCursor(0, 1);
        lcd.print(load1_ON_time); // print a simple message
        lcd.print("          ");
        break;
      }
    case 2:
      {
        lcd.setCursor(0, 0);
        lcd.print(F("Load 1 OFF Time: ")); // print a simple message
        lcd.setCursor(0, 1);
        lcd.print(load1_OFF_time); // print a simple message
        lcd.print("          ");
        break;
      }
    case 3:
      {
        lcd.setCursor(0, 0);
        lcd.print(F("Load 2 ON Time: ")); // print a simple message
        lcd.setCursor(0, 1);
        lcd.print(load2_ON_time); // print a simple message
        lcd.print("          ");
        break;
      }
    case 4:
      {
        lcd.setCursor(0, 0);
        lcd.print(F("Load 2 OFF Time: ")); // print a simple message
        lcd.setCursor(0, 1);
        lcd.print(load2_OFF_time); // print a simple message
        lcd.print("          ");
        break;
      }
    case 5:
      {
        lcd.setCursor(0, 0);
        lcd.print(F("Load 3 ON Time: ")); // print a simple message
        lcd.setCursor(0, 1);
        lcd.print(load3_ON_time); // print a simple message
        lcd.print("          ");
        break;
      }
    case 6:
      {
        lcd.setCursor(0, 0);
        lcd.print(F("Load 3 OFF Time: ")); // print a simple message
        lcd.setCursor(0, 1);
        lcd.print(load3_OFF_time); // print a simple message
        lcd.print("          ");
        break;
      }
    case 7:
      {
        lcd.setCursor(0, 0);
        lcd.print(F("Load 4 ON Time: ")); // print a simple message
        lcd.setCursor(0, 1);
        lcd.print(load4_ON_time); // print a simple message
        lcd.print("          ");
        break;
      }
    case 8:
      {
        lcd.setCursor(0, 0);
        lcd.print(F("Load 4 OFF Time: ")); // print a simple message
        lcd.setCursor(0, 1);
        lcd.print(load4_OFF_time); // print a simple message
        lcd.print("          ");
        break;
      }
  }
}
void updateButtons()
{
  switch (lcd_key) // depending on which button was pushed, we perform an action
  {
    case btnRIGHT:
      {
        if (button_press_flag == LOW)
        {
          button_press_flag = HIGH;
          writeToEEPROM();
          
          display_Mode = display_Mode + 1;
          lcd.clear();
          if (display_Mode >= max_display_Mode)
          {
            display_Mode = 0;
          }
        }
        break;
      }
    case btnLEFT:
      {
        if (button_press_flag == LOW)
        {
          button_press_flag = HIGH;
          // Here we need to decide to write to eeprom
          writeToEEPROM();
          display_Mode = display_Mode - 1;
          lcd.clear();
          if (display_Mode < 0)
          {
            display_Mode = max_display_Mode-1;
          }    
        }
        break;
      }
    case btnUP:
      {
        if (button_press_flag == LOW)
        {
          button_press_flag = HIGH;
          if (display_Mode == 1)
          {
            load1_ON_time = load1_ON_time + 1;
          }
          if (display_Mode == 2)
          {
            load1_OFF_time = load1_OFF_time + 1;
          }
          if (display_Mode == 3)
          {
            load2_ON_time = load2_ON_time + 1;
          }
          if (display_Mode == 4)
          {
            load2_OFF_time = load2_OFF_time + 1;
          }
          if (display_Mode == 5)
          {
            load3_ON_time = load3_ON_time + 1;
          }
          if (display_Mode == 6)
          {
            load3_OFF_time = load3_OFF_time + 1;
          }
          if (display_Mode == 7)
          {
            load4_ON_time = load4_ON_time + 1;
          }
          if (display_Mode == 8)
          {
            load4_OFF_time = load4_OFF_time + 1;
          }
        }
        break;
      }
    case btnDOWN:
      {
        if (button_press_flag == LOW)
        {
          button_press_flag = HIGH;
          if (display_Mode == 1)
          {
            //Adjust Timer 1
            if (load1_ON_time != 0)
            {
              load1_ON_time = load1_ON_time - 1;
            }
          }
          if (display_Mode == 2)
          {
            //Adjust Timer 1
            if (load1_OFF_time != 0)
            {
              load1_OFF_time = load1_OFF_time - 1;
            }
          }
          if (display_Mode == 3)
          {
            //Adjust Timer 2

            if (load2_ON_time != 0)
            {
              load2_ON_time = load2_ON_time - 1;
            }
          }
          if (display_Mode == 4)
          {
            //Adjust Timer 2
            if (load2_OFF_time != 0)
            {
              load2_OFF_time = load2_OFF_time - 1;
            }
          }
          if (display_Mode == 5)
          {
            //Adjust Timer 3
            if (load3_ON_time != 0)
            {
              load3_ON_time = load3_ON_time - 1;
            }
          }
          if (display_Mode == 6)
          {
            //Adjust Timer 3
            if (load3_OFF_time != 0)
            {
              load3_OFF_time = load3_OFF_time - 1;
            }
          }
          if (display_Mode == 7)
          {
            //Adjust Timer 4
            if (load4_ON_time != 0)
            {
              load4_ON_time = load4_ON_time - 1;
            }
          }
          if (display_Mode == 8)
          {
            //Adjust Timer 4
            if (load4_OFF_time != 0)
            {
              load4_OFF_time = load4_OFF_time - 1;
            }
          }
        }
        break;
      }

    case btnSELECT:
      {
        if (button_press_flag == LOW)
        {
          button_press_flag = HIGH;
        }
        break;
      }
    case btnNONE:
      {
        button_press_flag = LOW;
        break;
      }
  }
}

void sortLoads()
{
  // Sort out Load 1
  if (millis() > (load1_timer + (load1_OFF_time * 1000)) && (load1_flag == LOW) && load1_OFF_time != 0)
  {
    load1_flag = HIGH;
    load1_timer = millis();
  }
  if (millis() > (load1_timer + (load1_ON_time * 1000)) && (load1_flag == HIGH) && load1_ON_time != 0)
  {
    load1_flag = LOW;
    load1_timer = millis();
  }
  if (load1_OFF_time == 0 || load1_ON_time == 0 || display_Mode != 0)
  {
    //In this case load is OFF
    load1_flag = LOW;
  }

  // Sort out Load 2
  if (millis() > (load2_timer + (load2_OFF_time * 1000)) && (load2_flag == LOW) && load2_OFF_time != 0)
  {
    load2_flag = HIGH;
    load2_timer = millis();
  }
  if (millis() > (load2_timer + (load2_ON_time * 1000)) && (load2_flag == HIGH) && load2_ON_time != 0)
  {
    load2_flag = LOW;
    load2_timer = millis();
  }
  if (load2_OFF_time == 0 || load2_ON_time == 0 || display_Mode != 0)
  {
    //In this case load is OFF
    load2_flag = LOW;
  }

  // Sort out Load 3
  if (millis() > (load3_timer + (load3_OFF_time * 1000)) && (load3_flag == LOW) && load3_OFF_time != 0)
  {
    load3_flag = HIGH;
    load3_timer = millis();
  }
  if (millis() > (load3_timer + (load3_ON_time * 1000)) && (load3_flag == HIGH) && load3_ON_time != 0)
  {
    load3_flag = LOW;
    load3_timer = millis();
  }
  if (load3_OFF_time == 0 || load3_ON_time == 0 || display_Mode != 0)
  {
    //In this case load is OFF
    load3_flag = LOW;
  }

  // Sort out Load 4
  if (millis() > (load4_timer + (load4_OFF_time * 1000)) && (load4_flag == LOW) && load4_OFF_time != 0)
  {
    load4_flag = HIGH;
    load4_timer = millis();
  }
  if (millis() > (load4_timer + (load4_ON_time * 1000)) && (load4_flag == HIGH) && load4_ON_time != 0)
  {
    load4_flag = LOW;
    load4_timer = millis();
  }
  if (load4_OFF_time == 0 || load4_ON_time == 0 || display_Mode != 0)
  {
    //In this case load is OFF
    load4_flag = LOW;
  }
}


void writeToEEPROM()
{ // Here we need to decide to write to eeprom
  if (display_Mode == 1)
  {
    // Store new timer value to EEPROM
    EEPROMWriteInt(0, load1_ON_time);
  }
  if (display_Mode == 2)
  {
    // Store new timer value to EEPROM
    EEPROMWriteInt(2, load1_OFF_time);
  }
  if (display_Mode == 3)
  {
    // Store new timer value to EEPROM
    EEPROMWriteInt(4, load2_ON_time);
  }
  if (display_Mode == 4)
  {
    // Store new timer value to EEPROM
    EEPROMWriteInt(6, load2_OFF_time);
  }
  if (display_Mode == 5)
  {
    // Store new timer value to EEPROM
    EEPROMWriteInt(8, load3_ON_time);
  }
  if (display_Mode == 6)
  {
    // Store new timer value to EEPROM
    EEPROMWriteInt(10, load3_OFF_time);
  }
  if (display_Mode == 7)
  {
    // Store new timer value to EEPROM
    EEPROMWriteInt(12, load4_ON_time);
  }
  if (display_Mode == 8)
  {
    // Store new timer value to EEPROM
    EEPROMWriteInt(14, load4_OFF_time);
  }
}


void switchLoads()
{
  // Here we switch on/off loads as needed:
  if (load1_flag == HIGH)
  {
    digitalWrite(LOAD1_PIN, HIGH);
  } else
  {
    digitalWrite(LOAD1_PIN, LOW);
  }
  if (load2_flag == HIGH)
  {
    digitalWrite(LOAD2_PIN, HIGH);
  } else
  {
    digitalWrite(LOAD2_PIN, LOW);
  }
  if (load3_flag == HIGH)
  {
    digitalWrite(LOAD3_PIN, HIGH);
  } else
  {
    digitalWrite(LOAD3_PIN, LOW);
  }
  if (load4_flag == HIGH)
  {
    digitalWrite(LOAD4_PIN, HIGH);
  } else
  {
    digitalWrite(LOAD4_PIN, LOW);
  }
}



void EEPROMWriteInt(int address, int value)
{
  byte two = (value & 0xFF);
  byte one = ((value >> 8) & 0xFF);

  EEPROM.update(address, two);
  EEPROM.update(address + 1, one);
}

int EEPROMReadInt(int address)
{
  long two = EEPROM.read(address);
  long one = EEPROM.read(address + 1);

  return ((two << 0) & 0xFFFFFF) + ((one << 8) & 0xFFFFFFFF);
}
