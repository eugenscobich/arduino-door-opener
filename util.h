#include "config.h"

// PINs
#define RC_PIN PB12
#define INTERIOR_LIGHT_RELAY_PIN PA7
#define OPEN_CLOSE_BUTTON_PIN PB3

#define LEFT_MOTOR_COUNTER_PIN PB8
#define RIGHT_MOTOR_COUNTER_PIN PB9

#define LEFT_DOOR_OPEN_PIN PB11     // Normal Open
#define LEFT_DOOR_CLOSE_PIN PB10    // Normal Conected
#define RIGHT_DOOR_OPEN_PIN PA11    // Normal Open
#define RIGHT_DOOR_CLOSE_PIN PA12   // Normal Conected


#define RELAY_12_36_PIN PA6
#define MOTOR_ACTUATOR_RELAY_PIN PA5

#define OBSTACLE_LAMP_PIN PA4
#define SIGNAL_LAMP_PIN PA3

#define OPEN_LEFT_MOTOR_ACTUATOR_RELAY_PIN PC15
#define CLOSE_LEFT_MOTOR_ACTUATOR_RELAY_PIN PA0
#define OPEN_RIGHT_MOTOR_ACTUATOR_RELAY_PIN PA1
#define CLOSE_RIGHT_MOTOR_ACTUATOR_RELAY_PIN PA2

#define BUZZER_PIN PC13

// Request Command
#define REQUEST_COMMAND_UNKNOWN -1
#define REQUEST_COMMAND_TO_OPEN 0
#define REQUEST_COMMAND_TO_CLOSE 1
#define REQUEST_COMMAND_TO_OPEN_LEFT_DOOR 2
#define REQUEST_COMMAND_TO_STOP 3

char emptyLine[22] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\0'};
char displayLineWithNumbers[22] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\0'};
char displayLine1[22] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\0'};
char displayLine2[22] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\0'};
char displayLine3[22] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\0'};
char displayLine4[22] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\0'};
char displayLine5[22] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\0'};
char displayLine6[22] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\0'};
char displayLine7[22] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\0'};
char displayLine8[22] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\0'};

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
   strncpy(displayLine8, str, 21);                        \
   if (strlen(str) < 18) {                                \
     strncat(displayLine8, emptyLine, 21 - strlen(str));  \
   }


#define DEBUG_PRINT_NUMBER_LN(number)                                        \
   Serial.print(millis());                                                   \
   Serial.print(": ");                                                       \
   Serial.println(number);                                                   \
   strncpy(displayLine1, displayLine2, 21);                                  \
   strncpy(displayLine2, displayLine3, 21);                                  \
   strncpy(displayLine3, displayLine4, 21);                                  \
   strncpy(displayLine4, displayLine5, 21);                                  \
   strncpy(displayLine5, displayLine6, 21);                                  \
   strncpy(displayLine6, displayLine7, 21);                                  \
   strncpy(displayLine7, displayLine8, 21);                                  \
   sprintf(displayLineWithNumbers, "%d", number);                            \
   strncpy(displayLine8, displayLineWithNumbers, 21);                        \
   if (strlen(displayLineWithNumbers) < 18) {                                \
     strncat(displayLine8, emptyLine, 21 - strlen(displayLineWithNumbers));  \
   }

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

bool canOpenLeftDoor() {
    if (digitalRead(OPEN_LEFT_MOTOR_ACTUATOR_RELAY_PIN) == HIGH && digitalRead(MOTOR_ACTUATOR_RELAY_PIN) == LOW || digitalRead(LEFT_DOOR_OPEN_PIN) == LOW) {
      DEBUG_PRINT_LN("Can't Open Left Door");
      return false;
    }
    return true;
}

