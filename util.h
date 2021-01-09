#include "config.h"

// PINs
#define RC_PIN PB12
#define INTERIOR_LIGHT_RELAY_PIN PA7
#define OPEN_CLOSE_BUTTON_PIN PB3

#define LEFT_DOOR_OPEN_PIN PB11     // Normal Open
#define LEFT_DOOR_CLOSE_PIN PB10    // Normal Conected
#define RIGHT_DOOR_OPEN_PIN PA11    // Normal Open
#define RIGHT_DOOR_CLOSE_PIN PA12   // Normal Conected

#define MOTOR_ACTUATOR_RELAY_PIN PA5
#define RELAY_12_36_PIN PA6

#define OPEN_LEFT_MOTOR_ACTUATOR_RELAY_PIN PA4
#define CLOSE_LEFT_MOTOR_ACTUATOR_RELAY_PIN PA3
#define OPEN_RIGHT_MOTOR_ACTUATOR_RELAY_PIN PA2
#define CLOSE_RIGHT_MOTOR_ACTUATOR_RELAY_PIN PA1

#define SIGNAL_LAMP_PIN PA0
#define OBSTACLE_LAMP_PIN PC15
#define BUZZER_PIN PC14

// Request Command
#define REQUEST_COMMAND_UNKNOWN -1
#define REQUEST_COMMAND_TO_OPEN 0
#define REQUEST_COMMAND_TO_CLOSE 1
#define REQUEST_COMMAND_TO_OPEN_LEFT_DOOR 2

char displayLineWithNumbers[22];
char displayLine1[22];
char displayLine2[22];
char displayLine3[22];
char displayLine4[22];
char displayLine5[22];
char displayLine6[22];
char displayLine7[22];
char displayLine8[22];

#define DEBUG_PRINT_LN(str)                    \
   Serial.print(millis());                     \
   Serial.print(": ");                         \
   Serial.println(str);                        \
   strncpy(displayLine1, displayLine2, 21);    \
   strncpy(displayLine2, displayLine3, 21);    \
   strncpy(displayLine3, displayLine4, 21);    \
   strncpy(displayLine4, displayLine5, 21);    \
   strncpy(displayLine5, displayLine6, 21);    \
   strncpy(displayLine6, displayLine7, 21);    \
   strncpy(displayLine7, displayLine8, 21);    \
   strncpy(displayLine8, str, 21);


#define DEBUG_PRINT_NUMBER_LN(number)                 \
   Serial.print(millis());                            \
   Serial.print(": ");                                \
   Serial.println(number);                            \
   strncpy(displayLine1, displayLine2, 21);           \
   strncpy(displayLine2, displayLine3, 21);           \
   strncpy(displayLine3, displayLine4, 21);           \
   strncpy(displayLine4, displayLine5, 21);           \
   strncpy(displayLine5, displayLine6, 21);           \
   strncpy(displayLine6, displayLine7, 21);           \
   strncpy(displayLine7, displayLine8, 21);           \
   sprintf(displayLineWithNumbers, "%d", number);     \
   strncpy(displayLine8, displayLineWithNumbers, 21);   

#define DEBUG_PRINT(str)       \
   Serial.print(millis());     \
   Serial.print(": ");         \
   Serial.print(str);


unsigned long currentMillis = 0;

char* getRequestCommandName(int currentRequestCommand) {
  static char buf[14];
  if (currentRequestCommand == REQUEST_COMMAND_TO_OPEN) {
    strcpy(buf, "Open");
  } else if (currentRequestCommand == REQUEST_COMMAND_TO_CLOSE) {
    strcpy(buf, "Close");
  } else if (currentRequestCommand == REQUEST_COMMAND_TO_OPEN_LEFT_DOOR) {
    strcpy(buf, "Open Left Door");
  } else {
    strcpy(buf, "Unknown");
  }
  return buf;
}


