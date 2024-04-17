#include <Arduino.h>
#include <NimBLEDevice.h>
#include <LegoinoCommon.h>
#include <Lpf2Hub.h>
#include <Bounce2.h>
#include "main.h"
#include <EEPROM.h>
#define EEPROM_SIZE 10

// create a hub instance
Lpf2Hub myTrainHub;
byte port = (byte)PoweredUpHubPort::A;
#define ADC_THROTHLE_PIN    34 // ADC2_4 is GPIO 36 on ESP32
#define BUTTON_STOP_PIN     19
#define BUTTON_LIGHT_PIN    18
#define BUTTON_FUEL_PIN     5
#define BUTTON_SOUND_PIN    17

#define LED_PWR_G           21
#define LED_CON_B           22
#define LED_BAT_R           23

#define ENABLE_LED          HIGH
#define DISABLE_LED         LOW

#define LED_SET(ledPin, STATE) digitalWrite(ledPin, STATE)

uint16_t calibAdcMin = 0;
uint16_t calibAdcMax = 0;
uint16_t calibAdcCenter = 0;

uint8_t trainRolling = 0;
uint8_t trainStopButton = 0;
int16_t throthlePrev = 0;
int16_t throthleStop = 0;
uint8_t delayCntr = 0;
uint8_t ledBlink = 0;

Bounce buttonStop;
Bounce buttonLight;
Bounce buttonFuel;
Bounce buttonSound;

int getThrothle(void);
buttonAction_t checkButtons(void);
int getPotADC(void);
void calibCheck(void);
void setColorButton(void);
void playSoundButton(void);
void rebfuelButton(void);

void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);
  //initialize train connection

  pinMode(LED_PWR_G, OUTPUT);
  pinMode(LED_CON_B, OUTPUT);
  pinMode(LED_BAT_R, OUTPUT);

  LED_SET(LED_PWR_G, ENABLE_LED);
  LED_SET(LED_CON_B, ENABLE_LED);
  LED_SET(LED_BAT_R, ENABLE_LED);
  delay(100);
  LED_SET(LED_CON_B, DISABLE_LED);
  LED_SET(LED_BAT_R, DISABLE_LED);


  //check if calibration mode enabled - 4 buttons pressed on startup
  calibCheck();

  buttonStop = Bounce();
  buttonStop.attach(BUTTON_STOP_PIN, INPUT_PULLUP);
  buttonStop.interval(50);
  buttonLight = Bounce();
  buttonLight.attach(BUTTON_LIGHT_PIN, INPUT_PULLUP);
  buttonLight.interval(50);
  buttonFuel = Bounce();
  buttonFuel.attach(BUTTON_FUEL_PIN, INPUT_PULLUP);
  buttonFuel.interval(50);
  buttonSound = Bounce();
  buttonSound.attach(BUTTON_SOUND_PIN, INPUT_PULLUP);
  buttonSound.interval(50);

  pinMode(BUTTON_STOP_PIN, INPUT_PULLUP);

  Serial.println("initialize HUB connection");
  myTrainHub.init(2); // initalize the PoweredUpHub instance
} 