bool canOpenRightDoor() {
    if (digitalRead(OPEN_RIGHT_MOTOR_ACTUATOR_RELAY_PIN) == HIGH && digitalRead(MOTOR_ACTUATOR_RELAY_PIN) == LOW || digitalRead(RIGHT_DOOR_OPEN_PIN) == LOW) {
      DEBUG_PRINT_LN("Can't Open Right Door");
      return false;
    }
    return true;
}

bool canCloseLeftDoor() {
    if (digitalRead(CLOSE_LEFT_MOTOR_ACTUATOR_RELAY_PIN) == HIGH && digitalRead(MOTOR_ACTUATOR_RELAY_PIN) == LOW || digitalRead(LEFT_DOOR_CLOSE_PIN) == HIGH) {
      DEBUG_PRINT_LN("Can't Close Left Door");
      return false;
    }
    return true;
}

bool canCloseRightDoor() {
    if (digitalRead(CLOSE_RIGHT_MOTOR_ACTUATOR_RELAY_PIN) == HIGH && digitalRead(MOTOR_ACTUATOR_RELAY_PIN) == LOW || digitalRead(RIGHT_DOOR_CLOSE_PIN) == HIGH
    ) {
      DEBUG_PRINT_LN("Can't Close Right Door");
      return false;
    }
    return true;
}

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


unsigned long leftMotorCounter = 0;
unsigned long rightMotorCounter = 0;
uint8_t previousLeftMotorCounterPinState = HIGH;
uint8_t previousRightMotorCounterPinState = HIGH;
void handleMotorCounters() {
  uint8_t currentLeftMotorCounterPinState = digitalRead(LEFT_MOTOR_COUNTER_PIN);
  if (currentLeftMotorCounterPinState != previousLeftMotorCounterPinState && previousLeftMotorCounterPinState == LOW) {
    leftMotorCounter++;
  }
  previousLeftMotorCounterPinState = currentLeftMotorCounterPinState;

  uint8_t currentRightMotorCounterPinState = digitalRead(RIGHT_MOTOR_COUNTER_PIN);
  if (currentRightMotorCounterPinState != previousRightMotorCounterPinState && previousRightMotorCounterPinState == LOW) {
    rightMotorCounter++;
  }
  previousRightMotorCounterPinState = currentRightMotorCounterPinState;
}

// Variables to react on Request Command and handle doors oppening

unsigned long openLeftDoorStartMillis = 0;
unsigned long openLeftDoorChangeSpeedStartMillis = 0;

unsigned long closeLeftDoorStartMillis = 0;
unsigned long closeLeftDoorChangeSpeedStartMillis = 0;
unsigned long numberOfRightMotorRevolutionsAfterStartCloseLeftDoor = 0;

unsigned long openRightDoorStartMillis = 0;
unsigned long openRightDoorChangeSpeedStartMillis = 0;
unsigned long numberOfLeftMotorRevolutionsAfterStartOpenRightDoor = 0;

unsigned long closeRightDoorStartMillis = 0;
unsigned long closeRightDoorChangeSpeedStartMillis = 0;

