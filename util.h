#include "config.h"
#include <math.h> 

// PINs
#define RC_PIN PB12
#define INTERIOR_LIGHT_RELAY_PIN PA7
#define OPEN_CLOSE_BUTTON_PIN PB3

#define LEFT_MOTOR_COUNTER_PIN PB9
#define RIGHT_MOTOR_COUNTER_PIN PB8

#define LEFT_DOOR_OPEN_PIN PB11     // Normal Open
#define LEFT_DOOR_CLOSE_PIN PB10    // Normal Open
#define RIGHT_DOOR_OPEN_PIN PA11    // Normal Open
#define RIGHT_DOOR_CLOSE_PIN PA12   // Normal Open


#define RELAY_12_36_PIN PA6
#define MOTOR_ACTUATOR_RELAY_PIN PA5

#define OBSTACLE_LAMP_PIN PA4
#define SIGNAL_LAMP_PIN PA3

#define OPEN_LEFT_MOTOR_ACTUATOR_RELAY_PIN PC15
#define CLOSE_LEFT_MOTOR_ACTUATOR_RELAY_PIN PA0
#define OPEN_RIGHT_MOTOR_ACTUATOR_RELAY_PIN PA1
#define CLOSE_RIGHT_MOTOR_ACTUATOR_RELAY_PIN PA2

#define BUZZER_PIN PC14 // Use PC14 for production

// Request Command
#define REQUEST_COMMAND_UNKNOWN -1
#define REQUEST_COMMAND_TO_OPEN 0
#define REQUEST_COMMAND_TO_CLOSE 1
#define REQUEST_COMMAND_TO_OPEN_LEFT_DOOR 2
#define REQUEST_COMMAND_TO_STOP 3

char displayLine1[22];
char displayLine2[22];
char displayLine3[22];
char displayLine4[22];
char displayLine5[22];
char displayLine6[22];
char displayLine7[22];
char displayLine8[22];

#define DEBUG_PRINT_LN(str)                               \
   Serial.print(millis());                                \
   Serial.print(": ");                                    \
   Serial.println(str);                                   \
   strncpy(displayLine1, displayLine2, 21);               \
   strncpy(displayLine2, displayLine3, 21);               \
   strncpy(displayLine3, displayLine4, 21);               \
   strncpy(displayLine4, displayLine5, 21);               \
   strncpy(displayLine5, displayLine6, 21);               \
   strncpy(displayLine6, displayLine7, 21);               \
   strncpy(displayLine7, displayLine8, 21);               \
   strncpy(displayLine8, str, 21);

#define DEBUG_PRINT(str)                                                     \
   Serial.print(str);                                                        \
   strncpy(displayLine8, str, 21);

#define DEBUG_PRINT_NUMBER(number)                                           \
   Serial.print(number);                                                     \
   char displayLineWithNumbers[22];                                          \
   sprintf(displayLineWithNumbers, "%d", number);                            \
   size_t l = strlen(displayLine8);                                          \
   strncat(displayLine8, displayLineWithNumbers, 21 - l);
   
#define DEBUG_PRINT(str)       \
   Serial.print(millis());     \
   Serial.print(": ");         \
   Serial.print(str);


unsigned long currentMillis = 0;


// Variables to handle changes in Request commands
int8_t currentRequestCommand = REQUEST_COMMAND_UNKNOWN;
int8_t previousRequestCommand = currentRequestCommand;
int8_t opositeRequestCommand = REQUEST_COMMAND_TO_OPEN;

char* getRequestCommandName() {
  static char buf[14];
  if (currentRequestCommand == REQUEST_COMMAND_TO_OPEN) {
    strcpy(buf, "Open");
  } else if (currentRequestCommand == REQUEST_COMMAND_TO_CLOSE) {
    strcpy(buf, "Close");
  } else if (currentRequestCommand == REQUEST_COMMAND_TO_OPEN_LEFT_DOOR) {
    strcpy(buf, "Open Left Door");
  } else if (currentRequestCommand == REQUEST_COMMAND_TO_STOP) {
    strcpy(buf, "Stop Doors");
  } else {
    strcpy(buf, "Unknown");
  }
  return buf;
}