// main loop
void loop() {
  int16_t throthle=0;
  int16_t deltaThrothle = 0;

  switch(checkButtons())
  {
    case BUTTON_ACTION_STOP:
      myTrainHub.playSound(3);
      delay(100);
      myTrainHub.playSound(3);
      delay(100);
      trainStopButton = 1;
      trainRolling = 0;
      throthleStop = throthlePrev;
      myTrainHub.stopBasicMotor(port);
      delay(100);
      myTrainHub.stopBasicMotor(port);
      Serial.println("STOP pressed");
      break;
    case BUTTON_ACTION_LIGHT:
      Serial.println("LIGHT pressed");
      setColorButton();
      break;
    case BUTTON_ACTION_FUEL:
      Serial.println("FUEL pressed");
      rebfuelButton();
      break;
    case BUTTON_ACTION_SOUND: 
      Serial.println("SOUND pressed");
      playSoundButton();
      break;
    case BUTTON_ACTION_NONE:
      /*no break*/
    default:
      break;
  }

  getThrothle();

  //in case of loost connection - reinit
  if (!myTrainHub.isConnected() && !myTrainHub.isConnecting()) 
  {
    myTrainHub.init(2);
    //myTrainHub.init("90:84:2b:03:19:7f"); //example of initializing an hub with a specific address
  }

  // connect flow. Search for BLE services and try to connect if the uuid of the hub is found
  if (myTrainHub.isConnecting()) {
    myTrainHub.connectHub();
    if (myTrainHub.isConnected()) {
      Serial.println("Connected to HUB");
      Serial.print("Hub address: ");
      Serial.println(myTrainHub.getHubAddress().toString().c_str());
      Serial.print("Hub name: ");
      Serial.println(myTrainHub.getHubName().c_str());
    } else {
      Serial.println("Failed to connect to HUB");
    }
  }

  // if connected, you can set the name of the hub, the led color and shut it down
  if (myTrainHub.isConnected()) {
    if(++delayCntr > 10){
      delayCntr = 0;

      throthle = getThrothle();
      if(abs(throthleStop - throthle)>30){
        trainStopButton = 0;  //restart train only when throthle moved more than 30%
      }
      throthlePrev = throthle;
      if(!trainStopButton){
        if(throthle > 5 || throthle < -5){ //speed greater than 5%
          myTrainHub.setBasicMotorSpeed(port, throthle);
          trainRolling = 1;
        }else{
          if(trainRolling){
            myTrainHub.stopBasicMotor(port);
            trainRolling = 0;
          }
        }
      }
    }

  //   char hubName[] = "myTrainHub";
  //   myTrainHub.setHubName(hubName);

  // int adcValue = analogRead(ADC_THROTHLE_PIN);
  // Serial.print("ADC Value: ");
  // Serial.println(adcValue);

// myTrainHub.setTachoMotorSpeed(port,50,100,BrakingStyle::FLOAT);
// delay(1000);
// myTrainHub.setTachoMotorSpeed(port,0,100,BrakingStyle::FLOAT);
// delay(1000);

  // myTrainHub.setBasicMotorSpeed(port, 35);
  // delay(1000);
  // myTrainHub.setBasicMotorSpeed(port, 100);
  // delay(1000);
  // myTrainHub.setBasicMotorSpeed(port, 50);
  // delay(1000);
  // myTrainHub.setBasicMotorSpeed(port, -50);
  // delay(1000);C:\!MEGA\Projekty\LegoTrain
  // myTrainHub.setBasicMotorSpeed(port, 0);
  // delay(1000);
    // myTrainHub.setLedColor(GREEN);
    // delay(1000);
    // myTrainHub.setLedColor(RED);

    // myTrainHub.setBasicMotorSpeed(port, -35);
    // delay(1000);
    // myTrainHub.stopBasicMotor(port);
    // delay(1000);

    if(ledBlink < 7){
      LED_SET(LED_CON_B, ENABLE_LED);
    } else {
      LED_SET(LED_CON_B, DISABLE_LED);
    }
  } else {
    Serial.println("Train hub is disconnected");
    delay(100);
  }

  if(++ledBlink > 100){
    ledBlink = 0;
    LED_SET(LED_CON_B, DISABLE_LED);
  }
  delay(10);
  
} // End of loop

