#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RCSwitch.h>
#include "util.h"

RCSwitch mySwitch = RCSwitch();

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);









// Lock States
#define LOCK_STATE_UNKNOWN -1
#define LOCK_STATE_CLOSED 0
#define LOCK_STATE_OPENED_LEFT_DOOR 1
#define LOCK_STATE_OPENED_LEFT_AND_RIGHT_DOORS 2

// Left Door States
#define LEFT_DOOR_STATE_UNKNOWN -1
#define LEFT_DOOR_STATE_CLOSED 0
#define LEFT_DOOR_STATE_OPENED 1
#define LEFT_DOOR_STATE_OPENING 2
#define LEFT_DOOR_STATE_CLOSING 3

// Right Door States
#define RIGHT_DOOR_STATE_UNKNOWN -1
#define RIGHT_DOOR_STATE_CLOSED 0
#define RIGHT_DOOR_STATE_OPENED 1
#define RIGHT_DOOR_STATE_OPENING 2
#define RIGHT_DOOR_STATE_CLOSING 3



unsigned long previousMillis = 0;
const long interval = 100;
unsigned long counter = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  DEBUG_PRINT_LN("Hi!");
  mySwitch.enableReceive(RC_PIN);
  setupPins();

  checkLeftDoorStatus();
  checkRightDoorStatus();

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    DEBUG_PRINT_LN(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  display.clearDisplay();

}


// Variables to handle door state
int8_t currentLockState = LOCK_STATE_UNKNOWN;
int8_t currentLeftDoorState = LEFT_DOOR_STATE_UNKNOWN;
int8_t currentRightDoorState = RIGHT_DOOR_STATE_UNKNOWN;
int8_t previousLockState = currentLockState;
int8_t previousLeftDoorState = currentLeftDoorState;
int8_t previousRightDoorState = currentRightDoorState;

// Variables to handle changes in Request commands
int currentRequestCommand = REQUEST_COMMAND_UNKNOWN;
int previousRequestCommand = currentRequestCommand;


void loop() {
  currentMillis = millis();

  handleStopsSignals();
  checkRequestCommand();
  handleRequestCommandChanges();
















  handleLookUnlockCommands();
  handleDoorCommands();

  updateDisplay();

  previousMillis = currentMillis;


  previousLockState = currentLockState;
  previousLeftDoorState = currentLeftDoorState;
  previousRightDoorState = currentRightDoorState;

  previousRequestCommand = currentRequestCommand;
}





void handleRequestCommandChanges() {
  if (previousRequestCommand != currentRequestCommand) {

    DEBUG_PRINT("Identified a new request command: ");
    Serial.println(getRequestCommandName(currentRequestCommand));

    if (currentRequestCommand == REQUEST_COMMAND_TO_OPEN) {
      if (canOpenLeftDoor()) {
        openLeftLockDoorStartMillis = currentMillis; // Start unlock process for left door
        openLeftDoorStartMillis = currentMillis + TIME_TO_LOCK_UNLOCK_DOORS; // Start open left door proccess
      }
      if (canOpenRightDoor()) {
        openRightLockDoorStartMillis = currentMillis; // Start unlock process for right door
        openRightDoorStartMillis = currentMillis + TIME_TO_LOCK_UNLOCK_DOORS; // Start open right door proccess
      }
    } else if (currentRequestCommand == REQUEST_COMMAND_TO_CLOSE) {
      if (canCloseLeftDoor()) {
        openLeftLockDoorStartMillis = currentMillis; // Start unlock process for left door
        closeLeftDoorStartMillis = currentMillis + TIME_TO_LOCK_UNLOCK_DOORS; // Start open left door proccess
      }
      if (canCloseRightDoor()) {
        openRightLockDoorStartMillis = currentMillis; // Start unlock process for right door
        closeRightDoorStartMillis = currentMillis + TIME_TO_LOCK_UNLOCK_DOORS; // Start open right door proccess
      }
    } else if (currentRequestCommand == REQUEST_COMMAND_TO_OPEN_LEFT_DOOR) {
      if (canOpenLeftDoor()) {
        openLeftLockDoorStartMillis = currentMillis; // Start unlock process for left door
        openLeftDoorStartMillis = currentMillis + TIME_TO_LOCK_UNLOCK_DOORS; // Start open left door proccess
      }
    }
  }
}