// ==================================================================================================================

bool isLeftDoorOpening() { return digitalRead(MOTOR_ACTUATOR_RELAY_PIN) == LOW && digitalRead(OPEN_LEFT_MOTOR_ACTUATOR_RELAY_PIN) == HIGH;    }
bool isLeftDoorClosing() { return digitalRead(MOTOR_ACTUATOR_RELAY_PIN) == LOW && digitalRead(CLOSE_LEFT_MOTOR_ACTUATOR_RELAY_PIN) == HIGH;   }
bool isRightDoorOpening() { return digitalRead(MOTOR_ACTUATOR_RELAY_PIN) == LOW && digitalRead(OPEN_RIGHT_MOTOR_ACTUATOR_RELAY_PIN) == HIGH;  }
bool isRightDoorClosing() { return digitalRead(MOTOR_ACTUATOR_RELAY_PIN) == LOW && digitalRead(CLOSE_RIGHT_MOTOR_ACTUATOR_RELAY_PIN) == HIGH; }

bool isLeftDoorOpened() { return digitalRead(LEFT_DOOR_OPEN_PIN) == LOW;   }
bool isLeftDoorClosed() { return digitalRead(LEFT_DOOR_CLOSE_PIN) == LOW;  }
bool isRightDoorOpened() { return digitalRead(RIGHT_DOOR_OPEN_PIN) == LOW; }
bool isRightDoorClosed() { return digitalRead(RIGHT_DOOR_CLOSE_PIN) == LOW; }

// ==================================================================================================================

bool canOpenLeftDoor() {
  if (isLeftDoorOpening() && !isLeftDoorOpened()) { DEBUG_PRINT_LN("L Door is opening"); }
  if (isLeftDoorOpened()) { DEBUG_PRINT_LN("L Door is opened"); }
  return !isLeftDoorOpening() && !isLeftDoorOpened();
}

bool canCloseLeftDoor() {
  if (isLeftDoorClosing() && !isLeftDoorClosed()) { DEBUG_PRINT_LN("L Door is closing"); }
  if (isLeftDoorClosed()) { DEBUG_PRINT_LN("L Door is closed"); }
  return !isLeftDoorClosing() && !isLeftDoorClosed();
}

bool canOpenRightDoor() {
  if (isRightDoorOpening() && !isRightDoorOpened()) { DEBUG_PRINT_LN("R Door is opening"); }
  if (isRightDoorOpened()) { DEBUG_PRINT_LN("R Door is opened"); }
  return !isRightDoorOpening() && !isRightDoorOpened();
}

bool canCloseRightDoor() {
  if (isRightDoorClosing() && !isRightDoorClosed()) { DEBUG_PRINT_LN("R Door is closing"); }
  if (isRightDoorClosed()) { DEBUG_PRINT_LN("R Door is closed"); }
  return !isRightDoorClosing() && !isRightDoorClosed();
}

// ==================================================================================================================

// Variables to react on Request Command and handle lock/unlock doors
unsigned long unlockLeftDoorStartMillis = 0;
unsigned long unlockLeftDoor1StartMillis = 0;
unsigned long unlockLeftDoor2StartMillis = 0;
unsigned long unlockRightDoorStartMillis = 0;
unsigned long unlockRightDoor1StartMillis = 0;
unsigned long unlockRightDoor2StartMillis = 0;
unsigned long lockLeftDoorStartMillis = 0;
unsigned long lockLeftDoor1StartMillis = 0;
unsigned long lockLeftDoor2StartMillis = 0;
unsigned long lockRightDoorStartMillis = 0;
unsigned long lockRightDoor1StartMillis = 0;
unsigned long lockRightDoor2StartMillis = 0;

