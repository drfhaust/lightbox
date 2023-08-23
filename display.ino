#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup()
{
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }
  display.setTextColor(SSD1306_WHITE);
  display.display();
  // delay(2000);
}

void loop()
{
  refreshDisplay();
}

void refreshDisplay()
{
  display.clearDisplay();

  display.setCursor(10, 0);
  display.setTextSize(1);
  display.println("MAIN");

  display.setCursor(10, 8);
  display.print("200");
  display.print("v");

  //////////
  display.setCursor(57, 0);
  display.setTextSize(1);
  display.println("INV");

  display.setCursor(55, 8);
  display.print("220");
  display.print("v");

  ////////////

  display.setCursor(97, 0);
  display.setTextSize(1);
  display.println("GEN");

  display.setCursor(95, 8);
  display.print("210");
  display.print("v");

  ////////////////////////////////////////////////////////////////////////////////

  display.setCursor(5, 25);
  display.setTextSize(2);
  display.print("OUT");
  display.print(":");

  display.setCursor(50, 25);
  display.setTextSize(2);
  display.print("10000");
  display.print("W   ");

  ////////////////////////////////////////////////////////////////////////////////

  display.setTextSize(1);
  display.setCursor(5, 45);
  display.println("MODE");

  display.setCursor(55, 45);
  display.println("BATV");

  display.setCursor(95, 45);
  display.println("GENV");

  ////////////////////////////////////////////////////////////////////////
  display.setTextColor(BLACK, WHITE); // 'inverted' text

  display.setTextSize(1);
  display.setCursor(0, 55);
  display.println("MANUAL");

  display.setTextSize(1);
  display.setCursor(55, 55);
  display.print("100");
  display.print("%");

  display.setCursor(95, 55);
  display.print("100");
  display.print("%");
  display.setTextColor(WHITE, BLACK); // 'inverted' text
  display.display();

  delay(2000);
}
/////