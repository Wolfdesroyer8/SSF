#include <util/atomic.h>

// Set pin numbers
#define hallSensorA 2
#define hallSensorB 3
#define driverPin 5
#define driverIn1 11
#define driverIn2 12
#define encoderPwr 6

// starting speed, 1 is max, 0 is none // NOTE: anything less than 0.4 doesnt move
#define SPEED 0

// define variables
volatile int sensorHits_i = 0;
const int encoderHitsPerRotation = 6;

char serialInput[12];
int newData = 0;
int currentIndex = 0;

float currentSpeed = SPEED;
float rawVelocity = 0;
float rpm = 0;

long lastTime = 0;
int lastSensorHits = 0;

int IstartingSensorHits = 0;
int IendingSensorHits = 0;
int Ienabled = 0;

int direction = 1;

void setup() {
  // Intialize Serial
  Serial.begin(9600);
  // Initalize Pin Modes
  pinMode(hallSensorA, INPUT_PULLUP);
  pinMode(hallSensorB, INPUT_PULLUP);
  pinMode(driverIn1, OUTPUT);
  pinMode(driverIn2, OUTPUT);
  pinMode(driverPin, OUTPUT);
  pinMode(encoderPwr, OUTPUT);
  digitalWrite(driverIn1, HIGH);
  digitalWrite(driverIn2, LOW);
  digitalWrite(encoderPwr, HIGH);
  attachInterrupt(digitalPinToInterrupt(hallSensorA), sensorHit, CHANGE);
  // attachInterrupt(digitalPinToInterrupt(hallSensorB), sensorHit, CHANGE);

  analogWrite(driverPin, SPEED*255);
}

void loop() {
  int sensorHits = 0;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    sensorHits = sensorHits_i;
  }

  long currentTime = micros();
  float deltaTime = (float) ((currentTime - lastTime)/1000);
  if (deltaTime > 1000) {
    rawVelocity = (float) ((sensorHits - lastSensorHits)/deltaTime);
    lastTime = currentTime;
    lastSensorHits = sensorHits;
  }

  float rpm = 60*(rawVelocity/encoderHitsPerRotation);

  checkSerialData();

  if (newData == 1) {
    if (serialInput[0] == 's') {
      serialInput[0] = ' ';
      currentSpeed = atof(serialInput);

      if (currentSpeed < 0) {
        Serial.println("ERROR: Speed below 0, setting to 0");
        currentSpeed = 0;
      } else if (currentSpeed > 1) {
        Serial.println("ERROR: Speed above 1, setting to 1");
        currentSpeed = 1;
      }

      Serial.print("Setting speed to: ");
      Serial.println(currentSpeed);
      analogWrite(driverPin, currentSpeed*255);
    } else if (serialInput[0] == 'r') {
      Serial.print("Current rpm: ");
      Serial.println(rpm);
    } else if (serialInput[0] == 'd') {
      Serial.print("Current sensor hits: ");
      Serial.println(sensorHits);
      Serial.print("Last sensor hits: ");
      Serial.println(lastSensorHits);
      Serial.print("Delta Time: ");
      Serial.println(deltaTime);
      Serial.print("Current Time: ");
      Serial.println(currentTime);
      Serial.print("Last Time: ");
      Serial.println(lastTime);
    } else if (serialInput[0] == 'i') {
      if (Ienabled == 0) {
        Ienabled = 1;
        analogWrite(driverPin, 0);
        Serial.println("Input \"i\" when one rotation is made");
        IstartingSensorHits = sensorHits;
        analogWrite(driverPin, 0.5*255);
        delay(50);
        analogWrite(driverPin, 0.2*255);
      } else {
        Ienabled = 0;
        IendingSensorHits = sensorHits;
        analogWrite(driverPin, 0);
        Serial.print("There was ");
        Serial.print(IendingSensorHits - IstartingSensorHits);
        Serial.println(" sensor hits in one rotation");
      }
    } else if (serialInput[0] == 't') {
        if (rpm <= 0.1) { 
          Serial.println("Changing direction");
          if (direction == 1) {
              digitalWrite(driverIn1, LOW);
              digitalWrite(driverIn2, HIGH);
              direction = 0;
          } else {
              digitalWrite(driverIn1, HIGH);
              digitalWrite(driverIn2, LOW);
              direction = 1;
          }
        } else {
          Serial.println("ERROR: Stop motor before changing direction");
        }
    } else {
      Serial.println("ERROR: Invalid Command");
    }
    clearSerialData();
  }
}

void checkSerialData() {
  if (Serial.available() > 0) {
    char current = Serial.read();
    if (current == '\n' || current == '\r') {
      newData = 1;
      serialInput[currentIndex] = '\0';
      currentIndex = 0;
    } else {
      serialInput[currentIndex] = current;
      currentIndex++;
    }
  }
}

void clearSerialData() {
  for (int i = 0; i < sizeof(serialInput)/sizeof(serialInput[0]); i++)
    serialInput[i] = '\0';
  newData = 0;
}

void sensorHit() {
  sensorHits_i++;
}
