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

// Key bindings for the ble keyboard
#define KBD_LEFT_PRESS KEY_UP_ARROW
#define KBD_LEFT_HOLD KEY_PAGE_UP
#define KBD_RIGHT_PRESS KEY_DOWN_ARROW
#define KBD_RIGHT_HOLD KEY_PAGE_DOWN
#define KBD_BOTH_PRESS KEY_LEFT_CTRL
#define KBD_BOTH_HOLD KEY_RIGHT_CTRL

void on_change()
{
  Serial.println(fsm.getState()->getName());
}

void on_idle()
{
  Serial.println("---");
}

void on_left()
{
  on_change();
  bleKeyboard.write(KBD_LEFT_PRESS);
}

void on_leftHold()
{
  on_change();
  bleKeyboard.write(KBD_RIGHT_PRESS);
}

void on_right()
{
  on_change();
  bleKeyboard.write(KBD_RIGHT_PRESS);
}

void on_rightHold()
{
  on_change();
  bleKeyboard.write(KBD_RIGHT_HOLD);
}

void on_both()
{
  on_change();
  bleKeyboard.write(KBD_BOTH_PRESS);
}

void on_bothHold()
{
  on_change();
  bleKeyboard.write(KBD_BOTH_HOLD);
}

void on_leftright()
{
  Serial.println(fsm.getState()->getName());
  // Serial.println("LEFTRIGHT");
}

void on_rightleft()
{
  Serial.println(fsm.getState()->getName());
  // Serial.println("RIGHTLEFT");
}

enum triggers
{
  left_down,
  left_hold,
  left_released,
  left_idle,
  right_down,
  right_hold,
  right_released,
  right_idle
};

State s[] = {
    State("Idle", on_idle), // 0

    // left without right
    State("LP", on_change),    // 1
    State("LH", on_change),    // 2
    State("LPR", on_left),     // 3
    State("LHR", on_leftHold), // 4

    // // right without left
    State("RP", on_change),     // 5
    State("RH", on_change),     // 6
    State("RPR", on_right),     // 7
    State("RHR", on_rightHold), // 8

    // left + right simultaneously
    State("DP", on_change),    // 9
    State("DH", on_change),    // 10
    State("DPR", on_both),     // 11
    State("DHR", on_bothHold), // 12

    // left hold + right
    State("LHRP", on_change), // 13
    State("LHRH", on_change), // 14
    State("LHRR", on_change), // 15
    State("LHRI", on_change), // 16

    // right hold + left
    State("RHLP", on_change), // 17
    State("RHLH", on_change), // 18
    State("RHRR", on_change), // 19
    State("RHLI", on_change), // 20
};

Transition transitions[] = {
    // left without right
    Transition(&s[0], &s[1], left_down),
    Transition(&s[1], &s[2], left_hold),
    Transition(&s[1], &s[3], left_released),
    Transition(&s[2], &s[4], left_released),
    Transition(&s[3], &s[0], left_idle), // Return to global idle, when single key
    Transition(&s[4], &s[0], left_idle),

    // right without left
    Transition(&s[0], &s[5], right_down),
    Transition(&s[5], &s[6], right_hold),
    Transition(&s[5], &s[7], right_released),
    Transition(&s[6], &s[8], right_released),
    Transition(&s[7], &s[0], right_idle), // Return to global idle, when single key
    Transition(&s[8], &s[0], right_idle),

    // left + right simultaneously
    Transition(&s[5], &s[9], left_down),
    Transition(&s[1], &s[9], right_down),
    Transition(&s[9], &s[10], left_hold),
    Transition(&s[9], &s[10], right_hold),
    Transition(&s[9], &s[11], left_released),
    Transition(&s[9], &s[11], right_released),
    Transition(&s[10], &s[12], left_released),
    Transition(&s[10], &s[12], right_released),
    Transition(&s[11], &s[0], left_idle), // Return to global idle, when any of the keys becomes idle
    Transition(&s[11], &s[0], right_idle),
    Transition(&s[12], &s[0], left_idle), 
    Transition(&s[12], &s[0], right_idle),

    // // left hold + right
    // Transition(&s[2], &s[13], right_down),
    // Transition(&s[13], &s[14], right_hold),
    // Transition(&s[14], &s[15], right_released),
    // Transition(&s[13], &s[15], right_released),
    // Transition(&s[15], &s[16], right_idle),
    // Transition(&s[16], &s[13], right_down),
    // Transition(&s[13], &s[0], left_idle), // Releasing the left key cancels everything
    // Transition(&s[14], &s[0], left_idle),
    // Transition(&s[15], &s[0], left_idle),
    // Transition(&s[16], &s[0], left_idle),

    // // right hold + left
    // Transition(&s[6], &s[17], left_down),
    // Transition(&s[17], &s[18], left_hold),
    // Transition(&s[18], &s[19], left_released),
    // Transition(&s[17], &s[19], left_released),
    // Transition(&s[19], &s[20], left_idle),
    // Transition(&s[20], &s[17], left_down),
    // Transition(&s[17], &s[0], right_idle), // Releasing the right key cancels everything
    // Transition(&s[18], &s[0], right_idle),
    // Transition(&s[19], &s[0], right_idle),
    // Transition(&s[20], &s[0], right_idle),
};