bool canOpenLeftDoor() {
    if (digitalRead(OPEN_LEFT_MOTOR_ACTUATOR_RELAY_PIN) == HIGH && digitalRead(MOTOR_ACTUATOR_RELAY_PIN) == LOW || digitalRead(LEFT_DOOR_OPEN_PIN) == LOW) {
      DEBUG_PRINT_LN("Cannot Open Left Door");
      return false;
    }
    return true;
}

bool canOpenRightDoor() {
    if (digitalRead(OPEN_RIGHT_MOTOR_ACTUATOR_RELAY_PIN) == HIGH && digitalRead(MOTOR_ACTUATOR_RELAY_PIN) == LOW || digitalRead(RIGHT_DOOR_OPEN_PIN) == LOW) {
      DEBUG_PRINT_LN("Cannot Open Right Door");
      return false;
    }
    return true;
}

bool canCloseLeftDoor() {
    if (digitalRead(CLOSE_LEFT_MOTOR_ACTUATOR_RELAY_PIN) == HIGH && digitalRead(MOTOR_ACTUATOR_RELAY_PIN) == LOW || digitalRead(LEFT_DOOR_CLOSE_PIN) == HIGH) {
      DEBUG_PRINT_LN("Cannot Close Left Door");
      return false;
    }
    return true;
}

bool canCloseRightDoor() {
    if (digitalRead(CLOSE_RIGHT_MOTOR_ACTUATOR_RELAY_PIN) == HIGH && digitalRead(MOTOR_ACTUATOR_RELAY_PIN) == LOW || digitalRead(RIGHT_DOOR_CLOSE_PIN) == HIGH
    ) {
      DEBUG_PRINT_LN("Cannot Close Right Door");
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
    DEBUG_PRINT_LN("Unlock Left Door: Start");
    digitalWrite(CLOSE_LEFT_MOTOR_ACTUATOR_RELAY_PIN, LOW); // Stop any close actions
    digitalWrite(RELAY_12_36_PIN, LOW); // Switch to 12v
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
    DEBUG_PRINT_LN("Unlock Left Door: Finished");
  }

  if (lockLeftDoorStartMillis != 0 && currentMillis >= lockLeftDoorStartMillis) {
    DEBUG_PRINT_LN("Lock Left Door: Start");
    digitalWrite(OPEN_LEFT_MOTOR_ACTUATOR_RELAY_PIN, LOW); // Stop any Open actions
    digitalWrite(RELAY_12_36_PIN, LOW); // Switch to 12v
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
    DEBUG_PRINT_LN("Lock Left Door: Finished");
  }

  if (unlockRightDoorStartMillis != 0 && currentMillis >= unlockRightDoorStartMillis) {
    DEBUG_PRINT_LN("Unlock Right Door: Start");
    digitalWrite(CLOSE_RIGHT_MOTOR_ACTUATOR_RELAY_PIN, LOW); // Stop any close actions
    digitalWrite(RELAY_12_36_PIN, LOW); // Switch to 12v
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
    DEBUG_PRINT_LN("Unlock Right Door: Finished");
  }
  
  if (lockRightDoorStartMillis != 0 && currentMillis >= lockRightDoorStartMillis) {
    DEBUG_PRINT_LN("Lock Right Door: Start");
    digitalWrite(OPEN_RIGHT_MOTOR_ACTUATOR_RELAY_PIN, LOW); // Stop any Open actions
    digitalWrite(RELAY_12_36_PIN, LOW); // Switch to 12v
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
    DEBUG_PRINT_LN("Lock Right Door: Finished");
  }  
}


// Variables to react on Request Command and handle doors oppening

unsigned long openLeftDoorStartMillis = 0;
unsigned long openLeftDoorChangeSpeedStartMillis = 0;
unsigned long closeLeftDoorShoutdownChangeSpeedStartMillis = 0;
unsigned long closeLeftDoorShoutdownProccessStartMillis = 0;