void handleDoorCommands() {
  // Open Left Door
  if (openLeftDoorStartMillis != 0 && currentMillis >= openLeftDoorStartMillis) {
    DEBUG_PRINT_LN("L Door Opening: Start");
    digitalWrite(RELAY_12_36_PIN, LOW); // Ensure we are at 12v
    digitalWrite(OPEN_LEFT_MOTOR_ACTUATOR_RELAY_PIN, HIGH);
    openLeftDoorChangeSpeedStartMillis = currentMillis;
    openLeftDoorStartMillis = 0;
    leftMotorCounter = 0;
  } else if (!USE_MOTOR_REVOLUTION_COUNTERS && openLeftDoorChangeSpeedStartMillis != 0 && currentMillis >= openLeftDoorChangeSpeedStartMillis + TIME_TO_CHANGE_MOTORS_SPEED) {
    digitalWrite(RELAY_12_36_PIN, HIGH); // Switch to 36v
    openLeftDoorChangeSpeedStartMillis = 0;
    DEBUG_PRINT_LN("L Door Opening: In Process");
  }

  // Close Left Door
  if (closeLeftDoorStartMillis != 0 && currentMillis >= closeLeftDoorStartMillis && rightMotorCounter > numberOfRightMotorRevolutionsAfterStartCloseLeftDoor) {
    DEBUG_PRINT_LN("L Door Closing: Start");
    digitalWrite(RELAY_12_36_PIN, LOW); // Ensure we are at 12v
    digitalWrite(CLOSE_LEFT_MOTOR_ACTUATOR_RELAY_PIN, HIGH);
    closeLeftDoorChangeSpeedStartMillis = currentMillis;
    closeLeftDoorStartMillis = 0;
    leftMotorCounter = 0;
  } else if (!USE_MOTOR_REVOLUTION_COUNTERS && closeLeftDoorChangeSpeedStartMillis != 0 && currentMillis >= closeLeftDoorChangeSpeedStartMillis + TIME_TO_CHANGE_MOTORS_SPEED) {
    digitalWrite(RELAY_12_36_PIN, HIGH); // Switch to 36v
    closeLeftDoorChangeSpeedStartMillis = 0;
    DEBUG_PRINT_LN("L Door Closing: In Process");
  }

  // Open Right Door
  if (openRightDoorStartMillis != 0 && currentMillis >= openRightDoorStartMillis && leftMotorCounter > numberOfLeftMotorRevolutionsAfterStartOpenRightDoor) {
    DEBUG_PRINT_LN("R Door Opening: Start");
    digitalWrite(RELAY_12_36_PIN, LOW); // Ensure we are at 12v
    digitalWrite(OPEN_RIGHT_MOTOR_ACTUATOR_RELAY_PIN, HIGH);
    openRightDoorChangeSpeedStartMillis = currentMillis;
    openRightDoorStartMillis = 0;
    rightMotorCounter = 0;
  } else if (!USE_MOTOR_REVOLUTION_COUNTERS && openRightDoorChangeSpeedStartMillis != 0 && currentMillis >= openRightDoorChangeSpeedStartMillis + TIME_TO_CHANGE_MOTORS_SPEED) {
    digitalWrite(RELAY_12_36_PIN, HIGH); // Switch to 36v
    openRightDoorChangeSpeedStartMillis = 0;
    DEBUG_PRINT_LN("R Door Opening: In Process");
  }

  // Close Right Door
  if (closeRightDoorStartMillis != 0 && currentMillis >= closeRightDoorStartMillis) {
    DEBUG_PRINT_LN("R Door Closing: Start");
    digitalWrite(RELAY_12_36_PIN, LOW); // Ensure we are at 12v
    digitalWrite(CLOSE_RIGHT_MOTOR_ACTUATOR_RELAY_PIN, HIGH);
    closeRightDoorChangeSpeedStartMillis = currentMillis;
    closeRightDoorStartMillis = 0;
    rightMotorCounter = 0;
  } else if (!USE_MOTOR_REVOLUTION_COUNTERS && closeRightDoorChangeSpeedStartMillis != 0 && currentMillis >= closeRightDoorChangeSpeedStartMillis + TIME_TO_CHANGE_MOTORS_SPEED) {
    digitalWrite(RELAY_12_36_PIN, HIGH); // Switch to 36v
    closeRightDoorChangeSpeedStartMillis = 0;
    DEBUG_PRINT_LN("R Door Closing: In Process");
  }
}

