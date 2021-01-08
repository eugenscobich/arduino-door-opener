#include "config.h"

// PINs
#define RC_PIN PB12
#define INTERIOR_LIGHT_RELAY_PIN PB13


#define OPEN_CLOSE_BUTTON_PIN PB3


#define LEFT_DOOR_OPEN_PIN PB11
#define LEFT_DOOR_CLOSE_PIN PB10
#define RIGHT_DOOR_OPEN_PIN PA11
#define RIGHT_DOOR_CLOSE_PIN PA12

#define MOTOR_ACTUATOR_RELAY_PIN PA6
#define 12_36_RELAY_PIN PA5

#define OPEN_LEFT_MOTOR_ACTUATOR_RELAY_PIN PA4
#define CLOSE_LEFT_MOTOR_ACTUATOR_RELAY_PIN PA3
#define OPEN_RIGHT_MOTOR_ACTUATOR_RELAY_PIN PA2
#define CLOSE_RIGHT_MOTOR_ACTUATOR_RELAY_PIN PA1

// Deprecated
//#define OPEN_LEFT_MOTOR_RELAY_PIN PA4
//#define CLOSE_LEFT_MOTOR_RELAY_PIN PA3
//#define OPEN_RIGHT_MOTOR_RELAY_PIN PA2
//#define CLOSE_RIGHT_MOTOR_RELAY_PIN PA1


#define SIGNAL_LAMP_PIN PA0
#define CENTRAL_LAM PC15
#define BUZZER_PIN PC14

// Request Command
#define REQUEST_COMMAND_UNKNOWN -1
#define REQUEST_COMMAND_TO_OPEN 0
#define REQUEST_COMMAND_TO_CLOSE 1
#define REQUEST_COMMAND_TO_OPEN_LEFT_DOOR 2

#define DEBUG_PRINT_LN(str)    \
   Serial.print(millis());     \
   Serial.print(": ");         \
   Serial.println(str);

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
    if (digitalRead(OPEN_LEFT_MOTOR_RELAY_PIN) == HIGH || digitalRead(LEFT_DOOR_OPEN_PIN) == HIGH) {
      DEBUG_PRINT_LN("Cannot Open Left Door");
      return false;
    }
    return true;
}

bool canOpenRightDoor() {
    if (digitalRead(OPEN_RIGHT_MOTOR_RELAY_PIN) == HIGH || digitalRead(RIGHT_DOOR_OPEN_PIN) == HIGH) {
      DEBUG_PRINT_LN("Cannot Open Right Door");
      return false;
    }
    return true;
}

bool canCloseLeftDoor() {
    if (digitalRead(CLOSE_LEFT_MOTOR_RELAY_PIN) == HIGH || digitalRead(LEFT_DOOR_CLOSE_PIN) == HIGH) {
      DEBUG_PRINT_LN("Cannot Close Left Door");
      return false;
    }
    return true;
}

bool canCloseRightDoor() {
    if (digitalRead(CLOSE_RIGHT_MOTOR_RELAY_PIN) == HIGH || digitalRead(RIGHT_DOOR_CLOSE_PIN) == HIGH) {
      DEBUG_PRINT_LN("Cannot Close Right Door");
      return false;
    }
    return true;
}



