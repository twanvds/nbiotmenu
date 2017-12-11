
#include "screen.h"
#include "util.h"


static uint8_t UP[8] = {

  0b00000000,
  0b00000100,
  0b00001110,
  0b00010101,
  0b00000100,
  0b00000100,
  0b00000100,
  0b00000000
};

static uint8_t DN[8] = {

  0b00000100,
  0b00000100,
  0b00000100,
  0b00000100,
  0b00010101,
  0b00001110,
  0b00000100,
  0b00000000
};

static uint8_t EXEC[8] = {
  0b00000000,
  0b00000100,
  0b00001110,
  0b00011111,
  0b00001110,
  0b00000100,
  0b00000000,
  0b00000000
};


static uint8_t OTR[8] = {
  0b00001000,
  0b00000100,
  0b00000010,
  0b00000001,
  0b00000010,
  0b00000100,
  0b00001000,
  0b00000000
};


static uint8_t CTR[8] = {
  0b00001000,
  0b00001100,
  0b00001110,
  0b00001111,
  0b00001110,
  0b00001100,
  0b00001000,
  0b00000000
};


Screen::Screen() {}

Screen::Screen(Adafruit_RGBLCDShield *the_lcd):
  lcd(the_lcd), end_of_buffer(0), current_col(0), top_of_display(0) {
  lcd->begin(DISPLAY_WIDTH, DISPLAY_HEIGHT);
  setupCustomCharacters();
  clearDisplay();
}

void Screen::reset(void) {
  end_of_buffer = 0;
  current_col = 0;
  top_of_display = 0;
}

void Screen::wipeLine(uint8_t line) {
  memset(buffer[line], ' ', LINE_WIDTH);
  buffer[line][LINE_WIDTH] = '\0';
}

void Screen::clearDisplay(void) {
  for (int line = 0; line < LINES; line++) {
    wipeLine(line);
  }
  reset();
  refresh();
}


void Screen::add_line(char line[], uint8_t line_length, bool refresh_it) {

  if ((line_length == 0) || (line[0] == '\r') || (line[0] == '\n')) return; // skip empty lines.

  wipeLine(end_of_buffer);

  uint8_t copy_length = min(line_length, LINE_WIDTH);
  // Copy the content of line into the end_of_buffer line:
  strncpy(buffer[end_of_buffer], line, copy_length);
  buffer[end_of_buffer][copy_length] = '\0';
  // Go the the new end of the buffer:
  top_of_display = moddec(end_of_buffer, LINES);
  end_of_buffer = modinc(end_of_buffer, LINES);

  if (refresh_it) {

    refresh();
  }
}


void Screen::refreshFrom(uint8_t new_tos) {
  // Very slow, not suitable to refresh between Serial receives !!!!
  lcd->clear();
  lcd->setCursor(0, 0);
  lcd->print(buffer[top_of_display]);
  lcd->setCursor(0, 1);
  lcd->print(buffer[modinc(top_of_display, LINES)]);

  dump();
}


void Screen::refresh(void) {
  refreshFrom(top_of_display);
}



uint8_t Screen::get_button(void) {
  static uint8_t prev_b = 0;
  uint8_t b = lcd->readButtons();
  if (prev_b == b) return 0;
  if (b & BUTTON_SELECT) {
    b = BUTTON_SELECT;
  } else if (b & BUTTON_LEFT) {
    b = BUTTON_LEFT;
  } else if (b & BUTTON_DOWN) {
    b = BUTTON_DOWN;
  } else if (b & BUTTON_UP) {
    b = BUTTON_UP;
  } else if (b & BUTTON_RIGHT) {
    b = BUTTON_RIGHT;
  } else b = 0;
  prev_b = b;
  return b;
}


bool Screen::navigate(void) {
  uint8_t button = get_button();
  delay(200);
  switch (button) {
    case BUTTON_UP:
      current_col  = 0;
      top_of_display = moddec(top_of_display, LINES);
      refresh();
      break;
    case BUTTON_DOWN:
      current_col = 0;
      top_of_display = modinc(top_of_display, LINES);
      refresh();
      break;
    case BUTTON_RIGHT:
      if (current_col < (LINE_WIDTH - DISPLAY_WIDTH)) {
        current_col++;
        lcd->scrollDisplayLeft();
      }
      break;
    case BUTTON_LEFT:
      if (current_col > 0) {
        current_col--;
        lcd->scrollDisplayRight();
      }
      break;
    case BUTTON_SELECT:
      Serial.println("done");
      return false;
    default:
      ; // skip.
  }
  return true;
}


bool Screen::escape(void) {
  uint8_t buttons = lcd->readButtons();
  if (buttons) {
    if (buttons & BUTTON_SELECT) {
      Serial.println("done");
      return true;
    };
  }
  return false;
}

void Screen::flash_error(void) {
  // flash_error(): flash the screen to inform the user of an error:
  for (int f = 0; f < 4; f++) {
    lcd->setBacklight((f & 0x01) ? WHITE : RED);
    delay(150);
  }
}


void Screen::setupCustomCharacters(void) {
  // Set up characters not available in the LCD driver characterset:
  lcd->createChar(1, UP); // Up.
  lcd->createChar(2, DN); // Down.
  lcd->createChar(3, EXEC); // Diamond.
  lcd->createChar(4, OTR); // Open Triangle Right.
  lcd->createChar(5, CTR); // Closed Triangle Right.
}

void Screen::dump(void) {
  Serial.println("----[Current Screen]----");
  Serial.print("eob: "); Serial.println(end_of_buffer);
  Serial.print("tod: "); Serial.println(top_of_display);
  Serial.println("----");
  for (int i = 0; i < LINES; i++) {
    Serial.print(i); Serial.print((i == end_of_buffer) ? ">" : ":"); Serial.println(buffer[i]);
  }
}


// eof(screen.cpp).