void handleChangeMotorSpeed() {

  if (USE_MOTOR_REVOLUTION_COUNTERS && 
            leftMotorCounter > NUMBER_OF_MOTOR_REVOLUTIONS_BEFORE_TO_SPEED_UP && rightMotorCounter > NUMBER_OF_MOTOR_REVOLUTIONS_BEFORE_TO_SPEED_UP && 
            leftMotorCounter < NUMBER_OF_MOTOR_REVOLUTIONS_AFTER_TO_SPEED_DOWN && rightMotorCounter < NUMBER_OF_MOTOR_REVOLUTIONS_AFTER_TO_SPEED_DOWN &&
            digitalRead(RELAY_12_36_PIN) == LOW) {
    digitalWrite(RELAY_12_36_PIN, HIGH); // Switch to 36v
    closeRightDoorChangeSpeedStartMillis = 0;
    DEBUG_PRINT_LN("Doors Speed up");
  } else if (USE_MOTOR_REVOLUTION_COUNTERS && 
            (leftMotorCounter > NUMBER_OF_MOTOR_REVOLUTIONS_AFTER_TO_SPEED_DOWN || rightMotorCounter > NUMBER_OF_MOTOR_REVOLUTIONS_AFTER_TO_SPEED_DOWN) &&
            digitalRead(RELAY_12_36_PIN) == HIGH) {
    digitalWrite(RELAY_12_36_PIN, LOW); // Switch to 12v
    DEBUG_PRINT_LN("Doors Speed down");
  }
  
}



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


// Variables to handle stop signals
unsigned long stopOpenLeftDoorStartMillis = 0;
unsigned long stopCloseLeftDoorStartMillis = 0;
unsigned long stopOpenRightDoorStartMillis = 0;
unsigned long stopCloseRightDoorStartMillis = 0;