void handleLookUnlockCommands() {
  if (unlockLeftDoorStartMillis != 0 && currentMillis >= unlockLeftDoorStartMillis) {
    DEBUG_PRINT_LN("Unlock L Door Start");
    digitalWrite(RELAY_12_36_PIN, LOW); // Ensure we are at 12v
    digitalWrite(MOTOR_ACTUATOR_RELAY_PIN, HIGH); // Switch to Actuators output
    digitalWrite(CLOSE_LEFT_MOTOR_ACTUATOR_RELAY_PIN, LOW); // Both actuators should not be closing while we want to open
    digitalWrite(CLOSE_RIGHT_MOTOR_ACTUATOR_RELAY_PIN, LOW); // Both actuators should not be closing while we want to open
    unlockLeftDoorStartMillis = 0;
    unlockLeftDoor1StartMillis = currentMillis;
  } else if (unlockLeftDoor1StartMillis != 0 && currentMillis >= unlockLeftDoor1StartMillis + TIME_TO_RELAY_SWITCH) { // Need to wait until main relay will do the switch
    digitalWrite(OPEN_LEFT_MOTOR_ACTUATOR_RELAY_PIN, HIGH); // Open Left Door
    unlockLeftDoor1StartMillis = 0;
    unlockLeftDoor2StartMillis = currentMillis;
  } else if (unlockLeftDoor2StartMillis != 0 && currentMillis >= unlockLeftDoor2StartMillis + TIME_TO_LOCK_UNLOCK_DOORS) {
    digitalWrite(OPEN_LEFT_MOTOR_ACTUATOR_RELAY_PIN, LOW);
    digitalWrite(MOTOR_ACTUATOR_RELAY_PIN, LOW); // Switch back to Motors
    unlockLeftDoor2StartMillis = 0;
    DEBUG_PRINT_LN("Unlock L Door: Finished");
  }

  if (lockLeftDoorStartMillis != 0 && currentMillis >= lockLeftDoorStartMillis) {
    DEBUG_PRINT_LN("Lock L Door: Start");
    digitalWrite(RELAY_12_36_PIN, LOW); // Ensure we are at 12v
    digitalWrite(MOTOR_ACTUATOR_RELAY_PIN, HIGH); // Switch to Actuators output
    digitalWrite(OPEN_LEFT_MOTOR_ACTUATOR_RELAY_PIN, LOW); // Both actuators should not be opening while we want to close
    digitalWrite(OPEN_RIGHT_MOTOR_ACTUATOR_RELAY_PIN, LOW); // Both actuators should not be opening while we want to close
    lockLeftDoorStartMillis = 0;
    lockLeftDoor1StartMillis = currentMillis;
  } else if (lockLeftDoor1StartMillis != 0 && currentMillis >= lockLeftDoor1StartMillis + TIME_TO_RELAY_SWITCH) { // Need to wait until main relay will do the switch
    digitalWrite(CLOSE_LEFT_MOTOR_ACTUATOR_RELAY_PIN, HIGH); // Close Left Door
    lockLeftDoor1StartMillis = 0;
    lockLeftDoor2StartMillis = currentMillis;
  } else if (lockLeftDoor2StartMillis != 0 && currentMillis >= lockLeftDoor2StartMillis + TIME_TO_LOCK_UNLOCK_DOORS) {
    digitalWrite(CLOSE_LEFT_MOTOR_ACTUATOR_RELAY_PIN, LOW);
    digitalWrite(MOTOR_ACTUATOR_RELAY_PIN, LOW); // Switch back to Motors
    lockLeftDoor2StartMillis = 0;
    DEBUG_PRINT_LN("Lock L Door: Finished");
  }

  if (unlockRightDoorStartMillis != 0 && currentMillis >= unlockRightDoorStartMillis) {
    DEBUG_PRINT_LN("Unlock R Door: Start");
    digitalWrite(RELAY_12_36_PIN, LOW); // Ensure we are at 12v
    digitalWrite(MOTOR_ACTUATOR_RELAY_PIN, HIGH); // Switch to Actuators output
    digitalWrite(CLOSE_LEFT_MOTOR_ACTUATOR_RELAY_PIN, LOW); // Both actuators should not be closing while we want to open
    digitalWrite(CLOSE_RIGHT_MOTOR_ACTUATOR_RELAY_PIN, LOW); // Both actuators should not be closing while we want to open
    unlockRightDoorStartMillis = 0;
    unlockRightDoor1StartMillis = currentMillis;
  } else if (unlockRightDoor1StartMillis != 0 && currentMillis >= unlockRightDoor1StartMillis + TIME_TO_RELAY_SWITCH) { // Need to wait until main relay will do the switch
    digitalWrite(OPEN_RIGHT_MOTOR_ACTUATOR_RELAY_PIN, HIGH); // Open Right Door
    unlockRightDoor1StartMillis = 0;
    unlockRightDoor2StartMillis = currentMillis;
  } else if (unlockRightDoor2StartMillis != 0 && currentMillis >= unlockRightDoor2StartMillis + TIME_TO_LOCK_UNLOCK_DOORS) {
    digitalWrite(OPEN_RIGHT_MOTOR_ACTUATOR_RELAY_PIN, LOW);
    digitalWrite(MOTOR_ACTUATOR_RELAY_PIN, LOW); // Switch back to Motors
    unlockRightDoor2StartMillis = 0;
    DEBUG_PRINT_LN("Unlock R Door: Finished");
  }
  
  if (lockRightDoorStartMillis != 0 && currentMillis >= lockRightDoorStartMillis) {
    DEBUG_PRINT_LN("Lock R Door: Start");
    digitalWrite(RELAY_12_36_PIN, LOW); // Ensure we are at 12v
    digitalWrite(MOTOR_ACTUATOR_RELAY_PIN, HIGH); // Switch to Actuators output
    digitalWrite(OPEN_LEFT_MOTOR_ACTUATOR_RELAY_PIN, LOW); // Both actuators should not be opening while we want to close
    digitalWrite(OPEN_RIGHT_MOTOR_ACTUATOR_RELAY_PIN, LOW); // Both actuators should not be opening while we want to close
    lockRightDoorStartMillis = 0;
    lockRightDoor1StartMillis = currentMillis;
  } else if (lockRightDoor1StartMillis != 0 && currentMillis >= lockRightDoor1StartMillis + TIME_TO_RELAY_SWITCH) {
    digitalWrite(CLOSE_RIGHT_MOTOR_ACTUATOR_RELAY_PIN, HIGH); // Close Right Door
    lockRightDoor1StartMillis = 0;
    lockRightDoor2StartMillis = currentMillis;
  } else if (lockRightDoor2StartMillis != 0 && currentMillis >= lockRightDoor2StartMillis + TIME_TO_LOCK_UNLOCK_DOORS) {
    digitalWrite(CLOSE_RIGHT_MOTOR_ACTUATOR_RELAY_PIN, LOW);
    lockRightDoor2StartMillis = 0;
    DEBUG_PRINT_LN("Lock R Door: Finished");
  }  
}