int num_transitions = sizeof(transitions) / sizeof(Transition);

// bool isBatteryLow()
// {
//   const byte SAMPLE_COUNT = 16;
//   const int HI_VOLTAGE = 2200;  // About 3.7V
//   const int LOW_VOLTAGE = 1750; // About 3.0V

//   static int samples[SAMPLE_COUNT];
//   static byte ptr = 0;

//   int batteryAvg = 0;

//   // Get a reading from the ADC
//   samples[ptr++] = analogRead(battPin);
//   ptr &= 0x0F;

//   // Average the collected samples
//   for (byte i = 0; i < SAMPLE_COUNT; i++)
//   {
//     batteryAvg += samples[i];
//   }
//   batteryAvg += SAMPLE_COUNT / 2; // Make sure we round correctly
//   batteryAvg /= SAMPLE_COUNT;
//   Serial.printf("Battery: %d\n", batteryAvg);

//   bleKeyboard.setBatteryLevel(batteryAvg >= HI_VOLTAGE ? 100 : 10 + 90 * (batteryAvg - LOW_VOLTAGE) / (HI_VOLTAGE - LOW_VOLTAGE));
//   delay(BLE_DELAY);
//   Serial.printf("Battery: %d%%\n", 10 + 90 * (batteryAvg - LOW_VOLTAGE) / (HI_VOLTAGE - LOW_VOLTAGE));

//   return batteryAvg <= LOW_VOLTAGE ? true : false;
// }

// Handle short and long presses of the two buttons
// void keypadEvent(KeypadEvent key)
// {
//   static bool shortPress;

//   Serial.print(key);
//   switch (keypad.getState())
//   {
//   case PRESSED:
//     Serial.println('P');
//     shortPress = true;
//     break;

//   case RELEASED:
//     Serial.println('R');
//     if (shortPress)
//       bleKeyboard.press(key == RIGHT ? KEY_DOWN_ARROW : KEY_UP_ARROW);
//     break;

//   case HOLD:
//     Serial.println('H');
//     shortPress = false;
//     bleKeyboard.press(key == RIGHT ? KEY_PAGE_DOWN : KEY_PAGE_UP);
//     break;

//   case IDLE:
//     Serial.println('I');
//     break;
//   }
//   bleKeyboard.releaseAll(); // Release the keys
// }

void handleKeys()
{

  for (int i = 0; i < LIST_MAX; i++) // Scan the whole key list.
  {
    Key key = keypad.key[i];
    if (key.stateChanged) // Only find keys that have changed state.
    {
      switch (keypad.key[i].kstate)
      { // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
      case PRESSED:
        fsm.trigger(key.kchar == RIGHT ? right_down : left_down);
        break;

      case HOLD:
        fsm.trigger(key.kchar == RIGHT ? right_hold : left_hold);
        break;

      case RELEASED:
        fsm.trigger(key.kchar == RIGHT ? right_released : left_released);
        break;

      case IDLE:
        fsm.trigger(key.kchar == RIGHT ? right_idle : left_idle);
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
  fsm.setInitialState(&s[0]);

  bleKeyboard.begin();
  bleKeyboard.setDelay(BLE_DELAY);
}

void loop()
{
  // static unsigned long updateTimer = 0;

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

  // if (millis() - updateTimer > 1000)
  // {
  //   isBatteryLow();
  //   updateTimer = millis();
  // }
}