/* codigo para pantalla touch ILI9341 2,4" 320x240px (arduino shield)
 * por Gonzalo Olave y Joakin Ugalde para un proyecto de base rotatoria
 * 
 * (C) FabLab U. de Chile
 */

// These are the pins for the shield!
#define YP A2  // must be an analog pin, use "An" notation!
#define XM A3  // must be an analog pin, use "An" notation!
#define YM 8   // can be a digital pin
#define XP 9   // can be a digital pin

#define MINPRESSURE 100
#define MAXPRESSURE 300

#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_RS A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

#include <Adafruit_GFX.h>// Hardware-specific library
#include <UTFTGLUE.h>              // class methods are in here
#include <TouchScreen.h>

UTFTGLUE myGLCD(0x9341, LCD_RS, LCD_WR, LCD_CS, LCD_RESET, LCD_RD);

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

// Assign human-readable names to some common 16-bit color values:
#define BLACK       0x0000
#define BLUE        0x001F
#define RED         0xF800
#define GREEN       0x07E0
#define CYAN        0x07FF
#define MAGENTA     0xF81F
#define YELLOW      0xFFE0
#define WHITE       0xFFFF
#define ORANGE      0xFD20
#define GREENYELLOW 0xAFE5
#define NAVY        0x000F
#define DARKGREEN   0x03E0
#define DARKCYAN    0x03EF
#define MAROON      0x7800
#define PURPLE      0x780F
#define OLIVE       0x7BE0
#define LIGHTGREY   0xC618
#define DARKGREY    0x7BEF

#define GRAY_1 60

#define W 320
#define H 240
#define MARGIN 10 // px
#define SEP 5
#define B_H 70
#define B_W 90
#define BB_H 105
#define BB_W 90
#define TZ_W W/3
#define TZ_H H

// boton 1
#define B1_X MARGIN
#define B1_Y MARGIN
#define B1_W MARGIN+B_W
#define B1_H MARGIN+B_H
// boton 2
#define B2_X MARGIN
#define B2_Y MARGIN+B_H+SEP
#define B2_W MARGIN+B_W
#define B2_H MARGIN+B_H+SEP+B_H
// boton 3
#define B3_X MARGIN
#define B3_Y MARGIN+B_H+SEP+B_H+SEP
#define B3_W MARGIN+B_W
#define B3_H H-MARGIN
// boton 4
#define B4_X W-MARGIN-BB_W
#define B4_Y MARGIN
#define B4_W W-MARGIN
#define B4_H MARGIN+BB_H
// boton 5
#define B5_X W-MARGIN-BB_W
#define B5_Y H-MARGIN-BB_H
#define B5_W W-MARGIN
#define B5_H H-MARGIN

// zona de texto
#define TZ_X MARGIN+B_W+SEP
#define TZ_Y MARGIN
#define TZ_WP W-MARGIN-B_W-SEP
#define TZ_HP TZ_H-MARGIN

#define TIME_X  TZ_X+MARGIN
#define TIME_Y  TZ_Y+MARGIN*7
#define ANGLE_X TZ_X+MARGIN
#define ANGLE_Y TZ_Y+MARGIN*12
#define DELTA_X 0//5*MARGIN
#define DELTA_Y 2*MARGIN//0

// pantalla ILI9341 2,4" 320x240 px

#define TS_MINX 130
#define TS_MINY 120
#define TS_MAXX 950
#define TS_MAXY 920

#define PRESSED 1
#define NOT_PRESSED 0

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
TSPoint tp;

extern uint8_t SmallFont[];

int X, Y, Z;

bool _press = false;
bool b1_flag, b2_flag, b3_flag, b4_flag, b5_flag;

#define TIEMPO    1
#define ANGULO    2
#define VELOCIDAD 3

int staytime = 1;
int angle = 10;
int mode = 1;
unsigned long stepclock = 0;

#define M_EN A4
#define M_STEP A5

int en_st = 0;
int step_st = 0;

#define velocidad 1000
#define MIN_TIME 1
#define MAX_TIME 30
#define MIN_DEG 1
#define MAX_DEG 30

#define DELTA_ANGLE 1
#define DELTA_TIME 1