// ==================================================================================================================

long leftMotorCounter = 0;
long rightMotorCounter = 0;
uint8_t previousLeftMotorCounterPinState = HIGH;
uint8_t previousRightMotorCounterPinState = HIGH;
void handleMotorCounters() {
  uint8_t currentLeftMotorCounterPinState = digitalRead(LEFT_MOTOR_COUNTER_PIN);
  if (currentLeftMotorCounterPinState != previousLeftMotorCounterPinState && previousLeftMotorCounterPinState == LOW) {
    if (isLeftDoorOpening()) {
      leftMotorCounter++;
    } else if (isLeftDoorClosing()) {
      leftMotorCounter--;
    }
  }
  previousLeftMotorCounterPinState = currentLeftMotorCounterPinState;

  uint8_t currentRightMotorCounterPinState = digitalRead(RIGHT_MOTOR_COUNTER_PIN);
  if (currentRightMotorCounterPinState != previousRightMotorCounterPinState && previousRightMotorCounterPinState == LOW) {
    if (isRightDoorOpening()) {
      rightMotorCounter++;
    } else if (isRightDoorClosing()) {
      rightMotorCounter--;
    }
  }
  previousRightMotorCounterPinState = currentRightMotorCounterPinState;
}

// ==================================================================================================================

// Variables to react on Request Command and handle doors oppening
unsigned long openLeftDoorStartMillis = 0;
unsigned long closeLeftDoorStartMillis = 0;
unsigned long openRightDoorStartMillis = 0;
unsigned long closeRightDoorStartMillis = 0;

