#include <Arduino.h>
#include <NimBLEDevice.h>
#include <LegoinoCommon.h>
#include <Lpf2Hub.h>
#include <Bounce2.h>
#include "main.h"

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

Bounce buttonStop;
Bounce buttonLight;
Bounce buttonFuel;
Bounce buttonSound;

int getThrothle(void);
buttonAction_t checkButtons(void);
int getPotADC(void);
void calibCheck(void);

void setup() {
  Serial.begin(115200);
  //initialize train connection

  //initialize buttons
  // pinMode(BUTTON_STOP_PIN, INPUT_PULLUP);
  // pinMode(BUTTON_LIGHT_PIN, INPUT_PULLUP);
  // pinMode(BUTTON_FUEL_PIN, INPUT_PULLUP);
  // pinMode(BUTTON_SOUND_PIN, INPUT_PULLUP);

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
  switch(checkButtons())
  {
    case BUTTON_ACTION_STOP:
      Serial.println("STOP pressed");
      break;
    case BUTTON_ACTION_LIGHT:
      Serial.println("LIGHT pressed");
      break;
    case BUTTON_ACTION_FUEL:
      Serial.println("FUEL pressed");
      break;
    case BUTTON_ACTION_SOUND: 
      Serial.println("SOUND pressed");
      break;
    case BUTTON_ACTION_NONE:
      /*no break*/
    default:
      break;
  }

  getThrothle();

//   //in case of loost connection - reinit
//   if (!myTrainHub.isConnected() && !myTrainHub.isConnecting()) 
//   {
//     myTrainHub.init(2);
//     //myTrainHub.init("90:84:2b:03:19:7f"); //example of initializing an hub with a specific address
//   }

//   // connect flow. Search for BLE services and try to connect if the uuid of the hub is found
//   if (myTrainHub.isConnecting()) {
//     myTrainHub.connectHub();
//     if (myTrainHub.isConnected()) {
//       Serial.println("Connected to HUB");
//       Serial.print("Hub address: ");
//       Serial.println(myTrainHub.getHubAddress().toString().c_str());
//       Serial.print("Hub name: ");
//       Serial.println(myTrainHub.getHubName().c_str());
//     } else {
//       Serial.println("Failed to connect to HUB");
//     }
//   }

//   // if connected, you can set the name of the hub, the led color and shut it down
//   if (myTrainHub.isConnected()) {

//   //   char hubName[] = "myTrainHub";
//   //   myTrainHub.setHubName(hubName);

//   myTrainHub.setBasicMotorSpeed(port, getThrothle());

//   // int adcValue = analogRead(ADC_THROTHLE_PIN);
//   // Serial.print("ADC Value: ");
//   // Serial.println(adcValue);

// // myTrainHub.setTachoMotorSpeed(port,50,100,BrakingStyle::FLOAT);
// // delay(1000);
// // myTrainHub.setTachoMotorSpeed(port,0,100,BrakingStyle::FLOAT);
// // delay(1000);

//   // myTrainHub.setBasicMotorSpeed(port, 35);
//   // delay(1000);
//   // myTrainHub.setBasicMotorSpeed(port, 100);
//   // delay(1000);
//   // myTrainHub.setBasicMotorSpeed(port, 50);
//   // delay(1000);
//   // myTrainHub.setBasicMotorSpeed(port, -50);
//   // delay(1000);C:\!MEGA\Projekty\LegoTrain
//   // myTrainHub.setBasicMotorSpeed(port, 0);
//   // delay(1000);
//     // myTrainHub.setLedColor(GREEN);
//     // delay(1000);
//     // myTrainHub.setLedColor(RED);

//     // myTrainHub.setBasicMotorSpeed(port, -35);
//     // delay(1000);
//     // myTrainHub.stopBasicMotor(port);
//     // delay(1000);

//   } else {
//     Serial.println("Train hub is disconnected");
//     delay(100);
//   }

  delay(200);
  
} // End of loop

void calibCheck(void) {
  //all 4 buttons have to be pressed on startup to calibrate

}

int getPotADC(void) {
  int adcValue = analogRead(ADC_THROTHLE_PIN);
  // Serial.println(adcValue);
  return map(adcValue, 1500, 3000, -100, +100);
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
  Serial.println(throthleOut);
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