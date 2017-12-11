

/*********************

  Example code for the Adafruit RGB Character LCD Shield and Library

  This code displays text on the shield, and also reads the buttons on the keypad.
  When a button is pressed, the backlight changes color.

**********************/

// include the library code:
#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
#include <Sodaq_AT_Device.h>
#include <Sodaq_nbIOT.h>
#include <Sodaq_OnOffBee.h>

#include "screen.h"

typedef enum {
  sUSB = 0,
  sUSB_SPEED,
  sUSB_SPEED_115200,
  sUSB_SPEED_57600,
  sUSB_SPEED_9600,
  sUSB_SPEED_2400,
  sUSB_TEST,
  sUSB_TEST_RECV,
  sUSB_TEST_SEND,
  sNBIOT,
  sNBIOT_RADIO_TOGGLE,
  sNBIOT_RECV,
  sNBIOT_REBOOT,
  sNBIOT_NCONFIG,
  sNBIOT_NEUSTATS,
  sNBIOT_TEST,
  sNBIOT_TEST_SEND,
  sNBIOT_SNR
} MenuState_t;



char *MenuText[] = {
  "USB " CH_D CH_R,
  "SPEED " CH_L CH_D CH_R,
  "115200" CH_X CH_L CH_D,
  "57600" CH_X CH_L CH_U CH_D,
  "9600" CH_X CH_L CH_U CH_D,
  "2400"  CH_X CH_L CH_U,
  "TEST " CH_L CH_U CH_R,
  "RECV" CH_X CH_L CH_D,
  "SEND" CH_X CH_L CH_U,
  "NBIOT " CH_U CH_R,
  "RADIO" CH_X CH_L CH_D,
  "RECV" CH_X CH_L CH_U CH_D,
  "REBOOT" CH_X CH_L CH_U CH_D,
  "NCONFIG?" CH_X CH_L CH_U CH_D,
  "NEUSTATS" CH_X CH_L CH_U CH_D,
  "TEST " CH_L CH_U CH_D CH_R,
  "SEND" CH_X CH_L,
  "SNR" CH_X CH_L CH_U
};

typedef enum {
  NBIOT_BUSY = 0,
  NBIOT_UNKNOWN,
  NBIOT_OK,
  NBIOT_ERROR
} NBIOTStatus_t;

uint8_t i = 0;
volatile MenuState_t menustate = sUSB;
bool menu_speed_setting = false;
bool wait = false;
#define nbiot Serial1
#define NBIOT_PWR 7
unsigned long current_usb_speed = 115200UL;
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
Screen screen;


void setup() {
 // Debugging output
  Serial.begin(current_usb_speed);

  pinMode(NBIOT_PWR, OUTPUT);
  digitalWrite(NBIOT_PWR, HIGH);
  nbiot.begin(9600);

  screen = Screen(&lcd);

  lcd.print("NB IoT Menu " CH_X CH_OTR CH_CTR);
  for (uint8_t color = 0; color < 8; color++) {
    lcd.setBacklight(color);
    delay(200);
  }
  lcd.setBacklight(WHITE);
  delay(1000);

  lcd.home();
  lcd.clear();
  menustate = sUSB;
  lcd.print(MenuText[menustate]);

  delay(2000);
}

// flash_error(): flash the screen to inform the user of an error:
void flash_error() {
  for (int f = 0; f < 4; f++) {
    lcd.setBacklight((f & 0x01) ? WHITE : RED);
    Serial.println((f & 0x01) ? "WHITE" : "RED");
    delay(150);
  }
}

// test_sub_speed() check the currently selected USB speed against
// the current menu state.
void usb_speed_test(MenuState_t ms) {
  bool test = false;
  switch (ms) {
    case sUSB_SPEED_115200:
      test = (current_usb_speed == 115200);
      break;
    case sUSB_SPEED_57600:
      test = (current_usb_speed == 57600);
      break;
    case sUSB_SPEED_9600:
      test = (current_usb_speed == 9600);
      break;
    case sUSB_SPEED_2400:
      test = (current_usb_speed == 2400);
      break;
  }
  lcd.setBacklight(test ? GREEN : WHITE);

}