unsigned long numberOfRightMotorRevolutionAfterStartClosingLeftDoor = 0;
unsigned long numberOfLeftMotorRevolutionAfterStartOpeningRightDoor = 0;

void handleDoorCommands() {
  // Open Left Door
  if (openLeftDoorStartMillis != 0 && currentMillis >= openLeftDoorStartMillis) {
    DEBUG_PRINT_LN("L Door Opening: Start");
    digitalWrite(RELAY_12_36_PIN, LOW); // Ensure we are at 12v
    digitalWrite(MOTOR_ACTUATOR_RELAY_PIN, LOW); // Relay shoyld be in position for motors
    digitalWrite(CLOSE_LEFT_MOTOR_ACTUATOR_RELAY_PIN, LOW); // Both door should not be clossing while we want to open
    digitalWrite(CLOSE_RIGHT_MOTOR_ACTUATOR_RELAY_PIN, LOW); // Both door should not be clossing while we want to open
    digitalWrite(OPEN_LEFT_MOTOR_ACTUATOR_RELAY_PIN, HIGH);
    openLeftDoorStartMillis = 0;
  }

  // Close Left Door
  if (closeLeftDoorStartMillis != 0 && 
      currentMillis >= closeLeftDoorStartMillis && 
      (abs(rightMotorCounter) >= NUMBER_OF_RIGHT_MOTOR_REVOLUTIONS_AFTER_START_CLOSE_LEFT_DOOR || isRightDoorClosed())) {
        
    DEBUG_PRINT_LN("L Door Closing: Start");
    digitalWrite(RELAY_12_36_PIN, LOW); // Ensure we are at 12v
    digitalWrite(MOTOR_ACTUATOR_RELAY_PIN, LOW); // Relay shoyld be in position for motors
    digitalWrite(OPEN_LEFT_MOTOR_ACTUATOR_RELAY_PIN, LOW); // Both door should not be opening while we want to close
    digitalWrite(OPEN_RIGHT_MOTOR_ACTUATOR_RELAY_PIN, LOW); // Both door should not be opening while we want to close
    digitalWrite(CLOSE_LEFT_MOTOR_ACTUATOR_RELAY_PIN, HIGH);
    closeLeftDoorStartMillis = 0;
  }

  // Open Right Door
  if (openRightDoorStartMillis != 0 && currentMillis >= openRightDoorStartMillis && 
      (abs(leftMotorCounter) >= NUMBER_OF_RIGHT_MOTOR_REVOLUTIONS_AFTER_START_CLOSE_LEFT_DOOR || isLeftDoorOpened())) {

    DEBUG_PRINT_LN("R Door Opening: Start");
    digitalWrite(RELAY_12_36_PIN, LOW); // Ensure we are at 12v
    digitalWrite(MOTOR_ACTUATOR_RELAY_PIN, LOW); // Relay shoyld be in position for motors
    digitalWrite(CLOSE_LEFT_MOTOR_ACTUATOR_RELAY_PIN, LOW); // Both door should not be clossing while we want to open
    digitalWrite(CLOSE_RIGHT_MOTOR_ACTUATOR_RELAY_PIN, LOW); // Both door should not be clossing while we want to open
    digitalWrite(OPEN_RIGHT_MOTOR_ACTUATOR_RELAY_PIN, HIGH);
    openRightDoorStartMillis = 0;
  }

  // Close Right Door
  if (closeRightDoorStartMillis != 0 && currentMillis >= closeRightDoorStartMillis) {
    DEBUG_PRINT_LN("R Door Closing: Start");
    digitalWrite(RELAY_12_36_PIN, LOW); // Ensure we are at 12v
    digitalWrite(MOTOR_ACTUATOR_RELAY_PIN, LOW); // Relay shoyld be in position for motors
    digitalWrite(OPEN_LEFT_MOTOR_ACTUATOR_RELAY_PIN, LOW); // Both door should not be opening while we want to close
    digitalWrite(OPEN_RIGHT_MOTOR_ACTUATOR_RELAY_PIN, LOW); // Both door should not be opening while we want to close
    digitalWrite(CLOSE_RIGHT_MOTOR_ACTUATOR_RELAY_PIN, HIGH);
    closeRightDoorStartMillis = 0;
  } 
  
}

