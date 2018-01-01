//Sample using LiquidCrystal library
#include <LiquidCrystal.h>

// select the pins used on the LCD panel
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// define some values used by the panel and buttons
int lcd_key     = 0;
int prev_lcd_key = -1;
int prev_lcd_key_debug = -1;
int adc_key_in  = 0;

int globalCount = 0;
long diceOne;
long diceTwo;
int stats[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
float statsProcThreshold = 100/16;

#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

byte line_1[8] = { 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111 };
byte line_2[8] = { 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111 };
byte line_3[8] = { 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111, 0b11111 };
byte line_4[8] = { 0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111, 0b11111, 0b11111 };
byte line_5[8] = { 0b00000, 0b00000, 0b00000, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111 };
byte line_6[8] = { 0b00000, 0b00000, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111 };
byte line_7[8] = { 0b00000, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111 };
byte line_8[8] = { 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111 };

byte dice_1[8] = {
  0b00000,
  0b11111,
  0b11111,
  0b11011,
  0b11111,
  0b11111,
  0b00000,
  0b00000
};
byte dice_2[8] = {
  0b00000,
  0b11111,
  0b10111,
  0b11111,
  0b11101,
  0b11111,
  0b00000,
  0b00000
};
byte dice_3[8] = {
  0b00000,
  0b11111,
  0b10111,
  0b11011,
  0b11101,
  0b11111,
  0b00000,
  0b00000
};
byte dice_4[8] = {
  0b00000,
  0b11111,
  0b10101,
  0b11111,
  0b10101,
  0b11111,
  0b00000,
  0b00000
};
byte dice_5[8] = {
  0b00000,
  0b11111,
  0b10101,
  0b11011,
  0b10101,
  0b11111,
  0b00000,
  0b00000
};
byte dice_6[8] = {
  0b00000,
  0b11111,
  0b10001,
  0b11111,
  0b10001,
  0b11111,
  0b00000,
  0b00000
};

void setup()
{
  Serial.begin(38400);
  lcd.begin(16, 2);

  defineDiceChars();

  for (int i = 1; i <= 6; i++) {
    lcd.setCursor(i+4, 0);
    lcd.write(i);
  }

  lcd.setCursor(1, 1);
  lcd.print("Katana rullee!");
}

void loop()
{
  lcd_key = read_LCD_buttons();  // read the buttons

  if (lcd_key != btnNONE &&
      lcd_key != prev_lcd_key &&
      (lcd_key == btnUP || lcd_key == btnDOWN || lcd_key == btnLEFT)
  ) {
    // clear screen on new button press
    lcd.clear();
    Serial.print("\nclear: ");
    Serial.println(lcd_key);
  }

  switch (lcd_key)
  {
    case btnDOWN:
      {
        rollTwoDices();
        break;
      }
    case btnLEFT:
      {
        diceOne = 0;
        diceTwo = 0;
        printStatsDetailed();
        break;
      }
    case btnUP:
      {
        diceOne = 0;
        diceTwo = 0;
        printStats();
        break;
      }
    case btnRIGHT:
      {
        break;
      }
    case btnSELECT:
      {
        break;
      }
    case btnNONE:
      {
        if (prev_lcd_key != lcd_key &&
            diceOne > 0 &&
            diceTwo > 0 &&
            prev_lcd_key == btnDOWN
        ) {
          showDiceSum();
          addStats();
        }
        break;
      }
  }

  prev_lcd_key = lcd_key;
}


// -------------------------------------------------------------------------

void defineDiceChars() {
  lcd.createChar(1, dice_1);
  lcd.createChar(2, dice_2);
  lcd.createChar(3, dice_3);
  lcd.createChar(4, dice_4);
  lcd.createChar(5, dice_5);
  lcd.createChar(6, dice_6);
}

void defineLineChars() {
  lcd.createChar(1, line_1);
  lcd.createChar(2, line_2);
  lcd.createChar(3, line_3);
  lcd.createChar(4, line_4);
  lcd.createChar(5, line_5);
  lcd.createChar(6, line_6);
  lcd.createChar(7, line_7);
  lcd.createChar(8, line_8);
}

// read the buttons
int read_LCD_buttons()
{
  adc_key_in = analogRead(0);      // read the value from the sensor

  if (adc_key_in != prev_lcd_key_debug) {
    Serial.println(adc_key_in);
  }

  prev_lcd_key_debug = adc_key_in;

  // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
  // we add approx 50 to those values and check to see if we are close
  if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
  if (adc_key_in < 50)   return btnRIGHT;
  if (adc_key_in < 250)  return btnUP;
  if (adc_key_in < 500)  return btnDOWN;
  if (adc_key_in < 800)  return btnLEFT;

  return btnNONE;  // when all others fail, return this...
}

void rollTwoDices() {
  defineDiceChars();

  randomSeed(analogRead(2));

  diceOne = rollTheDice();
  diceTwo = rollTheDice();

  lcd.setCursor(7, 0);
  lcd.write(diceOne);
  lcd.setCursor(8, 0);
  lcd.write(diceTwo);
}

int rollTheDice() {
  int face = random(1, 100) % 3 + 1; // 1..3
  int direction = random(1, 100) % 2; // 0..1
  int dice = face + 3 * direction;

  if (dice < 1 ) {
    return 1;
  }

  if (dice > 6) {
    return 6;
  }

  return dice;
}

void showDiceSum() {
  int sum = diceOne + diceTwo;
  lcd.setCursor(7, 1);
  lcd.print(sum);
}

void addStats() {
  int idx = diceOne + diceTwo - 2; // 0 based array starting with 2

  stats[idx] += 1;
  globalCount += 1;
}

void printStatsDetailed() {
  String str = "";

  for (int i = 0; i <= 5; i++) {
    str = str + stats[i];
    if (i < 5) { str += "/"; }
  }
  lcd.setCursor(0, 0);
  lcd.print(str);

  str = "";
  for (int i = 6; i <= 10; i++) {
    str = str + stats[i];
    if (i < 10) { str += "/"; }
  }
  lcd.setCursor(0, 1);
  lcd.print(str);
}

void printStats() {
  int setColumn = 2;
  int num;
  int idx;
  int maxVal = getMaxValue();

  lcd.setCursor(0, 1);
  lcd.print("2");
  lcd.setCursor(14, 1);
  lcd.print("12");

  for (int i = 0; i <= 10; i++) {
    printStatsBar(setColumn, stats[i], maxVal);
    setColumn += 1;
  }
}

void printStatsBar(int col, int val, int maxVal) {
  defineLineChars();

  float proc;

  if (maxVal == 0) {
    proc = 0;
  } else {
    proc = val * 100 / maxVal;
  }

  int barLines = getBarLines(proc);

  Serial.print("col: ");
  Serial.print(col - 3);

  if (barLines > 8) {
    // make it 2 rows
    // top row
    int topLines = barLines - 8;
    lcd.setCursor(col, 0);
    lcd.write(topLines);

    Serial.print(" toplines: ");
    Serial.print(topLines);


    // bottom row full
    lcd.setCursor(col, 1);
    lcd.write(8);
  } else {
    // make it one (bottom) row

    Serial.print(" botlines: ");
    Serial.print(barLines);

    lcd.setCursor(col, 1);
    lcd.write(barLines);
  }
}

int getBarLines(float proc) {
  int lines = proc / statsProcThreshold;

  if (lines < 1)  { return 1; }
  if (lines > 16) { return 16; }
  return lines;
}

int getMaxValue() {
  int max = 0;

  for (int i = 0; i <= 10; i++){
    if (max < stats[i]) {
      max = stats[i];
    }
  }
  return max;
}