void set_usb_speed(unsigned long speed) {
  current_usb_speed = speed;
  Serial.begin(speed);
  lcd.setCursor(0, 1);
  lcd.print("baud : ");
  lcd.print(speed);
  lcd.setCursor(0, 0);
}

void usb_test_receiving() {
  unsigned long ccnt = 0;
  uint8_t buttons = 0;
  lcd.setBacklight(BLUE);
  lcd.clear();
  lcd.home();
  delay(200);
  while (!(buttons & BUTTON_SELECT)) {
    if (Serial.available()) {
      char c = Serial.read();
      lcd.print(c);
      if (ccnt > 16) {
        lcd.scrollDisplayLeft();
      } else ccnt++;
    }
    buttons = screen.get_button();
  }
  lcd.clear();
  lcd.setBacklight(WHITE);
}

void usb_test_sending() {
  uint8_t buttons = 0;
  char c = ' ';
  int16_t ccnt = 0;

  lcd.setBacklight(YELLOW);
  lcd.clear();
  lcd.home();
  delay(200);

  while (!(buttons & BUTTON_SELECT)) {
    if (c <= '~') {
      c += 1;
    } else {
      c = ' ';
    }
    Serial.print(c);
    lcd.print(c);
    if (ccnt > 16) {
      lcd.scrollDisplayLeft();
    } else ccnt++;
    buttons = lcd.readButtons();
  }
  lcd.clear();
  lcd.setBacklight(WHITE);

}

void nbiot_atcmd(const char *cmd) {
  nbiot.print("AT+");
  nbiot.print(cmd);
  nbiot.print("\r\n");
}

NBIOTStatus_t nbiot_response() {
  static char linebuf[LINE_WIDTH + 1];
  uint8_t line_cnt = 0;
  char c;
  uint8_t i = 0;
  bool ok_seen, error_seen, done;
  memset(linebuf, 0, 16 + 1);
  screen.reset();
  NBIOTStatus_t status = NBIOT_UNKNOWN;

  done = false; ok_seen = false; error_seen = false;
  while (!done) {
    if (nbiot.available()) {
      c = nbiot.read();
      Serial.print(c);
      if (c == '\r') {
        // absorb the CR.
      } else if (c == '\n') { // use the LF to signal the end of a line:
        ok_seen = (strncmp(linebuf, "OK", 2) == 0);
        error_seen = (strncmp(linebuf, "ERROR", 5) == 0);
        screen.add_line(linebuf, i);
        linebuf[i] = '\0';
        i = 0;
      } else if (i < LINE_WIDTH + 1) {
        linebuf[i++] = c;
      };
    } else {
      done =  ok_seen || error_seen; //screen_escape();
    }
  }

  if (ok_seen) {
    lcd.setBacklight(YELLOW);
    status = NBIOT_OK;
  }
  if (error_seen) {
    lcd.setBacklight(RED);
    status = NBIOT_ERROR;
  }
  screen.refresh();
  return status;
}

NBIOTStatus_t nbiot_reboot() {

  lcd.setBacklight(GREEN);
  char*msg = "Rebooting...";
  screen.add_line(msg, strlen(msg), true);
  delay(150);

  nbiot_atcmd("NRB");


  NBIOTStatus_t status = nbiot_response();
  delay(2000);

  while (screen.navigate());

  lcd.setBacklight(WHITE);
  lcd.clear();
  return status;
}

NBIOTStatus_t nbiot_get_config() {
  lcd.setBacklight(GREEN);
  screen.clearDisplay();
  char *msg = "Get NConfig...";
  screen.add_line(msg, strlen(msg), true);
  delay(150);

  nbiot_atcmd("NCONFIG?");

  NBIOTStatus_t status = nbiot_response();
  delay(2000);

  while (screen.navigate());

  lcd.setBacklight(WHITE);
  lcd.clear();
  return status;
}

NBIOTStatus_t nbiot_get_status() {
  lcd.setBacklight(GREEN);
  screen.clearDisplay();
  char *msg = "Get NUE Status...";
  screen.add_line(msg, strlen(msg), true);
  delay(150);

  nbiot_atcmd("NUESTATS");

  NBIOTStatus_t status = nbiot_response();
  delay(2000);

  while (screen.navigate());

  lcd.setBacklight(WHITE);
  lcd.clear();
  return status;
}