unsigned long closeLeftDoorStartMillis = 0;
unsigned long closeLeftDoorChangeSpeedStartMillis = 0;
unsigned long openLeftDoorShoutdownChangeSpeedStartMillis = 0;
unsigned long openLeftDoorShoutdownProccessStartMillis = 0;

unsigned long openRightDoorStartMillis = 0;
unsigned long openRightDoorChangeSpeedStartMillis = 0;
unsigned long closeRightDoorShoutdownChangeSpeedStartMillis = 0;
unsigned long closeRightDoorShoutdownProccessStartMillis = 0;

unsigned long closeRightDoorStartMillis = 0;
unsigned long closeRightDoorChangeSpeedStartMillis = 0;
unsigned long openRightDoorShoutdownChangeSpeedStartMillis = 0;
unsigned long openRightDoorShoutdownProccessStartMillis = 0;

void handleDoorCommands() {
  // Open Left Door
  if (openLeftDoorStartMillis != 0 && currentMillis >= openLeftDoorStartMillis) {
    if (digitalRead(CLOSE_LEFT_MOTOR_ACTUATOR_RELAY_PIN) == HIGH) {
      DEBUG_PRINT_LN("Stop Left Door Closing");
      digitalWrite(RELAY_12_36_PIN, LOW); // Switch to 12v
      closeLeftDoorShoutdownChangeSpeedStartMillis = currentMillis;
    } else {
      DEBUG_PRINT_LN("Start Left Door Opening");
      digitalWrite(OPEN_LEFT_MOTOR_ACTUATOR_RELAY_PIN, HIGH);
      openLeftDoorChangeSpeedStartMillis = currentMillis;
    }
    openLeftDoorStartMillis = 0;
  } else if (closeLeftDoorShoutdownChangeSpeedStartMillis != 0 && currentMillis >= closeLeftDoorShoutdownChangeSpeedStartMillis + TIME_TO_MOTORS_CLOSE_UP_DOORS)  {
    DEBUG_PRINT_LN("Stop Left Door Closing after change the speed");
    digitalWrite(CLOSE_LEFT_MOTOR_ACTUATOR_RELAY_PIN, LOW);
    closeLeftDoorShoutdownChangeSpeedStartMillis = 0;
    closeLeftDoorShoutdownProccessStartMillis = currentMillis;
  } else if (closeLeftDoorShoutdownProccessStartMillis != 0 && currentMillis >= closeLeftDoorShoutdownProccessStartMillis + TIME_TO_STOP_MOTORS)  {
    DEBUG_PRINT_LN("Start Left Door Opening after Clossing");
    digitalWrite(OPEN_LEFT_MOTOR_ACTUATOR_RELAY_PIN, HIGH);
    openLeftDoorChangeSpeedStartMillis = currentMillis;
    closeLeftDoorShoutdownProccessStartMillis = 0;
  }  
  if (openLeftDoorChangeSpeedStartMillis != 0 && currentMillis >= openLeftDoorChangeSpeedStartMillis + TIME_TO_CHANGE_MOTORS_SPEED) {
    digitalWrite(RELAY_12_36_PIN, HIGH); // Switch to 36v
    openLeftDoorChangeSpeedStartMillis = 0;
  }

  // Close Left Door
  if (closeLeftDoorStartMillis != 0 && currentMillis >= closeLeftDoorStartMillis) {
    if (digitalRead(OPEN_LEFT_MOTOR_ACTUATOR_RELAY_PIN) == HIGH) {
      DEBUG_PRINT_LN("Stop Left Door Opening");
      digitalWrite(RELAY_12_36_PIN, LOW); // Switch to 12v
      openLeftDoorShoutdownChangeSpeedStartMillis = currentMillis;
    } else {
      DEBUG_PRINT_LN("Start Left Door Closing");
      digitalWrite(CLOSE_LEFT_MOTOR_ACTUATOR_RELAY_PIN, HIGH);
      closeLeftDoorChangeSpeedStartMillis = currentMillis;
    }
    closeLeftDoorStartMillis = 0;
  } else if (openLeftDoorShoutdownChangeSpeedStartMillis != 0 && currentMillis >= openLeftDoorShoutdownChangeSpeedStartMillis + TIME_TO_MOTORS_CLOSE_UP_DOORS)  {
    DEBUG_PRINT_LN("Stop Left Door Opening after change the speed");
    digitalWrite(OPEN_LEFT_MOTOR_ACTUATOR_RELAY_PIN, LOW);
    openLeftDoorShoutdownChangeSpeedStartMillis = 0;
    openLeftDoorShoutdownProccessStartMillis = currentMillis;
  } else if (openLeftDoorShoutdownProccessStartMillis != 0 && currentMillis >= openLeftDoorShoutdownProccessStartMillis + TIME_TO_STOP_MOTORS)  {
    DEBUG_PRINT_LN("Start Left Door Closing after Opening");
    digitalWrite(CLOSE_LEFT_MOTOR_ACTUATOR_RELAY_PIN, HIGH);
    closeLeftDoorChangeSpeedStartMillis = currentMillis;
    openLeftDoorShoutdownProccessStartMillis = 0;
  }  
  if (closeLeftDoorChangeSpeedStartMillis != 0 && currentMillis >= closeLeftDoorChangeSpeedStartMillis + TIME_TO_CHANGE_MOTORS_SPEED) {
    digitalWrite(RELAY_12_36_PIN, HIGH); // Switch to 36v
    closeLeftDoorChangeSpeedStartMillis = 0;
  }

  // Open Right Door
  if (openRightDoorStartMillis != 0 && currentMillis >= openRightDoorStartMillis) {
    if (digitalRead(CLOSE_RIGHT_MOTOR_ACTUATOR_RELAY_PIN) == HIGH) {
      DEBUG_PRINT_LN("Stop Right Door Closing");
      digitalWrite(RELAY_12_36_PIN, LOW); // Switch to 12v
      closeRightDoorShoutdownChangeSpeedStartMillis = currentMillis;
    } else {
      DEBUG_PRINT_LN("Start Right Door Opening");
      digitalWrite(OPEN_RIGHT_MOTOR_ACTUATOR_RELAY_PIN, HIGH);
      openRightDoorChangeSpeedStartMillis = currentMillis;
    }
    openRightDoorStartMillis = 0;
  } else if (closeRightDoorShoutdownChangeSpeedStartMillis != 0 && currentMillis >= closeRightDoorShoutdownChangeSpeedStartMillis + TIME_TO_MOTORS_CLOSE_UP_DOORS)  {
    DEBUG_PRINT_LN("Stop Right Door Closing after change the speed");
    digitalWrite(CLOSE_RIGHT_MOTOR_ACTUATOR_RELAY_PIN, LOW);
    closeRightDoorShoutdownChangeSpeedStartMillis = 0;
    closeRightDoorShoutdownProccessStartMillis = currentMillis;
  } else if (closeRightDoorShoutdownProccessStartMillis != 0 && currentMillis >= closeRightDoorShoutdownProccessStartMillis + TIME_TO_STOP_MOTORS)  {
    DEBUG_PRINT_LN("Start Right Door Opening after Clossing");
    digitalWrite(OPEN_RIGHT_MOTOR_ACTUATOR_RELAY_PIN, HIGH);
    openRightDoorChangeSpeedStartMillis = currentMillis;
    closeRightDoorShoutdownProccessStartMillis = 0;
  }  
  if (openRightDoorChangeSpeedStartMillis != 0 && currentMillis >= openRightDoorChangeSpeedStartMillis + TIME_TO_CHANGE_MOTORS_SPEED) {
    digitalWrite(RELAY_12_36_PIN, HIGH); // Switch to 36v
    openRightDoorChangeSpeedStartMillis = 0;
  }

  // Close Right Door
  if (closeRightDoorStartMillis != 0 && currentMillis >= closeRightDoorStartMillis) {
    if (digitalRead(OPEN_RIGHT_MOTOR_ACTUATOR_RELAY_PIN) == HIGH) {
      DEBUG_PRINT_LN("Stop Right Door Opening");
      digitalWrite(RELAY_12_36_PIN, LOW); // Switch to 12v
      openRightDoorShoutdownChangeSpeedStartMillis = currentMillis;
    } else {
      DEBUG_PRINT_LN("Start Right Door Closing");
      digitalWrite(CLOSE_RIGHT_MOTOR_ACTUATOR_RELAY_PIN, HIGH);
      closeRightDoorChangeSpeedStartMillis = currentMillis;
    }
    closeRightDoorStartMillis = 0;
  } else if (openRightDoorShoutdownChangeSpeedStartMillis != 0 && currentMillis >= openRightDoorShoutdownChangeSpeedStartMillis + TIME_TO_MOTORS_CLOSE_UP_DOORS)  {
    DEBUG_PRINT_LN("Stop Right Door Opening after change the speed");
    digitalWrite(OPEN_RIGHT_MOTOR_ACTUATOR_RELAY_PIN, LOW);
    openRightDoorShoutdownChangeSpeedStartMillis = 0;
    openRightDoorShoutdownProccessStartMillis = currentMillis;
  } else if (openRightDoorShoutdownProccessStartMillis != 0 && currentMillis >= openRightDoorShoutdownProccessStartMillis + TIME_TO_STOP_MOTORS)  {
    DEBUG_PRINT_LN("Start Right Door Closing after Opening");
    digitalWrite(CLOSE_RIGHT_MOTOR_ACTUATOR_RELAY_PIN, HIGH);
    closeRightDoorChangeSpeedStartMillis = currentMillis;
    openRightDoorShoutdownProccessStartMillis = 0;
  }  
  if (closeRightDoorChangeSpeedStartMillis != 0 && currentMillis >= closeRightDoorChangeSpeedStartMillis + TIME_TO_CHANGE_MOTORS_SPEED) {
    digitalWrite(RELAY_12_36_PIN, HIGH); // Switch to 36v
    closeRightDoorChangeSpeedStartMillis = 0;
  }
}

