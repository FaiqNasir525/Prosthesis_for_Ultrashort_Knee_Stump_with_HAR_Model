#include "ml.h"
#include "bmp.h"
#include "mpu.h"
#include <esp_task_wdt.h>

//WaatchDog Setting
#define WDT_TIMEOUT 90

// Enumerations for Modes and Activities
enum Mode { manual, notManual };
enum Activity { idle = 65, walk = 55, fastWalk = 47, run = 40 };

// Pins
const int SDA_1 = 8;
const int SCL_1 = 9;
const int pinInt1 = 5;
const int pinInt2 = 11;
const int pinInt3 = 12;
const int pinInt4 = 13;
const int pinInt5 = 14;
const int pinPump = 19;
const int pinValve = 18;
const int pinBuzzer = 19;

// Activity Variables
volatile Mode mode;
float mpuData[6];

// Pressure parameters
volatile float setPoint;

// Power-saving variables
unsigned long lastTime = 0;
const unsigned long updateInterval = 5000; // Update every 5 seconds

// Interrupts Code
void IRAM_ATTR handleInt1() {
  setPoint = idle;
  mode = manual;
  //regulatePressure(setPoint, pinPump, pinValve);
}

/*void IRAM_ATTR handleInt2() {
  setPoint = walk;
  mode = manual;
  regulatePressure(setPoint, pinPump, pinValve);
}

void IRAM_ATTR handleInt3() {
  setPoint = fastWalk;
  mode = manual;
  regulatePressure(setPoint, pinPump, pinValve);
}

void IRAM_ATTR handleInt4() {
  setPoint = run;
  mode = manual;
  regulatePressure(setPoint, pinPump, pinValve);
  }

void IRAM_ATTR handleInt5() {
  mode = notManual;
}*/

void setup() {
  Serial.begin(115200);

  //Wire.begin(SDA_1, SCL_1);
  // Default values for variables
  setPoint = fastWalk;
  mode = manual;
  
  //WatchDog Setup
  /*esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch

  // Pinmode Setup
  pinMode(pinInt1, INPUT_PULLUP);
  pinMode(pinInt2, INPUT_PULLUP);
  pinMode(pinInt3, INPUT_PULLUP);
  pinMode(pinInt4, INPUT_PULLUP);
  pinMode(pinInt5, INPUT_PULLUP);
  pinMode(pinPump, OUTPUT);
  pinMode(pinValve, OUTPUT);*/

  // Interrupt Setup
  /*attachInterrupt(digitalPinToInterrupt(pinInt1), handleInt1, FALLING);
  attachInterrupt(digitalPinToInterrupt(pinInt2), handleInt2, FALLING);
  attachInterrupt(digitalPinToInterrupt(pinInt3), handleInt3, FALLING);
  attachInterrupt(digitalPinToInterrupt(pinInt4), handleInt4, FALLING);
  attachInterrupt(digitalPinToInterrupt(pinInt5), handleInt5, FALLING);

  // Sensor and Model Setup
  bmp_setup();
   mpu_setup();
   ml_setup();

  // Obtain pointers to the model's input and output tensors.
  input = interpreter->input(0);
  output = interpreter->output(0);
  Serial.println("Pointer Set");*/
}

void loop() {
  /*unsigned long currentTime = millis();
  
  // Auto mode setpoint setting
  if (currentTime - lastTime >= updateInterval && mode == notManual) {
    mpu_reading();
    float mpuData[] = {a.acceleration.x, a.acceleration.y, a.acceleration.z, g.gyro.x, g.gyro.y, g.gyro.z};
    setPoint = inference(mpuData);
  }
  
  if (currentTime - lastTime >= updateInterval){    
    regulatePressure(setPoint, pinPump, pinValve);
    
    lastTime = currentTime;
    esp_task_wdt_reset();
  }*/
  
  regulatePressure(setPoint, pinPump, pinValve);
}

void sensorTesting(){
  //Both Sensor Testing
  int pressure; 
  mpu_reading();
  if(bmp.takeForcedMeasurement()){
    pressure= (bmp.readPressure()/1000);
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
  mpu_reading();
  float mpuData[] = {a.acceleration.x, a.acceleration.y, a.acceleration.z, g.gyro.x, g.gyro.y, g.gyro.z};
  setPoint = inference(mpuData);
  Serial.println(setPoint);
}