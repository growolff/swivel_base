#include <TouchScreen.h>

// These are the pins for the shield!
#define YP A2  // must be an analog pin, use "An" notation!
#define XM A3  // must be an analog pin, use "An" notation!
#define YM 8   // can be a digital pin
#define XP 9   // can be a digital pin

#define MINPRESSURE 100
#define MAXPRESSURE 320

#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_RS A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

#include <Adafruit_GFX.h>// Hardware-specific library
#include <UTFTGLUE.h>              // class methods are in here

UTFTGLUE myGLCD(0x9341, LCD_RS, LCD_WR, LCD_CS, LCD_RESET, LCD_RD);

Adafruit_GFX_Button botones[5];

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
#define TIME_Y  TZ_Y+MARGIN*8
#define ANGLE_X TZ_X+MARGIN
#define ANGLE_Y TZ_Y+MARGIN*11
#define VEL_X   TZ_X+MARGIN
#define VEL_Y   TZ_Y+MARGIN*14
#define DELTA_X 5*MARGIN
#define DELTA_Y 0

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

int tiem = 1, vel = 10;
float angle = 18;
int mode = 1;

#define M_EN 10
#define M_DIR 11
#define M_STEP 12

int en_st = 0;
int dir_st = 0;
int step_st = 0;

int giranding = 0;
int tiempo = 5000;

unsigned long dt = 0;

void readResistiveTouch(void)
{
  digitalWrite(13, HIGH);
  tp = ts.getPoint();
  digitalWrite(13, LOW);

  pinMode(YP, OUTPUT);      //restore shared pins
  pinMode(XM, OUTPUT);
  //digitalWrite(YP, HIGH);   //because TFT control pins
  //digitalWrite(XM, HIGH);

  X = map(tp.x, TS_MAXX, TS_MINX, 0, W); // valores invertidos
  Y = map(tp.y, TS_MINY, TS_MAXY, 0, H);
  Z = tp.z;
  /*
    if (Z > MINPRESSURE && Z < MAXPRESSURE) {
    _press = true;
    }
    else {
    _press = false;
    }*/

  /*
    if (Z > MINPRESSURE && Z < MAXPRESSURE) {
      //Serial.print("x: ");
      //Serial.print(tp.x);
      //Serial.print("\ty: ");
      //Serial.print(tp.y);

      Serial.print("x: ");
      Serial.print(X);
      Serial.print("\ty: ");
      Serial.print(Y);

      Serial.print("\tz: ");
      Serial.println(Z);
    }
  */
}

void setup() {
  Serial.begin(9600);

  pinMode(M_EN, OUTPUT);
  pinMode(M_DIR, OUTPUT);
  pinMode(M_STEP, OUTPUT);
  /*
    randomSeed(analogRead(0));
    pinMode(A0, OUTPUT);
    digitalWrite(A0, HIGH);
  */
  // Setup the LCD
  myGLCD.InitLCD();
  myGLCD.setFont(SmallFont);

  myGLCD.setRotation(1);

  // Clear the screen and draw the frame
  //myGLCD.clrScr();

  myGLCD.fillScreen(BLACK);
  dibujarMenu();

  //myGLCD.setColor(GRAY_1, GRAY_1, GRAY_1);
  myGLCD.setColor(BLACK, BLACK, BLACK);
  myGLCD.drawRect(TZ_X , TZ_Y, TZ_WP , TZ_HP);

  dt = millis();
}

void loop()
{
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
    Serial.print("M_EN: ");
    Serial.println(en_st);
    b4_flag = LOW;
    updateData();
    dt = millis();
  }
  if (sel == 5 && b5_flag == HIGH) {
    touchB5();
    b5_flag = LOW;
    updateData();
  }
  /*
    if ( millis() - dt > 1000 ) {
      //updateData();
      //Serial.print("Selection: ");
      //Serial.println(sel);
      //Serial.print("PRESSED? ");
      //Serial.println(_press ? "SI" : "NO");
      dt = millis();
    }*/

  digitalWrite(M_EN, HIGH); //Giro hacia un sentido
  digitalWrite(M_DIR, HIGH); //Giro hacia un sentido

  if (en_st == 1) {
    digitalWrite(M_EN, LOW); // driver tiene la lógica inversa
    int nsteps = int(angle / 1.8);

    if (giranding == 0) {
      for (int i = 0; i < nsteps; i++)
      {
        digitalWrite(M_STEP, HIGH);
        delayMicroseconds(tiempo);
        digitalWrite(M_STEP, LOW);
        delayMicroseconds(tiempo);
      }
      giranding = 1;
    }
    if ( millis() - dt > tiem * 1000) {
      giranding = 0;
      dt = millis();
    }
    //Serial.println("PASO COMPLETED");
  }

  //myGLCD.setColor(0, 0, 255);
  //myGLCD.drawRect(0, 14, 319, 225);

  //delay(100);
}