void handleStopsSignals() {
  if (digitalRead(MOTOR_ACTUATOR_RELAY_PIN) == LOW && digitalRead(OPEN_LEFT_MOTOR_ACTUATOR_RELAY_PIN) == HIGH && digitalRead(LEFT_DOOR_OPEN_PIN) == LOW && stopOpenLeftDoorStartMillis == 0) { // Normal Open
    DEBUG_PRINT_LN("L Door is Opening");
    digitalWrite(RELAY_12_36_PIN, LOW); // Switch to 12v
    stopOpenLeftDoorStartMillis = currentMillis;
  } else if (digitalRead(MOTOR_ACTUATOR_RELAY_PIN) == LOW && digitalRead(OPEN_LEFT_MOTOR_ACTUATOR_RELAY_PIN) == HIGH && stopOpenLeftDoorStartMillis != 0 && currentMillis >= stopOpenLeftDoorStartMillis + TIME_TO_MOTORS_CLOSE_UP_DOORS) {
    digitalWrite(OPEN_LEFT_MOTOR_ACTUATOR_RELAY_PIN, LOW);
    stopOpenLeftDoorStartMillis = 0;
    currentRequestCommand = REQUEST_COMMAND_UNKNOWN;
    opositeRequestCommand = REQUEST_COMMAND_TO_CLOSE;
    if (digitalRead(LEFT_DOOR_OPEN_PIN) == LOW && digitalRead(RIGHT_DOOR_OPEN_PIN) == LOW) {
      digitalWrite(SIGNAL_LAMP_PIN, LOW);
      digitalWrite(BUZZER_PIN, LOW);
      signalLampStartMillis = 0;
      signalLamp1StartMillis = 0;
    }
    DEBUG_PRINT_LN("L Door is Opened");
  }

  if (digitalRead(MOTOR_ACTUATOR_RELAY_PIN) == LOW && digitalRead(CLOSE_LEFT_MOTOR_ACTUATOR_RELAY_PIN) == HIGH && digitalRead(LEFT_DOOR_CLOSE_PIN) == HIGH && stopCloseLeftDoorStartMillis == 0) { // Normal Conected
    DEBUG_PRINT_LN("L Door is Closing");
    digitalWrite(RELAY_12_36_PIN, LOW); // Switch to 12v
    stopCloseLeftDoorStartMillis = currentMillis;
  } else if (digitalRead(MOTOR_ACTUATOR_RELAY_PIN) == LOW && digitalRead(CLOSE_LEFT_MOTOR_ACTUATOR_RELAY_PIN) == HIGH && stopCloseLeftDoorStartMillis != 0 && currentMillis >= stopCloseLeftDoorStartMillis + TIME_TO_MOTORS_CLOSE_UP_DOORS) {
    digitalWrite(CLOSE_LEFT_MOTOR_ACTUATOR_RELAY_PIN, LOW);
    stopCloseLeftDoorStartMillis = 0;
    currentRequestCommand = REQUEST_COMMAND_UNKNOWN;
    opositeRequestCommand = REQUEST_COMMAND_TO_OPEN;
    if (digitalRead(LEFT_DOOR_CLOSE_PIN) == HIGH && digitalRead(RIGHT_DOOR_CLOSE_PIN) == HIGH) {
      lockLeftDoorStartMillis = currentMillis + TIME_TO_STOP_MOTORS; // Start lock proccess
      lockRightDoorStartMillis = currentMillis + TIME_TO_STOP_MOTORS; // Start lock proccess
    }
    DEBUG_PRINT_LN("L Door is Closed");
  }

  if (digitalRead(MOTOR_ACTUATOR_RELAY_PIN) == LOW && digitalRead(OPEN_RIGHT_MOTOR_ACTUATOR_RELAY_PIN) == HIGH && digitalRead(RIGHT_DOOR_OPEN_PIN) == LOW && stopOpenRightDoorStartMillis == 0) { // Normal Open
    DEBUG_PRINT_LN("R Door is Opening");
    digitalWrite(RELAY_12_36_PIN, LOW); // Switch to 12v
    stopOpenRightDoorStartMillis = currentMillis;
  } else if (digitalRead(MOTOR_ACTUATOR_RELAY_PIN) == LOW && digitalRead(OPEN_RIGHT_MOTOR_ACTUATOR_RELAY_PIN) == HIGH && stopOpenRightDoorStartMillis != 0 && currentMillis >= stopOpenRightDoorStartMillis + TIME_TO_MOTORS_CLOSE_UP_DOORS) {
    digitalWrite(OPEN_RIGHT_MOTOR_ACTUATOR_RELAY_PIN, LOW);
    stopOpenRightDoorStartMillis = 0;
    currentRequestCommand = REQUEST_COMMAND_UNKNOWN;
    opositeRequestCommand = REQUEST_COMMAND_TO_CLOSE;
    if (digitalRead(LEFT_DOOR_OPEN_PIN) == LOW && digitalRead(RIGHT_DOOR_OPEN_PIN) == LOW) {
      digitalWrite(SIGNAL_LAMP_PIN, LOW);
      digitalWrite(BUZZER_PIN, LOW);
      signalLampStartMillis = 0;
      signalLamp1StartMillis = 0;
    }
    DEBUG_PRINT_LN("R Door is Opened");
  }

  if (digitalRead(MOTOR_ACTUATOR_RELAY_PIN) == LOW && digitalRead(CLOSE_RIGHT_MOTOR_ACTUATOR_RELAY_PIN) == HIGH && digitalRead(RIGHT_DOOR_CLOSE_PIN) == HIGH && stopCloseRightDoorStartMillis == 0) { // Normal Conected
    DEBUG_PRINT_LN("R Door is Closing");
    digitalWrite(RELAY_12_36_PIN, LOW); // Switch to 12v
    stopCloseRightDoorStartMillis = currentMillis;
  } else if (digitalRead(MOTOR_ACTUATOR_RELAY_PIN) == LOW && digitalRead(CLOSE_RIGHT_MOTOR_ACTUATOR_RELAY_PIN) == HIGH && stopCloseRightDoorStartMillis != 0 && currentMillis >= stopCloseRightDoorStartMillis + TIME_TO_MOTORS_CLOSE_UP_DOORS) {
    digitalWrite(CLOSE_RIGHT_MOTOR_ACTUATOR_RELAY_PIN, LOW);
    stopCloseRightDoorStartMillis = 0;
    currentRequestCommand = REQUEST_COMMAND_UNKNOWN;
    opositeRequestCommand = REQUEST_COMMAND_TO_OPEN;
    if (digitalRead(LEFT_DOOR_CLOSE_PIN) == HIGH && digitalRead(RIGHT_DOOR_CLOSE_PIN) == HIGH) {
      lockLeftDoorStartMillis = currentMillis + TIME_TO_STOP_MOTORS; // Start lock proccess
      lockRightDoorStartMillis = currentMillis + TIME_TO_STOP_MOTORS; // Start lock proccess
    }
    DEBUG_PRINT_LN("R Door is Closed");
  }
}