void calibCheck(void) {
  //all 4 buttons have to be pressed on startup to calibrate
  //initialize buttons
  uint16_t adcCenter;
  uint16_t adcMin = 0xFFFF;
  uint16_t adcMax = 0;
  pinMode(BUTTON_STOP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_LIGHT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_FUEL_PIN, INPUT_PULLUP);
  pinMode(BUTTON_SOUND_PIN, INPUT_PULLUP);
  delay(50);
  if( !digitalRead(BUTTON_STOP_PIN) && 
      !digitalRead(BUTTON_LIGHT_PIN) &&
      !digitalRead(BUTTON_FUEL_PIN) &&
      !digitalRead(BUTTON_SOUND_PIN)){
        Serial.println("Calibration mode enabled");
          LED_SET(LED_CON_B, ENABLE_LED);
          LED_SET(LED_BAT_R, ENABLE_LED);
          LED_SET(LED_PWR_G, ENABLE_LED);
          while(!digitalRead(BUTTON_STOP_PIN)); //wait for button release
          delay(200);
          LED_SET(LED_CON_B, DISABLE_LED);
          LED_SET(LED_BAT_R, DISABLE_LED);
          LED_SET(LED_PWR_G, DISABLE_LED);

          
          while(digitalRead(BUTTON_STOP_PIN)){
            adcCenter = analogRead(ADC_THROTHLE_PIN);
            delay(10);
          }
          LED_SET(LED_CON_B, ENABLE_LED);
          LED_SET(LED_BAT_R, ENABLE_LED);
          LED_SET(LED_PWR_G, ENABLE_LED);
          delay(500);
          LED_SET(LED_BAT_R, DISABLE_LED);
          LED_SET(LED_PWR_G, DISABLE_LED);
          uint16_t adcRead;
          while(digitalRead(BUTTON_STOP_PIN)){
            adcRead = analogRead(ADC_THROTHLE_PIN);
            if(adcRead > adcMax){
              adcMax = adcRead;
            }
            if(adcRead < adcMin){
              adcMin = adcRead;
            }
            delay(10);
          }
          LED_SET(LED_CON_B, ENABLE_LED);
          LED_SET(LED_BAT_R, ENABLE_LED);
          LED_SET(LED_PWR_G, ENABLE_LED);
          delay(500);
          LED_SET(LED_CON_B, DISABLE_LED);
          LED_SET(LED_BAT_R, DISABLE_LED);
          LED_SET(LED_PWR_G, DISABLE_LED);

          Serial.println("New throthle ADC calibration value");
          Serial.print("Adc Min:");
          Serial.println(adcMin);
          Serial.print("Adc Max:");
          Serial.println(adcMax);
          Serial.print("Adc Center:");
          Serial.println(adcCenter);
          //store data to eeprom
          EEPROM.write(0, adcMin>>8);
          EEPROM.write(1, adcMin&0xFF);
          EEPROM.write(2, adcMax>>8);
          EEPROM.write(3, adcMax&0xFF);
          EEPROM.write(4, adcCenter>>8);
          EEPROM.write(5, adcCenter&0xFF);
          EEPROM.commit();
          Serial.println("Write to EEPROM - done");
      }
  //read adc calibration
  Serial.println("Read throthle ADC calibration data");
  calibAdcMin = EEPROM.read(0)<<8;
  calibAdcMin |= EEPROM.read(1);
  calibAdcMax = EEPROM.read(2)<<8;
  calibAdcMax |= EEPROM.read(3);
  calibAdcCenter = EEPROM.read(4)<<8;
  calibAdcCenter |= EEPROM.read(5);

  Serial.print("Adc Min:");
  Serial.println(calibAdcMin);
  Serial.print("Adc Max:");
  Serial.println(calibAdcMax);
  Serial.print("Adc Center:");
  Serial.println(calibAdcCenter);

  LED_SET(LED_PWR_G, ENABLE_LED);
}

int getPotADC(void) {
  int adcValue = analogRead(ADC_THROTHLE_PIN);
  // Serial.println(adcValue);
  if(adcValue>calibAdcCenter){
    return map(adcValue, calibAdcCenter, calibAdcMax, -5, +100); 
  }else{
    return map(adcValue, calibAdcMin, calibAdcCenter, -100, +5);
  }
}

int getThrothle(void) {
  int throthleOut=0;
  //hardcoded Values for now
  int adcValue = analogRead(ADC_THROTHLE_PIN);
  int inScaledVal = getPotADC();
  if(inScaledVal>-10 && inScaledVal<10){
    throthleOut = 0;
  }else{
    throthleOut = inScaledVal;
  }
  // Serial.println(throthleOut);
  return throthleOut;
}

buttonAction_t checkButtons(void) {
  //scan buttons and check which one was pressed
  buttonStop.update();
  buttonLight.update();
  buttonFuel.update();
  buttonSound.update();

  if(buttonStop.fell()){
    return BUTTON_ACTION_STOP;
  }
  if(buttonLight.fell()){
    return BUTTON_ACTION_LIGHT;
  }
  if(buttonFuel.fell()){
    return BUTTON_ACTION_FUEL;
  }
  if(buttonSound.fell()){
    return BUTTON_ACTION_SOUND;
  }
  return BUTTON_ACTION_NONE;
}

void setColorButton(void){
  static Color actualColor = BLACK;
  actualColor = static_cast<Color>((static_cast<int>(actualColor) + 1) % NUM_COLORS);
  if(actualColor > WHITE){
    actualColor = BLACK;
  }
  Serial.print("Set color");
  Serial.println(actualColor);

  myTrainHub.setLedColor(actualColor);
}

void playSoundButton(void){
  static uint8_t sound = 0;
  Serial.print("play sound");
  myTrainHub.playSound(9);
}

void rebfuelButton(void){
  static uint8_t sound = 0;
  Serial.print("Refuel - play sound:7");
  myTrainHub.playSound(7);
  delay(1500);
  myTrainHub.playSound(7);
  delay(1500);
  myTrainHub.playSound(7);
  delay(1500);
}
//tankowanie sound 7
//sound 9 huhu
//sound 10 mijanie
//sound 3 hamowanie