#include <util/atomic.h>

// Set pin numbers
#define m1_hallSensorA 2
#define m1_driverPin 5
#define m1_driverIn1 11
#define m1_driverIn2 12
#define m1_encoderPwr 4

#define m2_hallSensorA 3
#define m2_driverPin 6
#define m2_driverIn1 9
#define m2_driverIn2 10
#define m2_encoderPwr 7

// starting speed, 1 is max, 0 is none, anything less than 0.3 doesnt move
#define SPEED 0

// define variables
volatile int m1_sensorHits_i = 0;
volatile int m2_sensorHits_i = 0;

char serialInput[12];
int newData = 0;
int currentIndex = 0;

float m1_lastError = 0;
float m2_lastError = 0;
float kp = 1;
float ki = 1;
float kd = 1;

float m1_setSpeed = SPEED;
float m1_rawVelocity = 0;
float m1_rpm = 0;
int m1_direction = 1;

float m2_setSpeed = SPEED;
float m2_rawVelocity = 0;
float m2_rpm = 0;
int m2_direction = 1;

long lastTime = 0;
int m1_lastSensorHits = 0;
int m2_lastSensorHits = 0;

int currentSelectedMotor = 1;
float *currentSelectedMotorPtr = NULL;

void setup() {
  // Intialize Serial
  Serial.begin(9600);
  // Initalize Pin Modes
  pinMode(m1_hallSensorA, INPUT_PULLUP);
  pinMode(m1_driverIn1, OUTPUT);
  pinMode(m1_driverIn2, OUTPUT);
  pinMode(m1_driverPin, OUTPUT);
  pinMode(m1_encoderPwr, OUTPUT);

  pinMode(m2_hallSensorA, INPUT_PULLUP);
  pinMode(m2_driverIn1, OUTPUT);
  pinMode(m2_driverIn2, OUTPUT);
  pinMode(m2_driverPin, OUTPUT);
  pinMode(m2_encoderPwr, OUTPUT);
  // Power the encoder and set the intial direction
  digitalWrite(m1_driverIn1, HIGH);
  digitalWrite(m1_driverIn2, LOW);
  digitalWrite(m1_encoderPwr, HIGH);
  attachInterrupt(digitalPinToInterrupt(m1_hallSensorA), m1_sensorHit, CHANGE);

  digitalWrite(m2_driverIn1, HIGH);
  digitalWrite(m2_driverIn2, LOW);
  digitalWrite(m2_encoderPwr, HIGH);
  attachInterrupt(digitalPinToInterrupt(m2_hallSensorA), m2_sensorHit, CHANGE);

  analogWrite(m1_driverPin, SPEED*255);
  analogWrite(m2_driverPin, SPEED*255);
}

void loop() {
  // Get the number of sensor hits in non volatile stack
  int m1_sensorHits = 0;
  int m2_sensorHits = 0;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    m1_sensorHits = m1_sensorHits_i;
    m2_sensorHits = m2_sensorHits_i;
  }

  // Calculate rpm
  long currentTime = micros();
  float deltaTime = (float) ((currentTime - lastTime)/1000);
  if (deltaTime > 250) {
    // Calculate current speed
    m1_rawVelocity = (float) ((m1_sensorHits - m1_lastSensorHits)/deltaTime);
    m2_rawVelocity = (float) ((m2_sensorHits - m2_lastSensorHits)/deltaTime);
    lastTime = currentTime;
    m1_lastSensorHits = m1_sensorHits;
    m2_lastSensorHits = m2_sensorHits;
  }
  // This isnt really rpm but I just multiply by 10 to make it less of a decimal
  float m1_rpm = 10*(m1_rawVelocity); 
  float m2_rpm = 10*(m2_rawVelocity); 

  // PID
  if (deltaTime > 250) {
    m1_lastError = m1_error;
    m2_lastError = m2_error;
  }
  float m1_error = (m1_rpm - 10*m1_setSpeed);
  float m1_proportional = m1_error*kp;
  float m1_integral += m1_error*deltaTime*ki;
  float m1_derivative = kd*((m1_lastError - m1_error)/deltaTime);
  float m1_pidOffset = m1_proportional + m1_integral + m1_derivative;

  float m2_error = (m2_rpm - 10*m2_setSpeed);
  float m2_proportional = m2_error*kp;
  float m2_integral += m2_error*deltaTime*ki;
  float m2_derivative = kd*((m2_lastError - m2_error)/deltaTime);
  float m2_pidOffset = m2_proportional + m2_integral + m2_derivative

  // Uncomment to enable pid
  // analogWrite(m1_driverPin, (m1_setSpeed + m1_pidOffset)*255);
  // analogWrite(m2_driverPin, (m2_setSpeed + m2_pidOffset)*255);

  // Get serial input and check commands
  checkSerialData();
  if (newData == 1) {
    switch(serialInput[0]) {
      case 'm':
        if (currentSelectedMotor == 1) {
          currentSelectedMotor = 2;
          Serial.println("Selecting motor 2");
        } else {
          currentSelectedMotor = 1;
          Serial.println("Selecting motor 1");
        }
        break;
      case 's': 
        serialInput[0] = ' ';
        if (currentSelectedMotor == 1)
          setSpeed(&m1_setSpeed, m1_driverPin);
        else
          setSpeed(&m2_setSpeed, m2_driverPin);
        break;
      case 'r':
        if (currentSelectedMotor == 1) 
          rpm(m1_rpm);
        else 
          rpm(m2_rpm);
        break;
      case 't': 
        if (currentSelectedMotor == 1)
          changeDirection(m1_driverIn1, m1_driverIn2, m1_rpm, &m1_direction);
        else 
          changeDirection(m2_driverIn1, m2_driverIn2, m2_rpm, &m2_direction);
        break;
      case 'p':
        Serial.println("Pong!");
        Serial.println(m1_sensorHits);
        Serial.println(m2_sensorHits);
        break;
      case 'e':
        Serial.println(m1_error);
        break;
      default:
        Serial.print("ERROR: Invalid Command: ");
        Serial.println(serialInput[0]);
    }
    clearSerialData();
  }
}

void changeDirection(int driverIn1, int driverIn2, float rpm, int* direction) {
  if (rpm <= 0.1) { 
    Serial.println("Changing direction");
    if (*direction == 1) {
        digitalWrite(driverIn1, LOW);
        digitalWrite(driverIn2, HIGH);
        *direction = 0;
    } else {
        digitalWrite(driverIn1, HIGH);
        digitalWrite(driverIn2, LOW);
        *direction = 1;
    }
  } else {
    Serial.println("ERROR: Stop motor before changing direction");
  }
}

void setSpeed(float *speed, int driverPin) {
  *speed = atof(serialInput);

  if (*speed < 0) {
    Serial.println("ERROR: Speed below 0, setting to 0");
    *speed = 0;
  } else if (*speed > 1) {
    Serial.println("ERROR: Speed above 1, setting to 1");
    *speed = 1;
  } else if (*speed < 0.3 && *speed != 0) {
    Serial.println("ERROR: Speed below 0.3, setting to 0.3");
    *speed= 0.3;
  }

  if (*speed<= 0.4 && *speed!= 0) {
    analogWrite(driverPin, 0.5*255);
    delay(50);
  }
  Serial.print("Setting speed to: ");
  Serial.println(*speed);
  analogWrite(driverPin, *speed*255);
}

void rpm(float rpm) {
  Serial.print("Current rpm: ");
  Serial.println(rpm);
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

void m1_sensorHit() {
  m1_sensorHits_i++;
}

void m2_sensorHit() {
  m2_sensorHits_i++;
}