void dibujarMenu() {

  //myGLCD.fillRoundRect(desde x , desde y, hasta x , hasta y );

  // boton 1
  myGLCD.setColor(255, 0, 85); //magenta
  myGLCD.fillRoundRect(B1_X, B1_Y, B1_W, B1_H);
  myGLCD.setTextSize(3);
  myGLCD.setTextColor(WHITE);
  myGLCD.print("+1", (B1_W - B1_X) / 2, (B1_H - B1_Y) / 2, 1);

  // boton 2
  myGLCD.setColor(36, 182, 255); // calipso
  myGLCD.fillRoundRect(B2_X, B2_Y, B2_W, B2_H);
  myGLCD.setTextColor(WHITE);
  myGLCD.print("MODE", (B2_W - B2_X) / 4, (B2_H - B2_Y) / 2);

  // boton 3
  myGLCD.setColor(255, 0, 85);
  myGLCD.fillRoundRect(B3_X, B3_Y, B3_W, B3_H);
  //myGLCD.setTextColor(BLACK, WHITE);
  myGLCD.setTextColor(WHITE);
  myGLCD.print("-1", (B3_W - B3_X) / 2, (B3_H - B3_Y) / 2);

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
  myGLCD.print("time", TZ_X + MARGIN, TZ_Y + MARGIN);
  // value
  myGLCD.setTextSize(3);
  myGLCD.setTextColor(WHITE);
  myGLCD.printNumI(0, TZ_X + MARGIN, TZ_Y + MARGIN * 4);

  // time
  myGLCD.setTextSize(1);
  myGLCD.setTextColor(WHITE);
  myGLCD.print("time:  ", TIME_X, TIME_Y, 123);
  // time value
  myGLCD.setTextSize(2);
  myGLCD.setTextColor(WHITE);
  myGLCD.printNumI(tiem, TIME_X + DELTA_X, TIME_Y);

  // angle
  myGLCD.setTextSize(1);
  myGLCD.setTextColor(WHITE);
  myGLCD.print("angle: ", ANGLE_X, ANGLE_Y, 123);
  // angle value
  myGLCD.setTextSize(2);
  myGLCD.setTextColor(WHITE);
  myGLCD.printNumI(angle, ANGLE_X + DELTA_X, ANGLE_Y);

  // vel
  myGLCD.setTextSize(1);
  myGLCD.setTextColor(WHITE);
  myGLCD.print("vel:   ", VEL_X, VEL_Y, 123);
  // vel value
  myGLCD.setTextSize(2);
  myGLCD.setTextColor(WHITE);
  myGLCD.printNumI(vel, VEL_X + DELTA_X, VEL_Y);



  //myGLCD.setColor(64, 64, 64);
  //myGLCD.fillRect(0, 226, 319, 239);
  //myGLCD.setColor(255, 255, 255);
  //myGLCD.setBackColor(255, 0, 0);
  //myGLCD.print("* Universal Color TFT Display Library *", CENTER, 1);
  //myGLCD.setBackColor(64, 64, 64);
  //myGLCD.setColor(255,255,0);
  //myGLCD.print("<http://electronics.henningkarlsen.com>", CENTER, 227);

}