// ==================================================================================================================

unsigned long leftMotorCounterPositionToSpeedUp = 0;
unsigned long leftMotorCounterPositionToSpeedDown = 0;
unsigned long rightMotorCounterPositionToSpeedUp = 0;
unsigned long rightMotorCounterPositionToSpeedDown = 0;
void handleChangeMotorSpeed() {
  if (digitalRead(RELAY_12_36_PIN) == LOW &&
      abs(leftMotorCounter) >= leftMotorCounterPositionToSpeedUp && 
      abs(leftMotorCounter) < leftMotorCounterPositionToSpeedDown && 
      abs(rightMotorCounter) >= rightMotorCounterPositionToSpeedUp &&
      abs(rightMotorCounter) < rightMotorCounterPositionToSpeedDown) {
    digitalWrite(RELAY_12_36_PIN, HIGH); // Switch to 36v
    DEBUG_PRINT_LN("Doors speed up");
  } else if (digitalRead(RELAY_12_36_PIN) == HIGH && 
            (abs(leftMotorCounter) >= leftMotorCounterPositionToSpeedDown || 
            abs(rightMotorCounter) >= rightMotorCounterPositionToSpeedDown ||
            abs(leftMotorCounter) < leftMotorCounterPositionToSpeedUp || 
            abs(rightMotorCounter) < rightMotorCounterPositionToSpeedUp)) {
    digitalWrite(RELAY_12_36_PIN, LOW); // Switch to 12v
    DEBUG_PRINT_LN("Doors speed down");
  }
}

// ==================================================================================================================

