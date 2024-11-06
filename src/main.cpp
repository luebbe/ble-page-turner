#include <Arduino.h>

// LED_OFF is LOW for DOIT board, HIGH for LOLIN32, opposite for LED_ON
#define LED_OFF LOW
#define LED_ON HIGH

#define NAME "BTPageTurner"
#define AUTHOR "Lübbe Onken"

#include <BleKeyboard.h>
#include <Keypad.h>
#include <SimpleFSM.h>

const byte BLE_DELAY = 10; // Delay to prevent BT congestion

const byte battPin = 34; // 1/2 the battery voltage read on this pin

// Could do this without the keypad library fairly simply but this allows easy expansion
const byte ROWS = 1; // Just the one row
const byte COLS = 2; // Two columns

#define LEFT 'L'
#define RIGHT 'R'

// the library will return the character inside this array
// when the appropriate button is pressed.
char keys[ROWS][COLS] = {
    {LEFT, RIGHT}};

byte rowPins[ROWS] = {4};      // Connect to the row pinouts of the keypad
byte colPins[COLS] = {16, 17}; // Connect to the column pinouts of the keypad

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// First param is name, second is manufacturer, third is initial battery level
BleKeyboard bleKeyboard(NAME, AUTHOR, 100);

// Finite state machine for transitions
SimpleFSM fsm;

// Key bindings for the BLE keyboard
// left without right
#define KBD_LEFT_PRESS KEY_UP_ARROW
#define KBD_LEFT_HOLD KEY_PAGE_UP
// right without left
#define KBD_RIGHT_PRESS KEY_DOWN_ARROW
#define KBD_RIGHT_HOLD KEY_PAGE_DOWN
// left + right simultaneously
#define KBD_BOTH_PRESS KEY_LEFT_CTRL
#define KBD_BOTH_HOLD KEY_RIGHT_CTRL
// left hold + right press/hold
#define KBD_LEFT_RIGHT_PRESS KEY_RIGHT_ALT
#define KBD_LEFT_RIGHT_HOLD KEY_RIGHT_SHIFT
// right hold + left press/hold
#define KBD_RIGHT_LEFT_PRESS KEY_LEFT_ALT
#define KBD_RIGHT_LEFT_HOLD KEY_LEFT_SHIFT

void on_change()
{
  Serial.println(fsm.getState()->getName());
}

void on_idle()
{
  Serial.println("---");
}

void on_leftDown()
{
  on_change();
  bleKeyboard.write(KBD_LEFT_PRESS);
}

void on_leftHold()
{
  on_change();
  bleKeyboard.write(KBD_LEFT_HOLD);
}

void on_rightDown()
{
  on_change();
  bleKeyboard.write(KBD_RIGHT_PRESS);
}

void on_rightHold()
{
  on_change();
  bleKeyboard.write(KBD_RIGHT_HOLD);
}

void on_bothDown()
{
  on_change();
  bleKeyboard.write(KBD_BOTH_PRESS);
}

void on_bothHold()
{
  on_change();
  bleKeyboard.write(KBD_BOTH_HOLD);
}

void on_leftRightDown()
{
  on_change();
  bleKeyboard.write(KBD_LEFT_RIGHT_PRESS);
}

void on_leftRightHold()
{
  on_change();
  bleKeyboard.write(KBD_LEFT_RIGHT_HOLD);
}

void on_rightLeftDown()
{
  on_change();
  bleKeyboard.write(KBD_RIGHT_LEFT_PRESS);
}

void on_rightLeftHold()
{
  on_change();
  bleKeyboard.write(KBD_RIGHT_LEFT_HOLD);
}

enum triggers
{
  trgLeftDown,
  trgLeftHold,
  trgLeftReleased,
  trgLeftIdle,
  trgRightDown,
  trgRightHold,
  trgRightReleased,
  trgRightIdle
};

