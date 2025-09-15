/*************************************************************
Title         :   Home automation using blynk
Description   :   To control light's brigntness with brightness,monitor temperature , monitor water level in the tank through blynk app
Pheripherals  :   Arduino UNO , Temperature system, LED, LDR module, Serial Tank, Blynk cloud, Blynk App.
 *************************************************************/

// Template ID, Device Name and Auth Token are provided by the Blynk.Cloud
// See the Device Info tab, or Template settings
#define BLYNK_TEMPLATE_ID     "TMPL3luUCacTI"
#define BLYNK_TEMPLATE_NAME   "Home Automation"
#define BLYNK_AUTH_TOKEN      "wSEpEB51gFg1EhP4CDRDXrblj_IF3xoD"



// Comment this out to disable prints 
//#define BLYNK_PRINT Serial

#include <SPI.h>
#include <Ethernet.h>
#include <BlynkSimpleEthernet.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include "main.h"
#include "temperature_system.h"
#include "ldr.h"
#include "serial_tank.h"

char auth[] = BLYNK_AUTH_TOKEN;
bool heater_sw,inlet_sw,outlet_sw, cooler_sw;
unsigned int tank_volume;

BlynkTimer timer;

LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

// This function is called every time the Virtual Pin 0 state changes
/*To turn ON and OFF cooler based virtual PIN value*/
BLYNK_WRITE(COOLER_V_PIN)
{
  cooler_sw = param.asInt();
  if (cooler_sw)
  {
    cooler_control(ON);
     lcd.setCursor(8,0);
    lcd.print("CLR_ON ");
  }
  else
  {
    cooler_control(OFF);
     lcd.setCursor(8,0);
    lcd.print("CLR_OFF");
  }
}
/*To turn ON and OFF heater based virtual PIN value*/
BLYNK_WRITE(HEATER_V_PIN )
{
  heater_sw = param.asInt();
  if(heater_sw)
  {
    heater_control(ON);
     lcd.setCursor(8,0);
    lcd.print("HTR_ON ");
  }
  else
  {
    heater_control(OFF);
     lcd.setCursor(8,0);
    lcd.print("HTR_OFF");
  }

}
/*To turn ON and OFF inlet vale based virtual PIN value*/
BLYNK_WRITE(INLET_V_PIN)
{/*to read status of the inlet button*/
  inlet_sw = param.asInt();
  if(inlet_sw)
  {/*turn on the inlet valve*/
    enable_inlet();
    lcd.setCursor(7,1);
    lcd.print("IN_FL_ON ");
  }
  else
  {
    disable_inlet();
    lcd.setCursor(7,1);
    lcd.print("IN_FL_OFF");
  }
}
/*To turn ON and OFF outlet value based virtual switch value*/
BLYNK_WRITE(OUTLET_V_PIN)
{
  outlet_sw = param.asInt();
  if(outlet_sw)
  {/*turn on the inlet valve*/
    enable_outlet();
    lcd.setCursor(7,1);
    lcd.print("OT_FL_ON ");
  }
  else
  {
    disable_outlet();
    lcd.setCursor(7,1);
    lcd.print("OT_FL_OFF");
  }
}
/* To display temperature as gauge on the Blynk App*/  
void update_temperature_reading()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite( TEMPERATURE_GAUGE, read_temperature() );
  
}

/*To turn off the heater if the temperature raises above 35 deg C*/
void handle_temp(void)
{
  /* read temp and compare with 35 and if heater is on*/
  if( read_temperature() > (float) 35 && heater_sw)
  { /*turn OFF the heater*/
     heater_sw = 0;
     heater_control(OFF);
     Blynk.virtualWrite(HEATER_V_PIN, 0);
    lcd.setCursor(8,0);
    lcd.print("HTR_OFF");
  }


}

/*To control water volume above 2000ltrs*/
void handle_tank(void)
{
  /*read volume and compare it with 2000, inlet valve is off*/
  if((tank_volume < 2000) && (inlet_sw == 0))
  {
    enable_inlet();
    inlet_sw = 1;
    /*reflect the status of switch on blynk app*/
    Blynk.virtualWrite(INLET_V_PIN, 1);
    lcd.setCursor(7,1);
    lcd.print("IN_FL_ON ");
  }
    /*if the tank is full turn off the inlet*/
    /*if the volume is 3000 ltr, inlet is on*/
  if((tank_volume == 3000) && (inlet_sw == 1))
  {
      disable_inlet();
    inlet_sw = 0;
    /*reflect the status of switch on blynk app*/
    Blynk.virtualWrite(INLET_V_PIN, 0);
    lcd.setCursor(7,1);
    lcd.print("IN_FL_OFF");

  
  }

}


void setup(void)
{
  /*initialise the clcd*/
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.home();
   lcd.setCursor(0,0);
  lcd.print("Home Automation");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("T=");
  lcd.setCursor(0,1);
  lcd.print("V=");
  Blynk.begin(BLYNK_AUTH_TOKEN);
  init_ldr();
  init_temperature_system();
  init_serial_tank();
  /*update tempature to the blynk app every one second*/
  timer.setInterval(1000, update_temperature_reading );
}
String temp;
void loop(void) 
{
      /*read the value from ldr and control led brightness */
      brightness_control();
      /*read the temp and convert into string and display it on the clcd*/
       temp = String (read_temperature(), 2); // reading 2 fields
       lcd.setCursor(2,0);
       lcd.print(temp);
       /*to read the volume and display it on CLCD*/
       tank_volume = volume();
       lcd.setCursor(2,1);
       lcd.print(tank_volume);

       /*to connect the device continously to cloud*/
      Blynk.run();
      /*turning on the timer*/
      timer.run();
       handle_temp();
       handle_tank();
}