// Variables to handle signal lamp
unsigned long signalLampStartMillis = 0;
unsigned long signalLamp1StartMillis = 0;
void handleSignalLamp() {
  if (signalLampStartMillis != 0 && currentMillis >= signalLampStartMillis + TIME_TO_TOGGLE_SIGNAL_LAMP) {
    digitalWrite(SIGNAL_LAMP_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    signalLampStartMillis = 0;
    signalLamp1StartMillis = currentMillis;
    //DEBUG_PRINT_LN("Signal Lamp: ON");
  } else if (signalLamp1StartMillis != 0 && currentMillis >= signalLamp1StartMillis + TIME_TO_TOGGLE_SIGNAL_LAMP) {
    digitalWrite(SIGNAL_LAMP_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    signalLamp1StartMillis = 0;
    signalLampStartMillis = currentMillis;
    //DEBUG_PRINT_LN("Signal Lamp: OFF");
  }
}

// ==================================================================================================================

void handleStopsSignals() {
  if (isLeftDoorOpened() && isLeftDoorOpening()) {
    digitalWrite(RELAY_12_36_PIN, LOW); // Switch to 12v
    digitalWrite(OPEN_LEFT_MOTOR_ACTUATOR_RELAY_PIN, LOW);
    if (isLeftDoorOpened() && (isRightDoorOpened() || currentRequestCommand == REQUEST_COMMAND_TO_OPEN_LEFT_DOOR)) {
      currentRequestCommand = REQUEST_COMMAND_UNKNOWN;
      opositeRequestCommand = REQUEST_COMMAND_TO_CLOSE;
      digitalWrite(SIGNAL_LAMP_PIN, LOW);
      digitalWrite(BUZZER_PIN, LOW);
      signalLampStartMillis = 0;
      signalLamp1StartMillis = 0;
    }
    DEBUG_PRINT_LN("L Door is Opened");
  }

  if (isLeftDoorClosed() && isLeftDoorClosing()) {
    digitalWrite(RELAY_12_36_PIN, LOW); // Switch to 12v
    digitalWrite(CLOSE_LEFT_MOTOR_ACTUATOR_RELAY_PIN, LOW);
    if (isLeftDoorClosed() && isRightDoorClosed()) {
      currentRequestCommand = REQUEST_COMMAND_UNKNOWN;
      opositeRequestCommand = REQUEST_COMMAND_TO_OPEN;
      lockLeftDoorStartMillis = currentMillis + TIME_BETWEEN_ACTUATOR_AND_MOTORS_SWITCH; // Start lock proccess
      lockRightDoorStartMillis = currentMillis + TIME_BETWEEN_ACTUATOR_AND_MOTORS_SWITCH; // Start lock proccess

      digitalWrite(SIGNAL_LAMP_PIN, LOW);
      digitalWrite(BUZZER_PIN, LOW);
      signalLampStartMillis = 0;
      signalLamp1StartMillis = 0;
    }
    DEBUG_PRINT_LN("L Door is Closed");
  }

  if (isRightDoorOpened() && isRightDoorOpening()) {
    digitalWrite(RELAY_12_36_PIN, LOW); // Switch to 12v
    digitalWrite(OPEN_RIGHT_MOTOR_ACTUATOR_RELAY_PIN, LOW);
    if (isLeftDoorOpened() && isRightDoorOpened()) {
      currentRequestCommand = REQUEST_COMMAND_UNKNOWN;
      opositeRequestCommand = REQUEST_COMMAND_TO_CLOSE;
      digitalWrite(SIGNAL_LAMP_PIN, LOW);
      digitalWrite(BUZZER_PIN, LOW);
      signalLampStartMillis = 0;
      signalLamp1StartMillis = 0;
    }
    DEBUG_PRINT_LN("R Door is Opened");
  }

  if (isRightDoorClosed() && isRightDoorClosing()) {
    digitalWrite(RELAY_12_36_PIN, LOW); // Switch to 12v
    digitalWrite(CLOSE_RIGHT_MOTOR_ACTUATOR_RELAY_PIN, LOW);
    if (isLeftDoorClosed() && isRightDoorClosed()) {
      currentRequestCommand = REQUEST_COMMAND_UNKNOWN;
      opositeRequestCommand = REQUEST_COMMAND_TO_OPEN;
      lockLeftDoorStartMillis = currentMillis + TIME_BETWEEN_ACTUATOR_AND_MOTORS_SWITCH; // Start lock proccess
      lockRightDoorStartMillis = currentMillis + TIME_BETWEEN_ACTUATOR_AND_MOTORS_SWITCH; // Start lock proccess

      digitalWrite(SIGNAL_LAMP_PIN, LOW);
      digitalWrite(BUZZER_PIN, LOW);
      signalLampStartMillis = 0;
      signalLamp1StartMillis = 0;
    }
    DEBUG_PRINT_LN("R Door is Closed");
  }
}

// ==================================================================================================================

uint8_t previousButtonState = HIGH;
unsigned long buzzerStartMillis = 0;
void handleButtonSignals() {
  boolean currentButtonState = digitalRead(OPEN_CLOSE_BUTTON_PIN);
  if (currentButtonState != previousButtonState && currentButtonState == LOW) {
    digitalWrite(BUZZER_PIN, HIGH);
    buzzerStartMillis = currentMillis;
    if (currentRequestCommand == REQUEST_COMMAND_TO_STOP) {
      currentRequestCommand = opositeRequestCommand;
    } else if (currentRequestCommand == REQUEST_COMMAND_TO_OPEN || currentRequestCommand == REQUEST_COMMAND_TO_OPEN_LEFT_DOOR) {
      opositeRequestCommand = REQUEST_COMMAND_TO_CLOSE;
      currentRequestCommand = REQUEST_COMMAND_TO_STOP;
    } else if (currentRequestCommand == REQUEST_COMMAND_TO_CLOSE) {
      opositeRequestCommand = REQUEST_COMMAND_TO_OPEN;
      currentRequestCommand = REQUEST_COMMAND_TO_STOP;
    } else if (currentRequestCommand == REQUEST_COMMAND_UNKNOWN) {
      currentRequestCommand = opositeRequestCommand;
    }
  }

  if (buzzerStartMillis != 0 && currentMillis > buzzerStartMillis + 50) {
    digitalWrite(BUZZER_PIN, LOW);
    buzzerStartMillis = 0;
  }
  
  previousButtonState = currentButtonState;
}

// ==================================================================================================================

unsigned long stopDoorsStartMillis = 0;
void handleStopDoors() {
  if (stopDoorsStartMillis != 0 && currentMillis >= stopDoorsStartMillis) {
    digitalWrite(OPEN_LEFT_MOTOR_ACTUATOR_RELAY_PIN, LOW);
    digitalWrite(CLOSE_LEFT_MOTOR_ACTUATOR_RELAY_PIN, LOW);
    digitalWrite(OPEN_RIGHT_MOTOR_ACTUATOR_RELAY_PIN, LOW);
    digitalWrite(CLOSE_RIGHT_MOTOR_ACTUATOR_RELAY_PIN, LOW);
    digitalWrite(MOTOR_ACTUATOR_RELAY_PIN, LOW);
    //digitalWrite(INTERIOR_LIGHT_RELAY_PIN, LOW);
    digitalWrite(SIGNAL_LAMP_PIN, LOW);
    if (buzzerStartMillis == 0) {
      digitalWrite(BUZZER_PIN, LOW);
    }
    stopDoorsStartMillis = 0;
    DEBUG_PRINT_LN("Stop Doors");
  } 
}

// ==================================================================================================================

unsigned long interiorLightStartMillis = 0;
unsigned long interiorLight1StartMillis = 0;
void handleInteriorLight() {
  if (interiorLightStartMillis != 0 && currentMillis >= interiorLightStartMillis) {
    digitalWrite(INTERIOR_LIGHT_RELAY_PIN, HIGH);
    interiorLightStartMillis = 0;
    interiorLight1StartMillis = currentMillis;
    DEBUG_PRINT_LN("Interior Lamp: ON");
  } else if (interiorLight1StartMillis != 0 && currentMillis >= interiorLight1StartMillis + TIME_TO_SWITCH_ON_INTERIOR_LAMP) {
    digitalWrite(INTERIOR_LIGHT_RELAY_PIN, LOW);
    interiorLight1StartMillis = 0;
    DEBUG_PRINT_LN("Interior Lamp: OFF");
  }
}

// ==================================================================================================================

void printMotorCounter() {
  char leftCounterStr[4];
  char rightCounterStr[4];
  sprintf(leftCounterStr, "%03d", leftMotorCounter);
  sprintf(rightCounterStr, "%03d", rightMotorCounter);
  for(int i = 0; i < 21; i++) {
    if (displayLine1[i] == '\0') {
      displayLine1[i] = ' ';
    } else if (i >= 18) {
      displayLine1[i] = leftCounterStr[i - 18];
      displayLine2[i] = rightCounterStr[i - 18];
    }
    if (displayLine2[i] == '\0') {
      displayLine2[i] = ' ';
    } else if (i >= 18) {
      displayLine2[i] = rightCounterStr[i - 18];
    }
  }
}

// ==================================================================================================================

void resetVariables() {
  unlockLeftDoorStartMillis = 0;
  unlockLeftDoor1StartMillis = 0;
  unlockLeftDoor2StartMillis = 0;
  unlockRightDoorStartMillis = 0;
  unlockRightDoor1StartMillis = 0;
  unlockRightDoor2StartMillis = 0;
  lockLeftDoorStartMillis = 0;
  lockLeftDoor1StartMillis = 0;
  lockLeftDoor2StartMillis = 0;
  lockRightDoorStartMillis = 0;
  lockRightDoor1StartMillis = 0;
  lockRightDoor2StartMillis = 0;

  openLeftDoorStartMillis = 0;
  closeLeftDoorStartMillis = 0;
  openRightDoorStartMillis = 0;
  closeRightDoorStartMillis = 0;

  stopDoorsStartMillis = 0;
  
//  interiorLightStartMillis = 0;
//  interiorLight1StartMillis = 0;

  signalLampStartMillis = 0;
  signalLamp1StartMillis = 0;
  
}