int getSelection() {

  int sel = 0;
  // boton 1
  if (Z > MINPRESSURE && Z < MAXPRESSURE) {
    if ((X > B1_X && X < B1_W ) && (Y > B1_Y && Y < B1_H ) && b1_flag == LOW) {
      b1_flag = HIGH;
      Serial.print("B1 pressed.. ");
      //Serial.print("\tz: ");
      //Serial.println(Z);
      // cambia de color el boton
      myGLCD.setColor(255, 0, 0);
      myGLCD.fillRoundRect(B1_X, B1_Y, B1_W, B1_H);
      myGLCD.setTextSize(3);
      myGLCD.setTextColor(BLACK);
      myGLCD.print("+1", (B1_W - B1_X) / 2, (B1_H - B1_Y) / 2, 1);
      while (Z < MAXPRESSURE) {
        readResistiveTouch();
      }
      // vuelve al color original
      myGLCD.setColor(255, 0, 85); //magenta
      myGLCD.fillRoundRect(B1_X, B1_Y, B1_W, B1_H);
      myGLCD.setTextSize(3);
      myGLCD.setTextColor(WHITE);
      myGLCD.print("+1", (B1_W - B1_X) / 2, (B1_H - B1_Y) / 2, 1);
      Serial.println("released");
      sel = 1;
    }
    if ((X > B2_X && X < B2_W ) && (Y > B2_Y && Y < B2_H ) && b2_flag == LOW ) {
      b2_flag = HIGH;
      Serial.print("B2 pressed.. ");
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
      Serial.println("released");
      sel = 2;
    }
    if ((X > B3_X && X < B3_W ) && (Y > B3_Y && Y < B3_H ) && b3_flag == LOW) {
      b3_flag = HIGH;
      Serial.print("B3 pressed.. ");
      myGLCD.setColor(255, 0, 0);
      myGLCD.fillRoundRect(B3_X, B3_Y, B3_W, B3_H);
      myGLCD.setTextSize(3);
      myGLCD.setTextColor(BLACK);
      myGLCD.print("-1", (B3_W - B3_X) / 2, (B3_H - B3_Y) / 2, 1);
      while (Z < MAXPRESSURE) {
        readResistiveTouch();
      }
      // vuelve al color original
      myGLCD.setColor(255, 0, 85);
      myGLCD.fillRoundRect(B3_X, B3_Y, B3_W, B3_H);
      myGLCD.setTextSize(3);
      myGLCD.setTextColor(WHITE);
      myGLCD.print("-1", (B3_W - B3_X) / 2, (B3_H - B3_Y) / 2, 1);
      Serial.println("released");
      sel = 3;
    }
    if ((X > B4_X && X < B4_W ) && (Y > B4_Y && Y < B4_H ) && b4_flag == LOW) {
      b4_flag = HIGH;
      Serial.print("B4 pressed.. ");
      //Serial.print("\tz: ");
      //Serial.println(Z);
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
      Serial.println("released");
      sel = 4;
    }
    if ((X > B5_X && X < B5_W ) && (Y > B5_Y && Y < B5_H ) && b5_flag == LOW) {
      b5_flag = HIGH;
      Serial.print("B5 pressed.. ");
      //Serial.print("\tz: ");
      //Serial.println(Z);
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
      Serial.println("released");
      sel = 5;
    }
  }

  return sel;
}

void updateData() {

  myGLCD.setTextSize(2);
  myGLCD.setTextColor(WHITE, BLACK);
  myGLCD.print("        ", TZ_X + MARGIN, TZ_Y + MARGIN);
  if (mode == TIEMPO) myGLCD.print("time", TZ_X + MARGIN, TZ_Y + MARGIN);
  if (mode == VELOCIDAD) myGLCD.print("vel", TZ_X + MARGIN, TZ_Y + MARGIN);
  if (mode == ANGULO) myGLCD.print("angle", TZ_X + MARGIN, TZ_Y + MARGIN);

  myGLCD.setTextSize(2);
  myGLCD.setTextColor(WHITE, BLACK);
  myGLCD.print("   ", TIME_X + DELTA_X, TIME_Y);
  myGLCD.printNumI(tiem, TIME_X + DELTA_X, TIME_Y);

  myGLCD.setTextSize(2);
  myGLCD.setTextColor(WHITE, BLACK);
  myGLCD.print("   ", ANGLE_X + DELTA_X, ANGLE_Y);
  myGLCD.printNumI(angle, ANGLE_X + DELTA_X, ANGLE_Y);

  myGLCD.setTextSize(2);
  myGLCD.setTextColor(WHITE, BLACK);
  myGLCD.print("   ", VEL_X + DELTA_X, VEL_Y);
  myGLCD.printNumI(vel, VEL_X + DELTA_X, VEL_Y);

}

void touchB1() {
  if (mode == TIEMPO) {
    tiem++;
  }
  if (mode == VELOCIDAD) {
    vel++;
  }
  if (mode == ANGULO) {
    angle += 1.8;
  }
}

void touchB2() {
  if (mode == TIEMPO) {
    mode = ANGULO;
  }
  else if (mode == VELOCIDAD) {
    mode = TIEMPO;
  }
  else if (mode == ANGULO) {
    mode = VELOCIDAD;
  }
}

void touchB3() {
  if (mode == TIEMPO) {
    tiem--;
  }
  if (mode == VELOCIDAD) {
    vel--;
  }
  if (mode == ANGULO) {
    angle -= 1.8;
  }
}

void touchB4() {
  //Serial.println("START");
  en_st = 1;
  //digitalWrite(M_EN, !en_st);
}

void touchB5() {
  //Serial.println("STOP");
  en_st = 0;
  //digitalWrite(M_EN, !en_st);
}
