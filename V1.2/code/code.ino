// Conventions followed based on a mix of
// Xbox Core Wireless Controller/Logitech F310 Gamepad: Face Buttons Naming Convention and HID outputs in the Windows Game Controllers tool
// Nintendo Switch Controller: Bumpers Naming Convention
// Sony DualShock 4 Controller: Triggers and Stick Buttons Naming Convention
#include <Joystick.h>

#define DEBUG             false
#define USE_HAT           true
#define NANO_ANALOG       true // Becuase Leonardo Board doesn't have 8 Analog pins like the Nano 33 BLE, the two right stick pins were changed.
#define MAP_ANALOG        (!DEBUG || true)
#define USE_DEADZONE      true
#define SEPARATE_TRIGGERS false

#if USE_HAT
// Buttons
const int NUM_BUTTONS = 10;
int pinToButton    [NUM_BUTTONS] = {  0,   2,   4,   3,  12,  11,       5,        6,  13,     1};
String buttonName  [NUM_BUTTONS] = {"A", "B", "X", "Y", "L", "R", "Start", "Select", "L3", "R3"};
int lastButtonState[NUM_BUTTONS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// D-Pad
const int NUM_HATS = 1;
const int HAT_INDEX = 0;
const int NUM_HAT_BUTTONS = 4;
int pinToHat    [NUM_HAT_BUTTONS] = {   7,       8,      9,     10};
String hatName  [NUM_HAT_BUTTONS] = {"Up", "Right", "Down", "Left"};
int lastHatState[NUM_HAT_BUTTONS] = {0, 0, 0, 0};
int lastHatAngle = JOYSTICK_HATSWITCH_RELEASE;
#else
// Buttons
const int NUM_BUTTONS = 14;
int pinToButton    [NUM_BUTTONS] = {  0,   2,   4,   3,  12,  11,       5,        6,  13,     1,    7,       8,      9,     10};
String buttonName  [NUM_BUTTONS] = {"A", "B", "X", "Y", "L", "R", "Start", "Select", "L3", "R3", "Up", "Right", "Down", "Left"};
int lastButtonState[NUM_BUTTONS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

const int NUM_HATS  = 0;
#endif

// Analog Sticks and Triggers
// Using X axis and Y axis for left movement stick
// Using Rx axis and Ry axis for right stick
// Using either Z axis and Rz axis or Z axis alone for trigger switches
const int NUM_ANALOG = 6;
const int JOYSTICK_POS = 4;
#if NANO_ANALOG
int pinToAnalog    [NUM_ANALOG] = {            A4,             A5,              A6,              A7,   A2,   A3};
#else
int pinToAnalog    [NUM_ANALOG] = {            A4,             A5,              A0,              A1,   A2,   A3};
#endif
String analogName  [NUM_ANALOG] = {"Left Stick X", "Left Stick Y", "Right Stick X", "Right Stick Y", "L2", "R2"};
int lastAnalogState[NUM_ANALOG] = {0, 0, 0, 0, 0, 0};

#define STICK_MIN        10
#define STICK_MAX      1020
#define STICK_DEADZONE   20
#define FSR_MIN          10
#define FSR_MAX         750
#define LIB_MIN        JOYSTICK_DEFAULT_AXIS_MINIMUM
#define LIB_MAX        JOYSTICK_DEFAULT_AXIS_MAXIMUM
int pinRange[NUM_ANALOG][2] = {{STICK_MIN, STICK_MAX}, {STICK_MIN, STICK_MAX}, {STICK_MIN, STICK_MAX}, {STICK_MIN, STICK_MAX}, {FSR_MIN, FSR_MAX}, {FSR_MIN, FSR_MAX}};

// Create the Joystick
#define HAS_X_AXIS      true
#define HAS_Y_AXIS      true
#define HAS_Z_AXIS      true
#define HAS_RX_AXIS     true
#define HAS_RY_AXIS     true
#define HAS_RZ_AXIS     SEPARATE_TRIGGERS
#define HAS_RUDDER      false
#define HAS_THROTTLE    false
#define HAS_ACCELERATOR false
#define HAS_BRAKE       false
#define HAS_STEERING    false

Joystick_ joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_GAMEPAD,
                   NUM_BUTTONS, NUM_HATS, HAS_X_AXIS, HAS_Y_AXIS, HAS_Z_AXIS, HAS_RX_AXIS, HAS_RY_AXIS, HAS_RZ_AXIS,
                   HAS_RUDDER, HAS_THROTTLE, HAS_ACCELERATOR, HAS_BRAKE, HAS_STEERING);

int constrain_map(long x, long oldRangeMin, long oldRangeMax, long newRangeMin, long newRangeMax){
  int newX = constrain(x, oldRangeMin, oldRangeMax);
  return map(newX, oldRangeMin, oldRangeMax, newRangeMin, newRangeMax);
}

void setup() {
  #if !DEBUG
  // Initialise Serial Connection
  Serial.begin(115200);
  #endif

  // Initialise Button Pins
  for (int i = 0; i < NUM_BUTTONS; i++){
    pinMode(pinToButton[i], INPUT_PULLUP);
  }

  #if USE_HAT
  // Initialise Hat Pins
  for (int i = 0; i < NUM_HAT_BUTTONS; i++){
    pinMode(pinToHat[i], INPUT_PULLUP);
  }
  #endif

  // Initialise Joystick Library
  joystick.begin();
}

void loop() {
  // Handle Buttons
  for (int i = 0; i < NUM_BUTTONS; i++) {
    int currentButtonState = !digitalRead(pinToButton[i]);
    #if DEBUG
    Serial.print(buttonName[i] + " = " + currentButtonState + " ");
    #endif
    if (currentButtonState != lastButtonState[i]) {
      joystick.setButton(i, currentButtonState);
      lastButtonState[i] = currentButtonState;
    }
  }
  #if DEBUG
  Serial.println();
  #endif

  #if USE_HAT
  // Handle D-Pad
  bool hatChanged = false;
  for (int i = 0; i < NUM_HAT_BUTTONS; i++) {
    int currentButtonState = !digitalRead(pinToHat[i]);
    #if DEBUG
    Serial.print(hatName[i] + " = "+ currentButtonState+ " ");
    #endif
    if (currentButtonState != lastHatState[i]) {
      hatChanged = true;
      lastHatState[i] = currentButtonState;
    }
  }

  if(hatChanged){
    int numPressed = lastHatState[0] + lastHatState[1] + lastHatState[2] + lastHatState[3];
    // Only allow 1 button or 2 adjacent buttons to be pressed
    if (numPressed < 1 || numPressed > 2) lastHatAngle = JOYSTICK_HATSWITCH_RELEASE;
    // Ordinals or Diagonals
    else if(lastHatState[0] == 1 && lastHatState[1] == 1) lastHatAngle =  45; // NE
    else if(lastHatState[1] == 1 && lastHatState[2] == 1) lastHatAngle = 135; // SE
    else if(lastHatState[2] == 1 && lastHatState[3] == 1) lastHatAngle = 225; // SW
    else if(lastHatState[3] == 1 && lastHatState[0] == 1) lastHatAngle = 315; // NW
    // Invalid Combos: Up + Down, Left + Right
    else if(lastHatState[0] == 1 && lastHatState[2] == 1) lastHatAngle = JOYSTICK_HATSWITCH_RELEASE;
    else if(lastHatState[1] == 1 && lastHatState[3] == 1) lastHatAngle = JOYSTICK_HATSWITCH_RELEASE;
    // Cardinals or Non-Diagonals
    else if(lastHatState[0] == 1) lastHatAngle =   0; // N
    else if(lastHatState[1] == 1) lastHatAngle =  90; // E
    else if(lastHatState[2] == 1) lastHatAngle = 180; // S
    else if(lastHatState[3] == 1) lastHatAngle = 270; // W
    else lastHatAngle = JOYSTICK_HATSWITCH_RELEASE;
    joystick.setHatSwitch(HAT_INDEX, lastHatAngle);
  }

  #if DEBUG
  Serial.print("Hat Angle = ");
  Serial.println(lastHatAngle);
  #endif

  #endif

  // Handle Analog
  for (int i = 0; i < NUM_ANALOG; i++) {
    int currentState = analogRead(pinToAnalog[i]);
    #if MAP_ANALOG
    currentState = constrain_map(currentState, pinRange[i][0], pinRange[i][1], LIB_MIN, LIB_MAX);
    #endif
    #if USE_DEADZONE
    if(i < JOYSTICK_POS && ((LIB_MAX / 2 - STICK_DEADZONE) < currentState && currentState  < (LIB_MAX / 2 + STICK_DEADZONE))) currentState = 512;
    #endif
    #if DEBUG
    Serial.print(analogName[i] + " = " + currentState + " ");
    #endif
    lastAnalogState[i] = currentState;
  }
  #if DEBUG
  Serial.println();
  #endif

  // Left Stick
  joystick.setXAxis(lastAnalogState[0]);
  joystick.setYAxis(lastAnalogState[1]);
  // Right Stick
  joystick.setRxAxis(lastAnalogState[2]);
  joystick.setRyAxis(lastAnalogState[3]);
  // Triggers
  #if !MAP_ANALOG
  // Since FSR range is not 0 - 1023 we need to always map if we haven't already
  lastAnalogState[4] = constrain_map(lastAnalogState[4], FSR_MIN, FSR_MAX, LIB_MIN, LIB_MAX);
  lastAnalogState[5] = constrain_map(lastAnalogState[5], FSR_MIN, FSR_MAX, LIB_MIN, LIB_MAX);
  #endif
  #if SEPARATE_TRIGGERS
  joystick.setZAxis(lastAnalogState[4]);
  joystick.setRzAxis(lastAnalogState[5]);
  #else
  lastAnalogState[4] = map(lastAnalogState[4], LIB_MIN, LIB_MAX, LIB_MIN, LIB_MAX / 2);
  lastAnalogState[5] = map(lastAnalogState[5], LIB_MIN, LIB_MAX, LIB_MIN, LIB_MAX / 2);
  int stickValue = map(512 + lastAnalogState[4] - lastAnalogState[5], LIB_MIN + 1, LIB_MAX, LIB_MIN, LIB_MAX);
  joystick.setZAxis(stickValue);
  #endif

  #if DEBUG
  // Delay to read Serial console
  delay(1000);
  #else
  // Delay for responsive controls
  delay(16);
  #endif
}