// Variables to react on Request Command and handle lock/unlock doors
unsigned long openLeftLockDoorStartMillis = 0;
unsigned long openRightLockDoorStartMillis = 0;
unsigned long closeLeftLockDoorStartMillis = 0;
unsigned long closeRightLockDoorStartMillis = 0;
void handleLookUnlockCommands() {
  if (openLeftLockDoorStartMillis == currentMillis) {
    DEBUG_PRINT_LN("Unlock Left Door: Start");
    digitalWrite(CLOSE_LEFT_DOOR_RELAY_PIN, LOW);
    digitalWrite(OPEN_LEFT_DOOR_RELAY_PIN, HIGH);
  } else if (openLeftLockDoorStartMillis != 0 && currentMillis >= openLeftLockDoorStartMillis + TIME_TO_LOCK_UNLOCK_DOORS) {
    DEBUG_PRINT_LN("Unlock Left Door: Finished");
    digitalWrite(OPEN_LEFT_DOOR_RELAY_PIN, LOW);
    openLeftLockDoorStartMillis = 0;
  }

  if (openRightLockDoorStartMillis == currentMillis) {
    DEBUG_PRINT_LN("Unlock Right Door: Start");
    digitalWrite(CLOSE_LEFT_DOOR_RELAY_PIN, LOW);
    digitalWrite(OPEN_LEFT_DOOR_RELAY_PIN, HIGH);
    digitalWrite(OPEN_CLOSE_RIGHT_DOOR_RELAY_PIN, HIGH);
  } else if (openRightLockDoorStartMillis != 0 && currentMillis >= openRightLockDoorStartMillis + TIME_TO_LOCK_UNLOCK_DOORS) {
    DEBUG_PRINT_LN("Unlock Right Door: Finished");
    digitalWrite(OPEN_LEFT_DOOR_RELAY_PIN, LOW);
    digitalWrite(OPEN_CLOSE_RIGHT_DOOR_RELAY_PIN, LOW);
    openRightLockDoorStartMillis = 0;
  }

  if (closeLeftLockDoorStartMillis != 0 && currentMillis >= closeLeftLockDoorStartMillis && digitalRead(CLOSE_LEFT_DOOR_RELAY_PIN) == LOW) {
    DEBUG_PRINT_LN("Lock Left Door: Start");
    digitalWrite(OPEN_LEFT_DOOR_RELAY_PIN, LOW);
    digitalWrite(CLOSE_LEFT_DOOR_RELAY_PIN, HIGH);
  } else if (closeLeftLockDoorStartMillis != 0 && currentMillis >= closeLeftLockDoorStartMillis + TIME_TO_LOCK_UNLOCK_DOORS) {
    DEBUG_PRINT_LN("Lock Left Door: Finished");
    digitalWrite(CLOSE_LEFT_DOOR_RELAY_PIN, LOW);
    closeLeftLockDoorStartMillis = 0;
  }

  if (closeRightLockDoorStartMillis != 0 && currentMillis >= closeRightLockDoorStartMillis && digitalRead(CLOSE_LEFT_DOOR_RELAY_PIN) == LOW) {
    DEBUG_PRINT_LN("Lock Right Door: Start");
    digitalWrite(OPEN_LEFT_DOOR_RELAY_PIN, LOW);
    digitalWrite(CLOSE_LEFT_DOOR_RELAY_PIN, HIGH);
    digitalWrite(OPEN_CLOSE_RIGHT_DOOR_RELAY_PIN, HIGH);
  } else if (closeRightLockDoorStartMillis != 0 && currentMillis >= closeRightLockDoorStartMillis + TIME_TO_LOCK_UNLOCK_DOORS) {
    DEBUG_PRINT_LN("Lock Right Door: Finished");
    digitalWrite(CLOSE_LEFT_DOOR_RELAY_PIN, LOW);
    digitalWrite(OPEN_CLOSE_RIGHT_DOOR_RELAY_PIN, LOW);
    closeRightLockDoorStartMillis = 0;
  }  
}




// Variables to react on Request Command and handle doors oppening
unsigned long openLeftDoorStartMillis = 0;
unsigned long closeLeftDoorShoutdownProccessStartMillis = 0;
unsigned long openRightDoorStartMillis = 0;
unsigned long closeRightDoorShoutdownProccessStartMillis = 0;
unsigned long closeLeftDoorStartMillis = 0;
unsigned long openLeftDoorShoutdownProccessStartMillis = 0;
unsigned long closeRightDoorStartMillis = 0;
unsigned long openRightDoorShoutdownProccessStartMillis = 0;