// Variables to handle stop motors
unsigned long stopOpenLeftDoorStartMillis = 0;
unsigned long stopCloseLeftDoorStartMillis = 0;
unsigned long stopOpenRightDoorStartMillis = 0;
unsigned long stopCloseRightDoorStartMillis = 0;

void handleStopsSignals() {
  if (digitalRead(MOTOR_ACTUATOR_RELAY_PIN) == LOW && digitalRead(OPEN_LEFT_MOTOR_ACTUATOR_RELAY_PIN) == HIGH && digitalRead(LEFT_DOOR_OPEN_PIN) == LOW && stopOpenLeftDoorStartMillis == 0) { // Normal Open
    DEBUG_PRINT_LN("Left Door is Opening");
    digitalWrite(RELAY_12_36_PIN, LOW); // Switch to 12v
    stopOpenLeftDoorStartMillis = currentMillis;
  } else if (digitalRead(MOTOR_ACTUATOR_RELAY_PIN) == LOW && stopOpenLeftDoorStartMillis != 0 && currentMillis >= stopOpenLeftDoorStartMillis + TIME_TO_MOTORS_CLOSE_UP_DOORS) {
    digitalWrite(OPEN_LEFT_MOTOR_ACTUATOR_RELAY_PIN, LOW);
    stopOpenLeftDoorStartMillis = 0;
    DEBUG_PRINT_LN("Left Door is Opened");
  }

  if (digitalRead(MOTOR_ACTUATOR_RELAY_PIN) == LOW && digitalRead(CLOSE_LEFT_MOTOR_ACTUATOR_RELAY_PIN) == HIGH && digitalRead(LEFT_DOOR_CLOSE_PIN) == HIGH && stopCloseLeftDoorStartMillis == 0) { // Normal Conected
    DEBUG_PRINT_LN("Left Door is Closing");
    digitalWrite(RELAY_12_36_PIN, LOW); // Switch to 12v
    stopCloseLeftDoorStartMillis = currentMillis;
  } else if (digitalRead(MOTOR_ACTUATOR_RELAY_PIN) == LOW && stopCloseLeftDoorStartMillis != 0 && currentMillis >= stopCloseLeftDoorStartMillis + TIME_TO_MOTORS_CLOSE_UP_DOORS) {
    digitalWrite(CLOSE_LEFT_MOTOR_ACTUATOR_RELAY_PIN, LOW);
    stopCloseLeftDoorStartMillis = 0;
    if (digitalRead(LEFT_DOOR_CLOSE_PIN) == HIGH && digitalRead(RIGHT_DOOR_CLOSE_PIN) == HIGH) {
      lockLeftDoorStartMillis = currentMillis + TIME_TO_STOP_MOTORS; // Start lock proccess
      lockRightDoorStartMillis = currentMillis + TIME_TO_STOP_MOTORS; // Start lock proccess
    }
    DEBUG_PRINT_LN("Left Door is Closed");
  }

  if (digitalRead(MOTOR_ACTUATOR_RELAY_PIN) == LOW && digitalRead(OPEN_RIGHT_MOTOR_ACTUATOR_RELAY_PIN) == HIGH && digitalRead(RIGHT_DOOR_OPEN_PIN) == LOW && stopOpenRightDoorStartMillis == 0) { // Normal Open
    DEBUG_PRINT_LN("Right Door is Opening");
    digitalWrite(RELAY_12_36_PIN, LOW); // Switch to 12v
    stopOpenRightDoorStartMillis = currentMillis;
  } else if (digitalRead(MOTOR_ACTUATOR_RELAY_PIN) == LOW && stopOpenRightDoorStartMillis != 0 && currentMillis >= stopOpenRightDoorStartMillis + TIME_TO_MOTORS_CLOSE_UP_DOORS) {
    digitalWrite(OPEN_RIGHT_MOTOR_ACTUATOR_RELAY_PIN, LOW);
    stopOpenRightDoorStartMillis = 0;
    DEBUG_PRINT_LN("Right Door is Opened");
  }

  if (digitalRead(MOTOR_ACTUATOR_RELAY_PIN) == LOW && digitalRead(CLOSE_RIGHT_MOTOR_ACTUATOR_RELAY_PIN) == HIGH && digitalRead(RIGHT_DOOR_CLOSE_PIN) == HIGH && stopCloseRightDoorStartMillis == 0) { // Normal Conected
    DEBUG_PRINT_LN("Right Door is Closing");
    digitalWrite(RELAY_12_36_PIN, LOW); // Switch to 12v
    stopCloseRightDoorStartMillis = currentMillis;
  } else if (digitalRead(MOTOR_ACTUATOR_RELAY_PIN) == LOW && stopCloseRightDoorStartMillis != 0 && currentMillis >= stopCloseRightDoorStartMillis + TIME_TO_MOTORS_CLOSE_UP_DOORS) {
    digitalWrite(CLOSE_RIGHT_MOTOR_ACTUATOR_RELAY_PIN, LOW);
    stopCloseRightDoorStartMillis = 0;
    if (digitalRead(LEFT_DOOR_CLOSE_PIN) == HIGH && digitalRead(RIGHT_DOOR_CLOSE_PIN) == HIGH) {
      lockLeftDoorStartMillis = currentMillis + TIME_TO_STOP_MOTORS; // Start lock proccess
      lockRightDoorStartMillis = currentMillis + TIME_TO_STOP_MOTORS; // Start lock proccess
    }
    DEBUG_PRINT_LN("Right Door is Closed");
  }
}