void readResistiveTouch(void)
{
  digitalWrite(13, HIGH);
  tp = ts.getPoint();
  digitalWrite(13, LOW);

  pinMode(YP, OUTPUT);      //restore shared pins
  pinMode(XM, OUTPUT);

  X = map(tp.x, TS_MAXX, TS_MINX, 0, W); // valores invertidos
  Y = map(tp.y, TS_MINY, TS_MAXY, 0, H);
  Z = tp.z;
}

void setup() {

  Serial.begin(9600);
  pinMode(M_EN, OUTPUT);
  pinMode(M_STEP, OUTPUT);
  myGLCD.InitLCD();
  myGLCD.setFont(SmallFont);
  myGLCD.setRotation(1);
  myGLCD.fillScreen(BLACK);
  dibujarMenu();
  myGLCD.setColor(BLACK, BLACK, BLACK);
  myGLCD.drawRect(TZ_X , TZ_Y, TZ_WP , TZ_HP);

}

void loop() {

  int sel;
  readResistiveTouch();
  sel = getSelection();
  if (sel == 1 && b1_flag == HIGH) {
    touchB1();
    b1_flag = LOW;
    updateData();
  }
  if (sel == 2 && b2_flag == HIGH) {
    touchB2();
    b2_flag = LOW;
    updateData();
  }
  if (sel == 3 && b3_flag == HIGH) {
    touchB3();
    b3_flag = LOW;
    updateData();
  }
  if (sel == 4 && b4_flag == HIGH) {
    touchB4();
    b4_flag = LOW;
    updateData();
    stepclock = millis();
  }
  if (sel == 5 && b5_flag == HIGH) {
    touchB5();
    b5_flag = LOW;
    updateData();
  }

  if (en_st == 1) {

    if (millis() - stepclock >= staytime * 1000) {

      int nsteps = angle*3;
      digitalWrite(M_EN, LOW);
      for (int i = 0; i < nsteps; i++)
      {
        digitalWrite(M_STEP, HIGH);
        delayMicroseconds(velocidad);
        digitalWrite(M_STEP, LOW);
        delayMicroseconds(velocidad);
      }

      stepclock = millis();
    }

  }

}

void dibujarMenu() {

  // boton 1
  myGLCD.setColor(255, 0, 85); //magenta
  myGLCD.fillRoundRect(B1_X, B1_Y, B1_W, B1_H);
  myGLCD.setTextSize(3);
  myGLCD.setTextColor(WHITE);
  myGLCD.print("+", (B1_W - B1_X) / 2, (B1_H - B1_Y) / 2, 1);

  // boton 2
  myGLCD.setColor(36, 182, 255); // calipso
  myGLCD.fillRoundRect(B2_X, B2_Y, B2_W, B2_H);
  myGLCD.setTextSize(3);
  myGLCD.setTextColor(WHITE);
  myGLCD.print("MODE", (B2_W - B2_X) / 4, (B2_H - B2_Y) / 2);

  // boton 3
  myGLCD.setColor(255, 0, 85);
  myGLCD.fillRoundRect(B3_X, B3_Y, B3_W, B3_H);
  myGLCD.setTextSize(3);
  myGLCD.setTextColor(WHITE);
  myGLCD.print("-", (B3_W - B3_X) / 2, (B3_H - B3_Y) / 2);

  // boton 4 start
  myGLCD.setColor(0, 255, 0); //magenta
  myGLCD.fillRoundRect(B4_X, B4_Y, B4_W, B4_H);
  myGLCD.setTextSize(2);
  myGLCD.setTextColor(WHITE);
  myGLCD.print("START", (B4_X + 15), (B4_H - B4_Y) / 2);

  // boton 5 stop
  myGLCD.setColor(255, 0, 0);
  myGLCD.fillRoundRect(B5_X, B5_Y, B5_W, B5_H);
  myGLCD.setTextSize(2);
  myGLCD.setTextColor(WHITE);
  myGLCD.print("STOP", (B5_X + 25), (B5_Y + 45));

  // actual value
  myGLCD.setTextSize(2);
  myGLCD.setTextColor(WHITE);
  myGLCD.print("Time", TZ_X + MARGIN, TZ_Y + MARGIN);
  // value unit
  myGLCD.setTextSize(3);
  myGLCD.setTextColor(WHITE);
  myGLCD.print("[Seg]", TZ_X + MARGIN, TZ_Y + MARGIN+DELTA_Y);

  // time
  myGLCD.setTextSize(1);
  myGLCD.setTextColor(WHITE);
  myGLCD.print("Time:", TIME_X, TIME_Y);
  // time value
  myGLCD.setTextSize(2);
  myGLCD.setTextColor(WHITE);
  myGLCD.printNumI(staytime, TIME_X + DELTA_X, TIME_Y+DELTA_Y);

  // angle
  myGLCD.setTextSize(1);
  myGLCD.setTextColor(WHITE);
  myGLCD.print("Angle:", ANGLE_X, ANGLE_Y);
  // angle value
  myGLCD.setTextSize(2);
  myGLCD.setTextColor(WHITE);
  myGLCD.printNumI(angle, ANGLE_X + DELTA_X, ANGLE_Y+DELTA_Y);

  myGLCD.setTextSize(1);
  myGLCD.setTextColor(WHITE);
  myGLCD.print("FabLab U. de Chile", TZ_X+2, H-2*MARGIN);

}

