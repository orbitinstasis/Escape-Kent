/*

*/

#include "HX711.h"
#include "EEPROM.h"
#include <Bounce.h>

#define BAUD_RATE 9600
#define ON_BOARD_LED 13
#define DOOR_GATE 23
#define DOUT  3
#define CLK  2
#define SETUP_POWER 12
#define SETUP_READING 10
#define ADDR_READING_1_HEAVIEST_13_BALLS (sizeof(float) * 0)
#define ADDR_READING_2_ALL_BALLS (sizeof(float) * 1)
#define ADDR_READING_3_LIGHTEST_BALL (sizeof(float) * 2)
#define ADDR_MINIMUM_THRESHOLD (sizeof(float) * 3)
#define ADDR_MAXIMUM_THRESHOLD (sizeof(float) * 4)
#define ADDR_ZEROED_OFFSET (sizeof(float) * 5)
#define TEST_LOOPS 30

boolean is_game_finished = false; 
boolean isInCalibratorSetup = false;
HX711 scale(DOUT, CLK);
Bounce bouncer = Bounce( SETUP_READING, 10 ); 
float currentWeight = 0.00f;
float previousWeight = 0.00f;
float calibration_factor = -7050; 
float minimum_threshold = 0.00f;
float maximum_threshold = 0.00f;
float lightest_ball = 0.00f;
long zero_factor = 0.0f;

void setup() {
  Serial.begin(BAUD_RATE);
  Serial.println("Ball Weight Sensing Game by Ben Kazemi for Escape Kent Copyright 2017\n");
  pinMode(DOOR_GATE, OUTPUT);
  digitalWrite(DOOR_GATE, false);
  pinMode(ON_BOARD_LED, OUTPUT);
  pinMode(SETUP_POWER, OUTPUT);
  digitalWrite(SETUP_POWER, LOW);
  pinMode(SETUP_READING, INPUT_PULLUP);
  bouncer.update ( );
  if (digitalRead(SETUP_READING) == 0)  // in setup 
  { 
    isInCalibratorSetup = true;
    digitalWrite(ON_BOARD_LED, 1);
  }
  else                      // not in setup 
  { 
    EEPROM.get(ADDR_ZEROED_OFFSET, zero_factor);
    EEPROM.get(ADDR_MINIMUM_THRESHOLD, minimum_threshold);
    EEPROM.get(ADDR_MAXIMUM_THRESHOLD, maximum_threshold);   
    EEPROM.get(ADDR_READING_3_LIGHTEST_BALL, lightest_ball); 
    Serial.print(zero_factor);
    Serial.print(": Zero Factor;\t");
    Serial.print(minimum_threshold);
    Serial.print("Kg: Minimum Threshold;\t");
    Serial.print(maximum_threshold);
    Serial.println("Kg: Maximum Threshold.\n");
    isInCalibratorSetup = false;
    digitalWrite(ON_BOARD_LED, 0);
    scale.set_scale(calibration_factor); 
    scale.set_offset(zero_factor);
  }
}

void waitForPress(void)
{
  boolean pressed = false;
  while (!pressed)
  {
    bouncer.update();
    if (bouncer.fallingEdge())
    {
      pressed = true;
    }
  }
  Serial.print("Please Wait..\n\n");
  delay(2000);
}

void dumpEEPROM()
{
  for (int i = 0; i < 128; i++) 
  {
    Serial.print(i);
    Serial.print(":\t");
    Serial.println(EEPROM.read(i), DEC);    
  }
}

//void saveValueToEeprom(int addr, float val) 
//{
//  EEPROM.put(addr, val);
//}