void handleDoorCommands() {
  // Open Left Door
  if (openLeftDoorStartMillis != 0 && currentMillis >= openLeftDoorStartMillis) {
    if (digitalRead(CLOSE_LEFT_MOTOR_RELAY_PIN) == HIGH) {
      DEBUG_PRINT_LN("Stop Left Door Closing");
      digitalWrite(CLOSE_LEFT_MOTOR_RELAY_PIN, LOW);
      closeLeftDoorShoutdownProccessStartMillis = currentMillis;
    } else {
      DEBUG_PRINT_LN("Start Left Door Opening");
      digitalWrite(OPEN_LEFT_MOTOR_RELAY_PIN, HIGH);
    }
    openLeftDoorStartMillis = 0;
  } else if (closeLeftDoorShoutdownProccessStartMillis != 0 && currentMillis >= closeLeftDoorShoutdownProccessStartMillis + TIME_TO_STOP_MOTORS)  {
    DEBUG_PRINT_LN("Start Left Door Opening after Clossing");
    digitalWrite(OPEN_LEFT_MOTOR_RELAY_PIN, HIGH);
    closeLeftDoorShoutdownProccessStartMillis = 0;
  }

  // Open Right Door
  if (openRightDoorStartMillis != 0 && currentMillis >= openRightDoorStartMillis) {
    if (digitalRead(CLOSE_RIGHT_MOTOR_RELAY_PIN) == HIGH) {
      DEBUG_PRINT_LN("Stop Right Door Closing");
      digitalWrite(CLOSE_RIGHT_MOTOR_RELAY_PIN, LOW);
      closeRightDoorShoutdownProccessStartMillis = currentMillis;
    } else {
      DEBUG_PRINT_LN("Start Right Door Opening");
      digitalWrite(OPEN_RIGHT_MOTOR_RELAY_PIN, HIGH);
    }
    openRightDoorStartMillis = 0;
  } else if (closeRightDoorShoutdownProccessStartMillis != 0 && currentMillis >= closeRightDoorShoutdownProccessStartMillis + TIME_TO_STOP_MOTORS)  {
    DEBUG_PRINT_LN("Start Right Door Opening after Clossing");
    digitalWrite(OPEN_RIGHT_MOTOR_RELAY_PIN, HIGH);
    closeRightDoorShoutdownProccessStartMillis = 0;
  }

  // Close Left Door
  if (closeLeftDoorStartMillis != 0 && currentMillis >= closeLeftDoorStartMillis) {
    if (digitalRead(OPEN_LEFT_MOTOR_RELAY_PIN) == HIGH) {
      DEBUG_PRINT_LN("Stop Left Door Opening");
      digitalWrite(OPEN_LEFT_MOTOR_RELAY_PIN, LOW);
      openLeftDoorShoutdownProccessStartMillis = currentMillis;
    } else {
      DEBUG_PRINT_LN("Start Left Door Closing");
      digitalWrite(CLOSE_LEFT_MOTOR_RELAY_PIN, HIGH);
    }
    closeLeftDoorStartMillis = 0;
  } else if (openLeftDoorShoutdownProccessStartMillis != 0 && currentMillis >= openLeftDoorShoutdownProccessStartMillis + TIME_TO_STOP_MOTORS)  {
    DEBUG_PRINT_LN("Start Left Door Closing after Oppening");
    digitalWrite(CLOSE_LEFT_MOTOR_RELAY_PIN, HIGH);
    openLeftDoorShoutdownProccessStartMillis = 0;
  }

  
  // Close Right Door
  if (closeRightDoorStartMillis != 0 && currentMillis >= closeRightDoorStartMillis) {
    if (digitalRead(OPEN_RIGHT_MOTOR_RELAY_PIN) == HIGH) {
      DEBUG_PRINT_LN("Stop Right Door Opening");
      digitalWrite(OPEN_RIGHT_MOTOR_RELAY_PIN, LOW);
      openRightDoorShoutdownProccessStartMillis = currentMillis;
    } else {
      DEBUG_PRINT_LN("Start Right Door Closing");
      digitalWrite(CLOSE_RIGHT_MOTOR_RELAY_PIN, HIGH);
    }
    closeRightDoorStartMillis = 0;
  } else if (openRightDoorShoutdownProccessStartMillis != 0 && currentMillis >= openRightDoorShoutdownProccessStartMillis + TIME_TO_STOP_MOTORS)  {
    DEBUG_PRINT_LN("Start Right Door Closing after Oppening");
    digitalWrite(CLOSE_RIGHT_MOTOR_RELAY_PIN, HIGH);
    openRightDoorShoutdownProccessStartMillis = 0;
  }
}

void handleStopsSignals() {
  if (digitalRead(LEFT_DOOR_OPEN_PIN) == HIGH && digitalRead(OPEN_LEFT_MOTOR_RELAY_PIN) == HIGH) {
    DEBUG_PRINT_LN("Left Door is Opened");
    digitalWrite(OPEN_LEFT_MOTOR_RELAY_PIN, LOW);
  }

  if (digitalRead(LEFT_DOOR_CLOSE_PIN) == HIGH && digitalRead(CLOSE_LEFT_MOTOR_RELAY_PIN) == HIGH) {
    DEBUG_PRINT_LN("Left Door is Closed");
    closeLeftLockDoorStartMillis = currentMillis + TIME_TO_STOP_MOTORS;
    digitalWrite(CLOSE_LEFT_MOTOR_RELAY_PIN, LOW);
  }

  if (digitalRead(RIGHT_DOOR_OPEN_PIN) == HIGH && digitalRead(OPEN_RIGHT_MOTOR_RELAY_PIN) == HIGH) {
    DEBUG_PRINT_LN("Right Door is Opened");
    digitalWrite(OPEN_RIGHT_MOTOR_RELAY_PIN, LOW);
  }

  if (digitalRead(RIGHT_DOOR_CLOSE_PIN) == HIGH && digitalRead(CLOSE_RIGHT_MOTOR_RELAY_PIN) == HIGH) {
    DEBUG_PRINT_LN("Right Door is Closed");
    closeRightLockDoorStartMillis = currentMillis + TIME_TO_STOP_MOTORS;
    digitalWrite(CLOSE_RIGHT_MOTOR_RELAY_PIN, LOW);
  }
}