int getSelection() {

  int sel = 0;
  // boton 1
  if (Z > MINPRESSURE && Z < MAXPRESSURE) {
    if ((X > B1_X && X < B1_W ) && (Y > B1_Y && Y < B1_H ) && b1_flag == LOW) {
      b1_flag = HIGH;
      // cambia de color el boton
      myGLCD.setColor(255, 0, 0);
      myGLCD.fillRoundRect(B1_X, B1_Y, B1_W, B1_H);
      myGLCD.setTextSize(3);
      myGLCD.setTextColor(BLACK);
      myGLCD.print("+", (B1_W - B1_X) / 2, (B1_H - B1_Y) / 2, 1);
      while (Z < MAXPRESSURE) {
        readResistiveTouch();
      }
      // vuelve al color original
      myGLCD.setColor(255, 0, 85); //magenta
      myGLCD.fillRoundRect(B1_X, B1_Y, B1_W, B1_H);
      myGLCD.setTextSize(3);
      myGLCD.setTextColor(WHITE);
      myGLCD.print("+", (B1_W - B1_X) / 2, (B1_H - B1_Y) / 2, 1);
      //Serial.println("released");
      sel = 1;

    }
    if ((X > B2_X && X < B2_W ) && (Y > B2_Y && Y < B2_H ) && b2_flag == LOW ) {
      b2_flag = HIGH;
      myGLCD.setColor(0, 0, 255);
      myGLCD.fillRoundRect(B2_X, B2_Y, B2_W, B2_H);
      myGLCD.setTextSize(3);
      myGLCD.setTextColor(BLACK);
      myGLCD.print("MODE", (B2_W - B2_X) / 4, (B2_H - B2_Y) / 2);
      while (Z < MAXPRESSURE) {
        readResistiveTouch();
      }
      myGLCD.setColor(36, 182, 255); // calipso
      myGLCD.fillRoundRect(B2_X, B2_Y, B2_W, B2_H);
      myGLCD.setTextSize(3);
      myGLCD.setTextColor(WHITE);
      myGLCD.print("MODE", (B2_W - B2_X) / 4, (B2_H - B2_Y) / 2);
      //Serial.println("released");
      sel = 2;
    }

    if ((X > B3_X && X < B3_W ) && (Y > B3_Y && Y < B3_H ) && b3_flag == LOW) {
      b3_flag = HIGH;
      //Serial.print("B3 pressed.. ");
      myGLCD.setColor(255, 0, 0);
      myGLCD.fillRoundRect(B3_X, B3_Y, B3_W, B3_H);
      myGLCD.setTextSize(3);
      myGLCD.setTextColor(BLACK);
      myGLCD.print("-", (B3_W - B3_X) / 2, (B3_H - B3_Y) / 2, 1);
      while (Z < MAXPRESSURE) {
        readResistiveTouch();
      }
      // vuelve al color original
      myGLCD.setColor(255, 0, 85);
      myGLCD.fillRoundRect(B3_X, B3_Y, B3_W, B3_H);
      myGLCD.setTextSize(3);
      myGLCD.setTextColor(WHITE);
      myGLCD.print("-", (B3_W - B3_X) / 2, (B3_H - B3_Y) / 2, 1);
      //Serial.println("released");
      sel = 3;
    }

    if ((X > B4_X && X < B4_W ) && (Y > B4_Y && Y < B4_H ) && b4_flag == LOW) {
      b4_flag = HIGH;
      // cambia de color el boton
      myGLCD.setColor(0, 100, 0); //magenta
      myGLCD.fillRoundRect(B4_X, B4_Y, B4_W, B4_H);
      myGLCD.setTextSize(2);
      myGLCD.setTextColor(WHITE);
      myGLCD.print("START", (B4_X + 15), (B4_H - B4_Y) / 2);
      while (Z < MAXPRESSURE) {
        readResistiveTouch();
      }
      // vuelve al color original
      myGLCD.setColor(0, 255, 0); //magenta
      myGLCD.fillRoundRect(B4_X, B4_Y, B4_W, B4_H);
      myGLCD.setTextSize(2);
      myGLCD.setTextColor(WHITE);
      myGLCD.print("START", (B4_X + 15), (B4_H - B4_Y) / 2);
      //Serial.println("released");
      sel = 4;
    }

    if ((X > B5_X && X < B5_W ) && (Y > B5_Y && Y < B5_H ) && b5_flag == LOW) {
      b5_flag = HIGH;
      // cambia de color el boton
      myGLCD.setColor(100, 0, 0);
      myGLCD.fillRoundRect(B5_X, B5_Y, B5_W, B5_H);
      myGLCD.setTextSize(2);
      myGLCD.setTextColor(WHITE);
      myGLCD.print("STOP", (B5_X + 25), (B5_Y + 45));
      while (Z < MAXPRESSURE) {
        readResistiveTouch();
      }
      // vuelve al color original
      myGLCD.setColor(255, 0, 0); //magenta
      myGLCD.fillRoundRect(B5_X, B5_Y, B5_W, B5_H);
      myGLCD.setTextSize(2);
      myGLCD.setTextColor(WHITE);
      myGLCD.print("STOP", (B5_X + 25), (B5_Y + 45));
      sel = 5;
    }

  }

  return sel;

}

