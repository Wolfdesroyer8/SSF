#include <util/atomic.h>

// Set pin numbers
#define hallSensorA 2
#define driverPin 5
#define driverIn1 11
#define driverIn2 12
#define encoderPwr 6

// starting speed, 1 is max, 0 is none, anything less than 0.3 doesnt move
#define SPEED 0

// define variables
volatile int sensorHits_i = 0;

char serialInput[12];
int newData = 0;
int currentIndex = 0;

float currentSpeed = SPEED;
float rawVelocity = 0;
float rpm = 0;
int direction = 1;

long lastTime = 0;
int lastSensorHits = 0;

void setup() {
  // Intialize Serial
  Serial.begin(9600);
  // Initalize Pin Modes
  pinMode(hallSensorA, INPUT_PULLUP);
  pinMode(driverIn1, OUTPUT);
  pinMode(driverIn2, OUTPUT);
  pinMode(driverPin, OUTPUT);
  pinMode(encoderPwr, OUTPUT);
  // Power the encoder and set the intial direction
  digitalWrite(driverIn1, HIGH);
  digitalWrite(driverIn2, LOW);
  digitalWrite(encoderPwr, HIGH);
  attachInterrupt(digitalPinToInterrupt(hallSensorA), sensorHit, CHANGE);

  analogWrite(driverPin, SPEED*255);
}

void loop() {
  // Get the number of sensor hits in non volatile stack
  int sensorHits = 0;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    sensorHits = sensorHits_i;
  }

  // Calculate rpm
  long currentTime = micros();
  float deltaTime = (float) ((currentTime - lastTime)/1000);
  if (deltaTime > 250) {
    rawVelocity = (float) ((sensorHits - lastSensorHits)/deltaTime);
    lastTime = currentTime;
    lastSensorHits = sensorHits;
  }
  // This isnt really rpm but I just multiply by 10 to make it not a decimal
  float rpm = 10*(rawVelocity); 

  // Get serial input and check commands
  checkSerialData();
  if (newData == 1) {
    switch(serialInput[0]) {
      case 's': 
        serialInput[0] = ' ';
        currentSpeed = atof(serialInput);

        if (currentSpeed < 0) {
          Serial.println("ERROR: Speed below 0, setting to 0");
          currentSpeed = 0;
        } else if (currentSpeed > 1) {
          Serial.println("ERROR: Speed above 1, setting to 1");
          currentSpeed = 1;
        } else if (currentSpeed < 0.3 && currentSpeed != 0) {
          Serial.println("ERROR: Speed below 0.3, setting to 0.3");
          currentSpeed = 0.3;
        }

        if (currentSpeed <= 0.4 && currentSpeed != 0) {
          analogWrite(driverPin, 0.5*255);
          delay(50);
        }
        Serial.print("Setting speed to: ");
        Serial.println(currentSpeed);
        analogWrite(driverPin, currentSpeed*255);
        break;
      case 'r':
        Serial.print("Current rpm: ");
        Serial.println(rpm);
        break;
      case 't': 
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
          break;
      case 'p':
        Serial.println("Pong!");
        break;
      default:
        Serial.print("ERROR: Invalid Command: ");
        Serial.println(serialInput[0]);
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
