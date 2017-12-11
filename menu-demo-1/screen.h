
#ifndef _SCREEN_H_
#define  _SCREEN_H_

#include <Arduino.h>
#include <Adafruit_RGBLCDShield.h>


// These #defines make it easy to set the backlight color
#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7


#define LINES 15
#define LINE_WIDTH 40
#define DISPLAY_HEIGHT 2
#define DISPLAY_WIDTH 16


#define CH_U "\x01"
#define CH_D "\002"
#define CH_L "\x7F"
#define CH_R "\x7E"
#define CH_X "\x03"
#define CH_OTR "\x04"
#define CH_CTR "\x05"

class Screen {
  private:
    uint8_t end_of_buffer; // The first position in the buffer after the last added line.
    uint8_t current_col;  // Current  "cursor" column in the screen.
    uint8_t top_of_display;         // Index in the buffer to the first line displayed in the screen.
    char buffer[LINES][LINE_WIDTH+1];
    Adafruit_RGBLCDShield *lcd;

    void dump(void);
    void wipeLine(uint8_t line);
  public:
    Screen();
    Screen(Adafruit_RGBLCDShield *the_lcd);
    
    void clearDisplay(void);
    void refreshFrom(uint8_t aline);
    void refresh(void);
    uint8_t get_button(void);
    void add_line(char line[], uint8_t linelen, bool refresh_it = false);
    bool navigate(void);
    bool escape(void);
    void reset(void);

    void setupCustomCharacters(void);
    
    void test(void) {
      lcd->clear();
      lcd->home();
      lcd->print("Hello!");
    };

};
#endif // _SCREEN_H_

// eof(screen.h).