void updateData() {

  myGLCD.setTextSize(2);
  myGLCD.setTextColor(WHITE, BLACK);
  myGLCD.print("        ", TZ_X + MARGIN, TZ_Y + MARGIN);
  if (mode == TIEMPO) {
    myGLCD.print("Time", TZ_X + MARGIN, TZ_Y + MARGIN);
    myGLCD.setTextSize(3);
    myGLCD.print("[Seg]", TZ_X + MARGIN, TZ_Y + MARGIN+DELTA_Y);
  }
  if (mode == ANGULO) {
    myGLCD.print("Angle", TZ_X + MARGIN, TZ_Y + MARGIN);
    myGLCD.setTextSize(3);
    myGLCD.print("[Deg]", TZ_X + MARGIN, TZ_Y + MARGIN+DELTA_Y);
  }

  myGLCD.setTextSize(2);
  myGLCD.print("    ", TIME_X + DELTA_X, TIME_Y + DELTA_Y);
  myGLCD.printNumI(staytime, TIME_X + DELTA_X, TIME_Y + DELTA_Y);

  myGLCD.print("    ", ANGLE_X + DELTA_X, ANGLE_Y + DELTA_Y);
  myGLCD.printNumI(angle, ANGLE_X + DELTA_X, ANGLE_Y + DELTA_Y);
}

void touchB1() {
  if (mode == TIEMPO) {
    staytime += DELTA_TIME;
    if (angle > MAX_TIME) angle = MAX_TIME;
  }

  else {
    angle += DELTA_ANGLE;
    if (angle > MAX_DEG) angle = MAX_DEG;

  }
}

void touchB2() {
  if (mode == TIEMPO) {
    mode = ANGULO;
  }
  else {
    mode = TIEMPO;
  }

}

void touchB3() {
  if (mode == TIEMPO) {
    staytime -= DELTA_TIME;
    if (staytime < MIN_TIME)staytime = MIN_TIME;
  }
  else {
    angle -= DELTA_ANGLE;
    if (angle < MIN_DEG) angle = MIN_DEG;
    ;
  }
}

void touchB4() {

  en_st = 1;

}

void touchB5() {

  en_st = 0;

}