void setupPins() {
  pinMode(MCU_OPEN_PIN, INPUT);
  pinMode(MCU_CLOSE_PIN, INPUT);
  pinMode(MCU_TRUNK_PIN, INPUT);
  pinMode(MCU_TURN_PIN, INPUT);

  pinMode(OPEN_LEFT_MOTOR_RELAY_PIN, OUTPUT);
  digitalWrite(OPEN_LEFT_MOTOR_RELAY_PIN, LOW);
  pinMode(CLOSE_LEFT_MOTOR_RELAY_PIN, OUTPUT);
  digitalWrite(CLOSE_LEFT_MOTOR_RELAY_PIN, LOW);
  pinMode(OPEN_RIGHT_MOTOR_RELAY_PIN, OUTPUT);
  digitalWrite(OPEN_RIGHT_MOTOR_RELAY_PIN, LOW);
  pinMode(CLOSE_RIGHT_MOTOR_RELAY_PIN, OUTPUT);
  digitalWrite(CLOSE_RIGHT_MOTOR_RELAY_PIN, LOW);

  pinMode(OPEN_LEFT_DOOR_RELAY_PIN, OUTPUT);
  digitalWrite(OPEN_LEFT_DOOR_RELAY_PIN, LOW);
  pinMode(CLOSE_LEFT_DOOR_RELAY_PIN, OUTPUT);
  digitalWrite(CLOSE_LEFT_DOOR_RELAY_PIN, LOW);
  pinMode(OPEN_CLOSE_RIGHT_DOOR_RELAY_PIN, OUTPUT);
  digitalWrite(OPEN_CLOSE_RIGHT_DOOR_RELAY_PIN, LOW);

  pinMode(TURN_LIGHT_RELAY_PIN, OUTPUT);
  digitalWrite(TURN_LIGHT_RELAY_PIN, LOW);
  pinMode(OBSTACLE_LAMP_PIN, OUTPUT);
  digitalWrite(OBSTACLE_LAMP_PIN, LOW);

  pinMode(INTERIOR_LIGHT_RELAY_PIN, OUTPUT);
  digitalWrite(INTERIOR_LIGHT_RELAY_PIN, LOW);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  pinMode(LEFT_DOOR_OPEN_PIN, INPUT_PULLDOWN);
  pinMode(LEFT_DOOR_CLOSE_PIN, INPUT_PULLDOWN);
  pinMode(RIGHT_DOOR_OPEN_PIN, INPUT_PULLDOWN);
  pinMode(RIGHT_DOOR_CLOSE_PIN, INPUT_PULLDOWN);

}



void checkLeftDoorStatus() {
  if (digitalRead(LEFT_DOOR_CLOSE_PIN) == HIGH) {
    DEBUG_PRINT_LN("Left Door is closed");
    currentLeftDoorState = LEFT_DOOR_STATE_CLOSED;
  } else if (digitalRead(LEFT_DOOR_OPEN_PIN) == HIGH) {
    DEBUG_PRINT_LN("Left Door is opened");
    currentLeftDoorState = LEFT_DOOR_STATE_OPENED;
  } else if (digitalRead(LEFT_DOOR_CLOSE_PIN) == LOW && digitalRead(OPEN_LEFT_MOTOR_RELAY_PIN) == HIGH) {
    DEBUG_PRINT_LN("Left Door is opening");
    currentLeftDoorState = LEFT_DOOR_STATE_OPENING;
  } else if (digitalRead(LEFT_DOOR_OPEN_PIN) == LOW && digitalRead(CLOSE_LEFT_MOTOR_RELAY_PIN) == HIGH) {
    DEBUG_PRINT_LN("Left Door is closing");
    currentLeftDoorState = LEFT_DOOR_STATE_CLOSING;
  }
}

void checkRightDoorStatus() {
  if (digitalRead(RIGHT_DOOR_CLOSE_PIN) == HIGH) {
    DEBUG_PRINT_LN("Right Door is closed");
    currentLeftDoorState = RIGHT_DOOR_STATE_CLOSED;
  } else if (digitalRead(RIGHT_DOOR_OPEN_PIN) == HIGH) {
    DEBUG_PRINT_LN("Right Door is opened");
    currentLeftDoorState = RIGHT_DOOR_STATE_OPENED;
  } else if (digitalRead(RIGHT_DOOR_CLOSE_PIN) == LOW && digitalRead(OPEN_RIGHT_MOTOR_RELAY_PIN) == HIGH) {
    DEBUG_PRINT_LN("Right Door is opening");
    currentLeftDoorState = RIGHT_DOOR_STATE_OPENING;
  } else if (digitalRead(RIGHT_DOOR_OPEN_PIN) == LOW && digitalRead(CLOSE_RIGHT_MOTOR_RELAY_PIN) == HIGH) {
    DEBUG_PRINT_LN("Right Door is closing");
    currentLeftDoorState = RIGHT_DOOR_STATE_CLOSING;
  }
}

void checkRequestCommand() {

  unsigned long receivedValue = 0;

  if (mySwitch.available()) {
    receivedValue = mySwitch.getReceivedValue();
    DEBUG_PRINT("Received Value: ");
    Serial.println(receivedValue);
    if (receivedValue == KEY_1_CODE || receivedValue == KEY_2_CODE || receivedValue == KEY_3_CODE) {
      DEBUG_PRINT_LN("Received Close Command");
      currentRequestCommand = REQUEST_COMMAND_TO_CLOSE;
    }

    if (receivedValue == KEY_1_CODE + 1 || receivedValue == KEY_2_CODE + 1 || receivedValue == KEY_3_CODE + 1) {
      DEBUG_PRINT_LN("Received Open Command");
      currentRequestCommand = REQUEST_COMMAND_TO_OPEN;
    }
    if (receivedValue == KEY_1_CODE + 3 || receivedValue == KEY_2_CODE + 3 || receivedValue == KEY_3_CODE + 3) {
      DEBUG_PRINT_LN("Received Open Left Door Command");
      currentRequestCommand = REQUEST_COMMAND_TO_OPEN_LEFT_DOOR;
    }

    mySwitch.resetAvailable();
  }

}





void updateDisplay() {
  counter = counter + 1;
  display.clearDisplay();
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.println(counter);
  display.print("Request State: ");  display.println(currentRequestCommand);

  display.print("MCU Turn: ");  display.println(digitalRead(MCU_TURN_PIN));
  display.print("MCU Open: ");  display.println(digitalRead(MCU_OPEN_PIN));
  display.print("MCU Close: ");  display.println(digitalRead(MCU_CLOSE_PIN));
  display.print("MCU Trunk: ");  display.println(digitalRead(MCU_TRUNK_PIN));

  display.display();
}