enum states
{
  stIdle,
  // left without right
  stLeftDown,
  stLeftHold,
  stLeftDownReleased,
  stLeftHoldReleased,
  // right without left
  stRightDown,
  stRightHold,
  stRightDownReleased,
  stRightHoldReleased,
  // left + right simultaneously
  stBothDown,
  stBothHold,
  stBothDownReleased,
  stBothHoldReleased,
  // left hold + right press/hold
  stLeftHoldRightDown,
  stLeftHoldRightHold,
  stLeftHoldRightDownReleased,
  stLeftHoldRightHoldReleased,
  stLeftHoldRightIdle,
  // right hold + left press/hold
  stRightHoldLeftDown,
  stRightHoldLeftHold,
  stRightHoldLeftDownReleased,
  stRightHoldLeftHoldReleased,
  stRightHoldLeftIdle,
};

State s[] = {
    State("Idle", on_idle),
    // left without right
    State("LD", on_change),
    State("LH", on_change),
    State("LDR", on_leftDown),
    State("LHR", on_leftHold),
    // right without left
    State("RD", on_change),
    State("RH", on_change),
    State("RDR", on_rightDown),
    State("RHR", on_rightHold),
    // left + right simultaneously
    State("BD", on_change),
    State("BH", on_change),
    State("BDR", on_bothDown),
    State("BHR", on_bothHold),
    // left hold + right press/hold
    State("LHRD", on_change),
    State("LHRH", on_change),
    State("LHRDR", on_leftRightDown),
    State("LHRHR", on_leftRightHold),
    State("LHRI", on_change),
    // right hold + left press/hold
    State("RHLD", on_change),
    State("RHLH", on_change),
    State("RHLDR", on_rightLeftDown),
    State("RHLHR", on_rightLeftHold),
    State("RHLI", on_change),
};

Transition transitions[] = {
    // left without right
    Transition(&s[stIdle], &s[stLeftDown], trgLeftDown),
    Transition(&s[stLeftDown], &s[stLeftHold], trgLeftHold),
    Transition(&s[stLeftDown], &s[stLeftDownReleased], trgLeftReleased),
    Transition(&s[stLeftHold], &s[stLeftHoldReleased], trgLeftReleased),
    Transition(&s[stLeftDownReleased], &s[stIdle], trgLeftIdle), // Return to idle, when single key
    Transition(&s[stLeftHoldReleased], &s[stIdle], trgLeftIdle),

    // right without left
    Transition(&s[stIdle], &s[stRightDown], trgRightDown),
    Transition(&s[stRightDown], &s[stRightHold], trgRightHold),
    Transition(&s[stRightDown], &s[stRightDownReleased], trgRightReleased),
    Transition(&s[stRightHold], &s[stRightHoldReleased], trgRightReleased),
    Transition(&s[stRightDownReleased], &s[stIdle], trgRightIdle), // Return to idle, when single key
    Transition(&s[stRightHoldReleased], &s[stIdle], trgRightIdle),

    // left + right simultaneously
    Transition(&s[stRightDown], &s[stBothDown], trgLeftDown),
    Transition(&s[stLeftDown], &s[stBothDown], trgRightDown),
    Transition(&s[stBothDown], &s[stBothHold], trgLeftHold),
    Transition(&s[stBothDown], &s[stBothHold], trgRightHold),
    Transition(&s[stBothDown], &s[stBothDownReleased], trgLeftReleased),
    Transition(&s[stBothDown], &s[stBothDownReleased], trgRightReleased),
    Transition(&s[stBothHold], &s[stBothHoldReleased], trgLeftReleased),
    Transition(&s[stBothHold], &s[stBothHoldReleased], trgRightReleased),
    Transition(&s[stBothDownReleased], &s[stIdle], trgLeftIdle), // Return to idle, when any of the keys becomes idle
    Transition(&s[stBothDownReleased], &s[stIdle], trgRightIdle),
    Transition(&s[stBothHoldReleased], &s[stIdle], trgLeftIdle),
    Transition(&s[stBothHoldReleased], &s[stIdle], trgRightIdle),

    // left hold + right press/hold
    Transition(&s[stLeftHold], &s[stLeftHoldRightDown], trgRightDown),
    Transition(&s[stLeftHoldRightDown], &s[stLeftHoldRightHold], trgRightHold),
    Transition(&s[stLeftHoldRightDown], &s[stLeftHoldRightDownReleased], trgRightReleased),
    Transition(&s[stLeftHoldRightHold], &s[stLeftHoldRightHoldReleased], trgRightReleased),
    Transition(&s[stLeftHoldRightDownReleased], &s[stLeftHoldRightIdle], trgRightIdle),
    Transition(&s[stLeftHoldRightHoldReleased], &s[stLeftHoldRightIdle], trgRightIdle),
    Transition(&s[stLeftHoldRightIdle], &s[stLeftHoldRightDown], trgRightDown),
    Transition(&s[stLeftHoldRightDown], &s[stIdle], trgLeftIdle), // Return to idle, when the left key is released
    Transition(&s[stLeftHoldRightHold], &s[stIdle], trgLeftIdle),
    Transition(&s[stLeftHoldRightIdle], &s[stIdle], trgLeftIdle),
    Transition(&s[stLeftHoldRightDownReleased], &s[stIdle], trgLeftIdle),
    Transition(&s[stLeftHoldRightHoldReleased], &s[stIdle], trgLeftIdle),

    // right hold + left press/hold
    Transition(&s[stRightHold], &s[stRightHoldLeftDown], trgLeftDown),
    Transition(&s[stRightHoldLeftDown], &s[stRightHoldLeftHold], trgLeftHold),
    Transition(&s[stRightHoldLeftDown], &s[stRightHoldLeftDownReleased], trgLeftReleased),
    Transition(&s[stRightHoldLeftHold], &s[stRightHoldLeftHoldReleased], trgLeftReleased),
    Transition(&s[stRightHoldLeftDownReleased], &s[stRightHoldLeftIdle], trgLeftIdle),
    Transition(&s[stRightHoldLeftHoldReleased], &s[stRightHoldLeftIdle], trgLeftIdle),
    Transition(&s[stRightHoldLeftIdle], &s[stRightHoldLeftDown], trgLeftDown),
    Transition(&s[stRightHoldLeftDown], &s[stIdle], trgRightIdle), // Return to idle, when the right key is released
    Transition(&s[stRightHoldLeftHold], &s[stIdle], trgRightIdle),
    Transition(&s[stRightHoldLeftIdle], &s[stIdle], trgRightIdle),
    Transition(&s[stRightHoldLeftDownReleased], &s[stIdle], trgRightIdle),
    Transition(&s[stRightHoldLeftHoldReleased], &s[stIdle], trgRightIdle),
};