NBIOTStatus_t nbiot_radio_toggle() {
  static bool radio_on = false;
  NBIOTStatus_t status;

  lcd.setBacklight(GREEN);
  screen.clearDisplay();
  char *msg = "NB-IoT Radio ...";
  screen.add_line(msg, strlen(msg), true);
  delay(150);
  radio_on = !radio_on;
  if (radio_on) {
    nbiot_atcmd("CFUN=0");
    status = nbiot_response();
    nbiot_atcmd("NCDP=172.16.15.22");
    status = nbiot_response();
    screen.refresh();
    nbiot_atcmd("CGDCONT=1,\"IP\",\"oceanconnect.t-mobile.nl\"");
    status = nbiot_response();
    screen.refresh();
    nbiot_atcmd("CFUN=1");
    status = nbiot_response();
    screen.refresh();
    nbiot_atcmd("COPS=1,2,\"20416\"");
    status = nbiot_response();
    screen.add_line("AAN", 3);
    screen.refresh();
  } else {
    nbiot_atcmd("CFUN=0");
    status = nbiot_response();
    screen.add_line("UIT", 3);
    screen.refresh();
  }

  lcd.setBacklight(WHITE);
  lcd.clear();
  return status;
}

void loop() {
  uint8_t button = screen.get_button();
 
  if (button && !wait) {
    wait = true;
    lcd.clear();
    lcd.setCursor(0, 0);

    switch (menustate) {
      case sUSB:
        switch (button) {
          case BUTTON_RIGHT:
            menustate = sUSB_SPEED;
            menu_speed_setting = true;
            break;
          case BUTTON_DOWN:
            menustate = sNBIOT;
            break;
          default:
            flash_error();
        }
        break;
      case sUSB_SPEED:
        switch (button) {
          case BUTTON_DOWN:
            menustate = sUSB_TEST;
            menu_speed_setting = false;
            break;
          case BUTTON_RIGHT:
            menustate = sUSB_SPEED_115200;
            break;
          case BUTTON_LEFT:
            menustate = sUSB;
            menu_speed_setting = false;
            break;
          default: flash_error();
        }
        break;
      case sUSB_SPEED_115200:
        switch (button) {
          case BUTTON_DOWN:
            menustate = sUSB_SPEED_57600;
            break;
          case BUTTON_LEFT:
            menustate = sUSB_SPEED;
            break;
          case BUTTON_SELECT:
            set_usb_speed(115200UL);
            break;
          default: flash_error();
        }
        break;
      case sUSB_SPEED_57600:
        switch (button) {
          case BUTTON_UP:
            menustate = sUSB_SPEED_115200;
            break;
          case BUTTON_DOWN:
            menustate = sUSB_SPEED_9600;
            break;
          case BUTTON_LEFT:
            menustate = sUSB_SPEED;
            break;
          case BUTTON_SELECT:
            set_usb_speed(57600UL);
            break;
          default: flash_error();
        }
        break;
      case sUSB_SPEED_9600:
        switch (button) {
          case BUTTON_UP:
            menustate = sUSB_SPEED_57600;
            break;
          case BUTTON_DOWN:
            menustate = sUSB_SPEED_2400;
            break;
          case BUTTON_LEFT:
            menustate = sUSB_SPEED;
            break;
          case BUTTON_SELECT:
            set_usb_speed(9600UL);
            break;
          default: flash_error();
        }
        break;
      case sUSB_SPEED_2400:
        switch (button) {
          case BUTTON_UP:
            menustate = sUSB_SPEED_9600;
            break;
          case BUTTON_LEFT:
            menustate = sUSB_SPEED;
            break;
          case BUTTON_SELECT:
            set_usb_speed(2400UL);
            break;
          default: flash_error();
        }
        break;
      case sUSB_TEST:
        switch (button) {
          case BUTTON_RIGHT:
            menustate = sUSB_TEST_RECV;
            break;
          case BUTTON_UP:
            menustate = sUSB_SPEED;
            menu_speed_setting = true;
            break;
          case BUTTON_LEFT:
            menustate = sUSB;
            break;
          default: flash_error();
        }
        break;
      case sUSB_TEST_RECV:
        switch (button) {
          case  BUTTON_DOWN:
            menustate = sUSB_TEST_SEND;
            break;
          case BUTTON_LEFT:
            menustate = sUSB_TEST;
            break;
          case BUTTON_SELECT:
            usb_test_receiving();
            break;
          default: flash_error();
        }
        break;
      case sUSB_TEST_SEND:
        switch (button) {
          case  BUTTON_UP:
            menustate = sUSB_TEST_RECV;
            break;
          case BUTTON_LEFT:
            menustate = sUSB_TEST;
            break;
          case BUTTON_SELECT:
            usb_test_sending();
            break;
          default: flash_error();
        }
        break;
      // NBIOT:
      case sNBIOT:
        switch (button) {
          case  BUTTON_UP:
            menustate = sUSB;
            break;
          case BUTTON_RIGHT:
            menustate = sNBIOT_RADIO_TOGGLE;
            break;
          default: flash_error();
        }
        break;
      case sNBIOT_RADIO_TOGGLE:
        switch (button) {
          case  BUTTON_DOWN:
            menustate = sNBIOT_RECV;
            break;
          case BUTTON_LEFT:
            menustate = sNBIOT;
            break;
          case BUTTON_SELECT:
            nbiot_radio_toggle();
            break;
          default: flash_error();
        }
        break;
      case sNBIOT_RECV:
        switch (button) {
          case  BUTTON_UP:
            menustate = sNBIOT_RADIO_TOGGLE;
            break;
          case BUTTON_DOWN:
            menustate = sNBIOT_REBOOT;
            break;
          case BUTTON_LEFT:
            menustate = sNBIOT;
            break;
          default: flash_error();
        }
        break;
      case sNBIOT_REBOOT:
        switch (button) {
          case  BUTTON_UP:
            menustate = sNBIOT_RECV;
            break;
          case BUTTON_DOWN:
            menustate = sNBIOT_NCONFIG;
            break;
          case BUTTON_LEFT:
            menustate = sNBIOT;
            break;
          case BUTTON_SELECT:
            nbiot_reboot();
            break;
          default: flash_error();
        }
        break;
      case sNBIOT_NCONFIG:
        switch (button) {
          case  BUTTON_UP:
            menustate = sNBIOT_REBOOT;
            break;
          case BUTTON_DOWN:
            menustate = sNBIOT_NEUSTATS;
            break;
          case BUTTON_LEFT:
            menustate = sNBIOT;
            break;
          case BUTTON_SELECT:
            nbiot_get_config();
            break;
          default: flash_error();
        }
        break;
      case sNBIOT_NEUSTATS:
        switch (button) {
          case  BUTTON_UP:
            menustate = sNBIOT_NCONFIG;
            break;
          case BUTTON_DOWN:
            menustate = sNBIOT_TEST;
            break;
          case BUTTON_LEFT:
            menustate = sNBIOT;
            break;
          case BUTTON_SELECT:
            nbiot_get_status();
            break;
          default: flash_error();
        }
        break;
      case sNBIOT_TEST:
        switch (button) {
          case  BUTTON_UP:
            menustate = sNBIOT_REBOOT;
            break;
          case BUTTON_DOWN:
            menustate = sNBIOT_SNR;
            break;
          case BUTTON_RIGHT:
            menustate = sNBIOT_TEST_SEND;
            break;
          case BUTTON_LEFT:
            menustate = sNBIOT;
            break;
          default: flash_error();
        }
        break;
      case sNBIOT_TEST_SEND:
        switch (button) {
          case  BUTTON_LEFT:
            menustate = sNBIOT_TEST;
            break;
          default: flash_error();
        }
        break;
      case sNBIOT_SNR:
        switch (button) {
          case  BUTTON_UP:
            menustate = sNBIOT_TEST;
            break;
          case BUTTON_LEFT:
            menustate = sNBIOT;
            break;
          default: flash_error();
        }
        break;
      default: flash_error(); // Illegal menu state !!!
    }
    if (menu_speed_setting) {
      lcd.setCursor(0, 1);
      lcd.print("Speed: ");
      lcd.print(current_usb_speed);
    }

    usb_speed_test(menustate);
    lcd.setCursor(0, 0);
    lcd.print(MenuText[menustate]);
    Serial.println(MenuText[menustate]);
    wait = false;


  }
}