//void timedAdjustment()
//{
//  long starttime = millis();
//  long endtime = starttime;
//  while ((endtime - starttime) <=30000)
//  {
//    scale.set_scale(calibration_factor); //Adjust to this calibration factor
//  
//    Serial.print("Reading: ");
//    Serial.print(0.4535 * scale.get_units(), 2);
//    Serial.print(" KG"); //Change this to kg and re-adjust the calibration factor if you follow SI units like a sane person
//    Serial.print(" Calibration Factor: ");
//    Serial.print(calibration_factor);
//    Serial.println();
//  
//    if(Serial.available())
//    {
//      char temp = Serial.read();
//      if(temp == '+' || temp == 'a')
//        calibration_factor += 300;
//      else if(temp == '-' || temp == 'z')
//        calibration_factor -= 300;
//    }
//    endtime = millis();
//  }
//}

void loop() {
  if (isInCalibratorSetup)
  {
    float f = 0.00f;
    Serial.println("Setup button detected, entering calibration setup mode\nPlease push the button ONCE after completing each step.\nRepeat these steps everytime a weight element changes in the game changes (different balls/platform).\nBe careful not to touch the weight elements during calibration.\n");
   
    Serial.println("\nStep 1: Press the button to check it works.\n");

    waitForPress();
    Serial.println("Step 2: Place whatever platform you use to place the balls onto, onto the weight sensor WITHOUT *ANY* BALLS.\nWe will save it's weight and offset it during the game at boot so you don't always have to zero the game.\n");
    waitForPress();

    scale.set_scale();
    scale.tare();  
    long zero_factor_long = scale.read_average();   
    EEPROM.put(ADDR_ZEROED_OFFSET, zero_factor_long);
    long l = 0;
    EEPROM.get(ADDR_ZEROED_OFFSET, l);
    Serial.print(l);
    Serial.println(" saved to EEPROM as zero factor.\n");  
     
//    Serial.println("Good, you will see readings for 30 seconds showing an approximate weight, you want to decrease this number until it's 0");
//    Serial.println("In the arduino terminal:\n\tEnter a + or a to increase calibration factor");
//    Serial.println("\tEnter a - or z to decrease calibration factor");  
//    Serial.println("\t\tYou can enter many characters at once and then press enter to speed the process up. Press button when ready");  
//    waitForPress();
//loop:
//    timedAdjustment();
//    Serial.println("Hit the button ONCE in the next 10 seconds if you wish to continue adjusting to get to 0");
//    long starttime = millis();
//    long endtime = starttime;
//    while ((endtime - starttime) <= 10000)
//    {
//      boolean pressed = false;
//      while (!pressed)
//      {
//        bouncer.update();
//        if (bouncer.fallingEdge())
//        {
//          goto loop;
//        }
//      }
//      endtime = millis();
//    }
//    EEPROM.put(ADDR_CALIBRATION_FACTOR, calibration_factor_setup_only);
//    EEPROM.get(ADDR_CALIBRATION_FACTOR, f);
//    Serial.print(f);
//    Serial.println(" read as setup calibration factor.\n");   

    scale.set_scale(calibration_factor); 
    
    Serial.println("Step 3: Place ONLY the THIRTEEN *HEAVIEST* WEIGHTS onto the weight element. (Weigh them before placing on the sensor)");
    waitForPress();
    float reading_1 = 0.4535 * scale.get_units();
    EEPROM.put(ADDR_READING_1_HEAVIEST_13_BALLS, reading_1);
    EEPROM.get(ADDR_READING_1_HEAVIEST_13_BALLS, f);
    Serial.print(f);
    Serial.println("Kg saved to EEPROM as reading 1 (thirteen heaviest balls).");

    Serial.println("\nStep 4: Now place the final fourteenth ball in with the rest.");
    waitForPress();
    float reading_2 = 0.4535 * scale.get_units();
    EEPROM.put(ADDR_READING_2_ALL_BALLS, reading_2);
    EEPROM.get(ADDR_READING_2_ALL_BALLS, f);
    Serial.print(f);
    Serial.println("Kg saved to EEPROM as reading 2 (all balls).\n");
    delay(500);
    
    EEPROM.put(ADDR_READING_3_LIGHTEST_BALL, (reading_2 - reading_1));
    EEPROM.get(ADDR_READING_3_LIGHTEST_BALL, f);
    Serial.print(f);
    Serial.println("Kg calculated and read from EEPROM as reading 3 (lightest ball).\n");
    delay(500);
    
    EEPROM.put(ADDR_MINIMUM_THRESHOLD, (reading_2 - ((reading_2 - reading_1) * 0.5)));
    EEPROM.get(ADDR_MINIMUM_THRESHOLD, f);
    Serial.print(f);
    Serial.println("Kg calculated and read from EEPROM as MINIMUM threshold.\n");
    delay(500);
    
    EEPROM.put(ADDR_MAXIMUM_THRESHOLD, (reading_2 + ((reading_2 - reading_1) * 0.5)));
    EEPROM.get(ADDR_MAXIMUM_THRESHOLD, f);
    Serial.print(f);
    Serial.println("Kg calculated and read from EEPROM as MAXIMUM threshold.\n");

    Serial.println("\nDumping EEPROM after button press.");
    waitForPress();
    dumpEEPROM();
    Serial.println("\nFinished.\nReboot without holding button to enter game mode, or press button now to start. ");
    waitForPress();
    EEPROM.get(ADDR_ZEROED_OFFSET, zero_factor);
    Serial.print(zero_factor);
    Serial.println(" Zero Factor.");
    EEPROM.get(ADDR_ZEROED_OFFSET, zero_factor);
    EEPROM.get(ADDR_MINIMUM_THRESHOLD, minimum_threshold);
    EEPROM.get(ADDR_MAXIMUM_THRESHOLD, maximum_threshold);   
    EEPROM.get(ADDR_READING_3_LIGHTEST_BALL, lightest_ball); 
    Serial.print(zero_factor);
    Serial.print(": Zero Factor;\t");
    Serial.print(minimum_threshold);
    Serial.print("Kg: Minimum Threshold;\t");
    Serial.print(maximum_threshold);
    Serial.println("Kg: Maximum Threshold.\n");
    isInCalibratorSetup = false;
    digitalWrite(ON_BOARD_LED, 0);
    scale.set_scale(calibration_factor); 
    scale.set_offset(zero_factor);
  }
  else
  {
    currentWeight = 0.4535 * scale.get_units();
    
    if (abs(currentWeight - previousWeight) > 0.01)
    {
      Serial.print("Reading: ");
      Serial.print(currentWeight, 2);
      Serial.println(" Kg"); 
      previousWeight = currentWeight;
    }
    
    if (!is_game_finished && (currentWeight > minimum_threshold && currentWeight < maximum_threshold))
    {
      digitalWrite(ON_BOARD_LED, HIGH);
      Serial.println("\nYou're in the right region."); 
      boolean flag = true;
      for (int i = 0; i < TEST_LOOPS && flag; i++)
      {
        float test = 0.4535 * scale.get_units();
        if (abs(currentWeight - test) > lightest_ball / 4) {
          Serial.println("I think you're manipulating the system."); 
          flag = false;
        }
        Serial.print(i + 1); 
        Serial.print("/"); 
        Serial.print(TEST_LOOPS);
        Serial.println(" passes."); 
        if (i >= TEST_LOOPS - 1) 
        {
          is_game_finished = true;
          digitalWrite(DOOR_GATE, is_game_finished);
        }
      }
    }
    else 
      digitalWrite(ON_BOARD_LED, LOW);

    if (is_game_finished) 
    {
      Serial.println("\nYou've won. The door's open.\n\tThe game will reset, and the door will lock when you remove the weights.");
      currentWeight = 0.4535 * scale.get_units();
      while (currentWeight > (minimum_threshold / 2))   // UNLOCK DOOR HERE
      {
        currentWeight = 0.4535 * scale.get_units();
      }
      Serial.println("Game has reset, door is now locked!");
      is_game_finished = false;
      digitalWrite(DOOR_GATE, is_game_finished);
    }
  }
}