int num_transitions = sizeof(transitions) / sizeof(Transition);

void handleKeys()
{

  for (int i = 0; i < LIST_MAX; i++) // Scan the whole key list.
  {
    Key key = keypad.key[i];
    if (key.stateChanged) // Only find keys that have changed state.
    {
      switch (key.kstate)
      {
      // Trigger the FSM depending on the active key state : IDLE, PRESSED, HOLD, or RELEASED
      case PRESSED:
        fsm.trigger(key.kchar == RIGHT ? trgRightDown : trgLeftDown);
        break;

      case HOLD:
        fsm.trigger(key.kchar == RIGHT ? trgRightHold : trgLeftHold);
        break;

      case RELEASED:
        fsm.trigger(key.kchar == RIGHT ? trgRightReleased : trgLeftReleased);
        break;

      case IDLE:
        fsm.trigger(key.kchar == RIGHT ? trgRightIdle : trgLeftIdle);
      }
    }
  }
}

void setLedState(byte state)
{
  digitalWrite(LED_BUILTIN, state);
}

void setConnected(bool state)
{
  // Initialize with opposite state on startup
  static bool connected = true;

  if (connected != state)
  {
    connected = state;
    setLedState(connected ? LED_OFF : LED_ON);
    Serial.println(connected ? "connected" : "connecting");
  }
}

void setup()
{
  Serial.begin(SERIAL_SPEED);
  Serial.println();
  Serial.printf("%s %s", NAME, AUTHOR);
  Serial.println();

  pinMode(LED_BUILTIN, OUTPUT);
  setConnected(false);

  fsm.add(transitions, num_transitions);
  fsm.setInitialState(&s[stIdle]);

  bleKeyboard.begin();
  bleKeyboard.setDelay(BLE_DELAY);
}

void loop()
{
  fsm.run();

  if (bleKeyboard.isConnected())
  {
    setConnected(true);
    if (keypad.getKeys())
    {
      handleKeys();
    }
  }
  else
  {
    setConnected(false);
  }
}