unsigned long stopDoorsStartMillis = 0;
unsigned long stopDoors1StartMillis = 0;
void handleStopDoors() {
  if (stopDoorsStartMillis != 0 && currentMillis >= stopDoorsStartMillis) {
      DEBUG_PRINT_LN("Stop Doors: Start");
      digitalWrite(RELAY_12_36_PIN, LOW); // Switch to 12v
      stopDoors1StartMillis = currentMillis;
      stopDoorsStartMillis = 0;
  } else if (stopDoors1StartMillis != 0 && currentMillis >= stopDoors1StartMillis + TIME_TO_MOTORS_CLOSE_UP_DOORS)  {
    digitalWrite(OPEN_LEFT_MOTOR_ACTUATOR_RELAY_PIN, LOW);
    digitalWrite(CLOSE_LEFT_MOTOR_ACTUATOR_RELAY_PIN, LOW);
    digitalWrite(OPEN_RIGHT_MOTOR_ACTUATOR_RELAY_PIN, LOW);
    digitalWrite(CLOSE_RIGHT_MOTOR_ACTUATOR_RELAY_PIN, LOW);
    digitalWrite(MOTOR_ACTUATOR_RELAY_PIN, LOW);
    //digitalWrite(INTERIOR_LIGHT_RELAY_PIN, LOW);
    digitalWrite(SIGNAL_LAMP_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    stopDoors1StartMillis = 0;
    DEBUG_PRINT_LN("Stop Doors: Finished");
  } 
}



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


void printMotorCounter() {
  char leftCounterStr[4];
  char rightCounterStr[4];
  sprintf(leftCounterStr, "%03u", leftMotorCounter);
  sprintf(rightCounterStr, "%03u", rightMotorCounter);

  char finalDisplayLine1[22];
  char finalDisplayLine2[22];

  strncpy(finalDisplayLine1, displayLine1, 18);
  strncpy(finalDisplayLine2, displayLine2, 18);

  finalDisplayLine1[18] = '\0';
  finalDisplayLine2[18] = '\0';

  strcat(finalDisplayLine1, leftCounterStr);
  strcat(finalDisplayLine2, rightCounterStr);

  strncpy(displayLine1, finalDisplayLine1, 21);
  strncpy(displayLine2, finalDisplayLine2, 21);
}


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
  openLeftDoorChangeSpeedStartMillis = 0;
  closeLeftDoorStartMillis = 0;
  closeLeftDoorChangeSpeedStartMillis = 0;
  openRightDoorStartMillis = 0;
  openRightDoorChangeSpeedStartMillis = 0;
  closeRightDoorStartMillis = 0;
  closeRightDoorChangeSpeedStartMillis = 0;

  stopOpenLeftDoorStartMillis = 0;
  stopCloseLeftDoorStartMillis = 0;
  stopOpenRightDoorStartMillis = 0;
  stopCloseRightDoorStartMillis = 0;

  stopDoorsStartMillis = 0;
  stopDoors1StartMillis = 0;

//  interiorLightStartMillis = 0;
//  interiorLight1StartMillis = 0;

  signalLampStartMillis = 0;
  signalLamp1StartMillis = 0;
  
}
