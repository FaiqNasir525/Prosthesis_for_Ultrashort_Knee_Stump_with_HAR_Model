#include "bmp.h"
#include <esp_task_wdt.h>
#include "ml_mpu.h"

//WaatchDog Setting
#define WDT_TIMEOUT 90

//Interrupt Variables
bool finish;
bool act;

//debounce variables
#define DEBOUNCE_DELAY 500
volatile unsigned long lastInterruptTime = 0;

// Enumerations for Modes and Activities
enum Mode { manual, automatic };
enum Activity { idle = 20, walk = 15, run = 10 };

// Pins
//const int SDA_1 = 8;
//const int SCL_1 = 9;
const int pinInt1 = 4;
const int pinInt2 = 16;  
const int pinInt3 = 17;
const int pinInt4 = 5;
//const int pinInt5 = 14;
const int pinPump = 19;
const int pinValve = 18;
const int pinBuzzer= 37;

// Activity Variables
volatile Mode mode;

// Pressure parameters
volatile float setPoint;
float prevSet;

// Power-saving variables
unsigned long lastTime = 0;
const unsigned long updateInterval = 5000; // Update every 5 seconds

// Interrupts Code
void IRAM_ATTR handleInt1() {
  unsigned long interruptTime = millis();
  if (interruptTime - lastInterruptTime > DEBOUNCE_DELAY) {
    setPoint = idle;
    mode = manual;
    act= 1;
    lastInterruptTime = interruptTime;
  }
}

void IRAM_ATTR handleInt2() {
  unsigned long interruptTime = millis();
  if (interruptTime - lastInterruptTime > DEBOUNCE_DELAY) {
    setPoint = walk;
    mode = manual;
    act= 1;
    lastInterruptTime = interruptTime;
  }
}


void IRAM_ATTR handleInt3() {
  unsigned long interruptTime = millis();
  if (interruptTime - lastInterruptTime > DEBOUNCE_DELAY) {
    mode = automatic;
    act= 1;
    lastInterruptTime = interruptTime;
  }
}

void IRAM_ATTR handleInt4() {
  unsigned long interruptTime = millis();
  if (interruptTime - lastInterruptTime > DEBOUNCE_DELAY) {
    mode = manual;
    finish= 1;
    lastInterruptTime = interruptTime;
  }
}


void setup() {
  Serial.begin(115200);

  //Wire.begin(SDA_1, SCL_1);

  // Default values for variables
  setPoint = idle;
  mode = manual;
  prevSet= 0;
  finish= 0;
  act= 0;
  
  //WatchDog Setup
  esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch

  // Pinmode Setup
  pinMode(pinInt1, INPUT_PULLUP);
  pinMode(pinInt2, INPUT_PULLUP);
  pinMode(pinInt3, INPUT_PULLUP);
  pinMode(pinInt4, INPUT_PULLUP);
  //pinMode(pinInt5, INPUT_PULLUP);
  pinMode(pinPump, OUTPUT);
  pinMode(pinValve, OUTPUT);

  // Interrupt Setup
  attachInterrupt(digitalPinToInterrupt(pinInt1), handleInt1, FALLING);
  attachInterrupt(digitalPinToInterrupt(pinInt2), handleInt2, FALLING);
  attachInterrupt(digitalPinToInterrupt(pinInt3), handleInt3, FALLING);
  attachInterrupt(digitalPinToInterrupt(pinInt4), handleInt4, FALLING);
  //attachInterrupt(digitalPinToInterrupt(pinInt5), handleInt5, FALLING);

  // Sensor and Model Setup
  mpu_setup();
  bmp_setup();
  ml_setup();

}


void loop() {
  unsigned long currentTime = millis();
  
  if (act){
    Serial.println("Interrupt Activated");
    if (mode== automatic){
      setPoint= inference();
    }
    regulatePressure(setPoint, pinPump, pinValve);
    act= 0; 
  }
  
  if (finish){
    digitalWrite(pinPump,LOW);
    digitalWrite(pinValve,HIGH);
    while (1){
      delay(100);
    }
  }

  // Auto mode setpoint setting
  if (currentTime - lastTime >= updateInterval && mode == automatic) {
    sample_mpu();
    float set= inference();
    if (set==prevSet){
      setPoint= set;
    }
    prevSet= set;
  }
  
  if (currentTime - lastTime >= updateInterval){    
    regulatePressure(setPoint, pinPump, pinValve);
    
    lastTime = currentTime;
  }
  esp_task_wdt_reset();
}



void sensorTesting(){
  //Both Sensor Testing
  float pressure; 
  mpu_reading();
  if(bmp.takeForcedMeasurement()){
    pressure= (bmp.readPressure()/3386);
  }
  Serial.print(a.acceleration.x);
  Serial.print(","); Serial.print(a.acceleration.y);
  Serial.print(","); Serial.print(a.acceleration.z);
  Serial.print(","); Serial.print(g.gyro.x);
  Serial.print(","); Serial.print(g.gyro.y);
  Serial.print(","); Serial.print(g.gyro.z);
  Serial.println();
  Serial.println(pressure);
}

void inferenceTest(){
  //Testing for Inference
  sample_mpu();
  setPoint = inference();
  print_prediction();
  Serial.println(setPoint);
  delay(2000);
}