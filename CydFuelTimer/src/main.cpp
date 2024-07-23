




#include <Arduino.h>
/*  Rui Santos & Sara Santos - Random Nerd Tutorials
    THIS EXAMPLE WAS TESTED WITH THE FOLLOWING HARDWARE:
    1) ESP32-2432S028R 2.8 inch 240×320 also known as the Cheap Yellow Display (CYD): https://makeradvisor.com/tools/cyd-cheap-yellow-display-esp32-2432s028r/
      SET UP INSTRUCTIONS: https://RandomNerdTutorials.com/cyd/
    2) REGULAR ESP32 Dev Board + 2.8 inch 240x320 TFT Display: https://makeradvisor.com/tools/2-8-inch-ili9341-tft-240x320/ and https://makeradvisor.com/tools/esp32-dev-board-wi-fi-bluetooth/
      SET UP INSTRUCTIONS: https://RandomNerdTutorials.com/esp32-tft/
    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/

#include <SPI.h>

/*  Install the "TFT_eSPI" library by Bodmer to interface with the TFT Display - https://github.com/Bodmer/TFT_eSPI
    *** IMPORTANT: User_Config.h available on the internet will probably NOT work with the examples available at Random Nerd Tutorials ***
    *** YOU MUST USE THE User_Config.h FILE PROVIDED IN THE LINK BELOW IN ORDER TO USE THE EXAMPLES FROM RANDOM NERD TUTORIALS ***
    FULL INSTRUCTIONS AVAILABLE ON HOW CONFIGURE THE LIBRARY: https://RandomNerdTutorials.com/cyd/ or https://RandomNerdTutorials.com/esp32-tft/   */
#define TOUCH_CS 33 //T_CS (defined only to prevent warning about it not being defined)
#include <TFT_eSPI.h>

// Install the "XPT2046_Touchscreen" library by Paul Stoffregen to use the Touchscreen - https://github.com/PaulStoffregen/XPT2046_Touchscreen
// Note: this library doesn't require further configuration
#include <XPT2046_Touchscreen.h>

TFT_eSPI tft = TFT_eSPI();

// Touchscreen pins
#define XPT2046_IRQ 36   // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK 25   // T_CLK
#define XPT2046_CS 33    // T_CS

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define FONT_SIZE 2

// Potentiometer is connected to GPIO 34 (Analog ADC1_CH6) 
const int potPin = 35;
// variable for storing the potentiometer value
int potValue = 0;

signed long LeftFuelTimerReset = 110 * 60000 ;  // reset minutes for Left Fuel Tank (60000 millis=minute)
signed long RightFuelTimerReset = 110 * 60000;  // reset minutes for Right Fuel Tank(60000 millis=minute)
bool LeftTimerON = false;
bool RightTimerON = false;
bool DisplaySetLeft = false;
bool DisplaySetRight = false;
signed long LeftTimer = LeftFuelTimerReset;  //Left timer in millis
signed long RightTimer = RightFuelTimerReset; //Right timer in millis
long LoopTime; // time it took t
long LoopStart;
int LeftSeconds;
int RightSeconds;
int fillcolor=TFT_BLACK;
int btn;

// Using two fonts since numbers are nice when bold
#define LABEL1_FONT &FreeSansOblique12pt7b // Key label font 1
#define LABEL2_FONT &FreeSansBold12pt7b    // Key label font 2

// Keypad start position, key sizes and spacing
#define KEY_X 80// Centre of key
#define KEY_Y 64
#define KEY_W 80 // Width and height
#define KEY_H 50
#define KEY_SPACING_X 0 // X and Y gap
#define KEY_SPACING_Y 0
#define KEY_TEXTSIZE 1   // Font size multiplier
// Numeric display box size and location
#define DISP_X 1
#define DISP_Y 10
#define DISP_W 238
#define DISP_H 50
#define DISP_TSIZE 3
#define DISP_TCOLOR TFT_CYAN

// Number length, buffer for storing it and character index
#define NUM_LEN 12
char numberBuffer[NUM_LEN + 1] = "";
uint8_t numberIndex = 0;

// We have a status line for messages
#define STATUS_X 120 // Centred on this
#define STATUS_Y 65

// Create 15 keys for the keypad
char keyLabel[15][5] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "Del", "0", "Ent" };
uint16_t keyColor[15] = {TFT_BLUE, TFT_BLUE, TFT_BLUE,
                         TFT_BLUE, TFT_BLUE, TFT_BLUE,
                         TFT_BLUE, TFT_BLUE, TFT_BLUE,
                         TFT_RED, TFT_BLUE, TFT_GREEN,
                        };
// Touchscreen coordinates: (x, y) and pressure (z)
int x, y, z;

// Print Touchscreen info about X, Y and Pressure (Z) on the Serial Monitor
void printTouchToSerial(int touchX, int touchY, int touchZ) {
  Serial.print("X = ");
  Serial.print(touchX);
  Serial.print(" | Y = ");
  Serial.print(touchY);
  Serial.print(" | Pressure = ");
  Serial.print(touchZ);
  Serial.println();
}

// Print Touchscreen info about X, Y and Pressure (Z) on the TFT Display
void printTouchToDisplay(int touchX, int touchY, int touchZ) {
  // Clear TFT screen
  // tft.fillScreen(TFT_WHITE);
  // tft.setTextColor(TFT_BLACK, TFT_YELLOW);
  // tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);

  int centerX = 180;
  int textY = 180;
 
  String tempText = "X = " + String(touchX);
  // tft.drawCentreString(tempText, centerX, textY, FONT_SIZE);

  // textY += 20;
  // tempText = "Y = " + String(touchY);
  // tft.drawCentreString(tempText, centerX, textY, FONT_SIZE);

  // textY += 20;
  // tempText = "Pressure = " + String(touchZ);
  // tft.drawCentreString(tempText, centerX, textY, FONT_SIZE);

    textY += 20;
  tempText = "Pwr = " + String(potValue);
  tft.setTextColor(TFT_GREEN, TFT_YELLOW);
  tft.drawCentreString(tempText, 180, 180,2);
}

//------------------------------------------------------------------------------------------

// Print something in the mini status bar
void status(const char *msg) {
  tft.setTextPadding(240);
  //tft.setCursor(STATUS_X, STATUS_Y);
  tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
  tft.setTextFont(0);
  tft.setTextDatum(TC_DATUM);
  tft.setTextSize(1);
  tft.drawString(msg, STATUS_X, STATUS_Y);
}

void stopproc() {
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawCentreString("<S>", 25  , 25, 4);
  delay(5000);
tft.drawCentreString("<G>", 25  , 25, 4);
delay(1000);
}
int8_t button8check() {
// check for button press return button pressed or 0 if button not pressed

// check for button press
if (touchscreen.tirqTouched() && touchscreen.touched()) {
   // Calibrate Touchscreen points with map function to the correct width and height
    TS_Point p = touchscreen.getPoint();
    x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
    y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
    z = p.z;
// left screen button 1 through 4
 if (x < 160) {  //Left Side of screen
   if (y <60){   //btn 1
    
    return 1;
    }
       if (y <120){   //btn 1
    return 2;
    }
       if (y <180){   //btn 1
    return 3;
    }
       if (y <240){   //btn 1
    return 4;
    }
 }
  if (x > 160) {  //Right Side of screen
   if (y <60){   //btn 1
 //  tft.drawCentreString("<5>", 25  , 25, 4);delay(2500);
    return 5;
    
    }
       if (y <120){   //btn 1
  //     tft.drawCentreString("<6>", 25  , 25, 4);delay(2500);
    return 6;
    
    }
       if (y <180){   //btn 1
 //      tft.drawCentreString("<7>", 25  , 25, 4);delay(2500);
    return 7;
    }
       if (y <240){   //btn 1
  //     tft.drawCentreString("<8>", 25  , 25, 4);delay(2500);
      return 8;
    
    }
 }
}
else   return 0;  //no button pressed

}



//*********************************SETUP***************************************/
void setup() {

  Serial.begin(115200);

 Serial.println("TTEMP CYD Example Starting");
  // Start the SPI for the touchscreen and init the touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI); // 1.4 version touch feature working
   // touchscreen.begin(); // (1.3 version) touch feature not working
  // Set the Touchscreen rotation in landscape mode
  // Note: in some displays, the touchscreen might be upside down, so you might need to set the rotation to 3: touchscreen.setRotation(3);
  touchscreen.setRotation(1);

  // Start the tft display
  tft.init();
  // Set the TFT display rotation in landscape mode
  tft.setRotation(1);

  // Clear the screen before writing to it
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  
  // Set X and Y coordinates for center of display
  int centerX = SCREEN_WIDTH / 2;
  int centerY = SCREEN_HEIGHT / 2;



  
}

//  ********************************************LOOP *************************************
void loop() {
  LoopStart=millis();
  //******************************Check Voltage for shutdown**************************** */
    potValue = analogRead(potPin);
  Serial.println(potValue);
  String tempText = "Pwr = " + String(potValue);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawCentreString(tempText, 180, 180,2);
  //*******************************update timer values***********************************
  if (LeftTimerON){  
    LeftTimer = LeftTimer - LoopTime;
  }
  if (RightTimerON){
    RightTimer = RightTimer - LoopTime;
  }

  int centerX = SCREEN_WIDTH / 2;
  int centerY = SCREEN_HEIGHT / 2;
 
  

  //***************************************set Seconds Values******************************
  LeftSeconds = LeftTimer%60000;
  RightSeconds =  RightTimer%60000;
  //*************************************Update Left Timer Colors if ON *****************
  if (LeftTimerON ) {
   if ((LeftTimer/60000) < 5 && !DisplaySetLeft && !DisplaySetRight) {   //Start flashing if below this value

      if ((LeftSeconds/1000)%2 == 0) {
       tft.setTextColor(TFT_GREEN, TFT_BLACK);
       if (fillcolor == TFT_YELLOW) {
        //  tft.fillScreen(TFT_BLACK);
          tft.fillRect(0,0,160,240,TFT_BLACK);
          fillcolor=TFT_BLACK;
       }
      }
      else {
        tft.setTextColor(TFT_RED, TFT_YELLOW);
        if (fillcolor == TFT_BLACK) {
         // tft.fillScreen(TFT_YELLOW);
          tft.fillRect(0,0,160,240,TFT_YELLOW);
          fillcolor=TFT_YELLOW;
       }
      }

   }
   else    tft.setTextColor(TFT_GREEN, TFT_BLACK);
  }

 //*************************************Update RightTimer Colors if ON *****************
    if (RightTimerON ) {
   if ((RightTimer/60000) < 5 && !DisplaySetLeft && !DisplaySetRight ) {   //Start flashing if below this value

       if ((RightSeconds/1000)%2 == 0) {
           tft.setTextColor(TFT_GREEN, TFT_BLACK);
            if (fillcolor == TFT_YELLOW) {
             // tft.fillScreen(TFT_BLACK);
              tft.fillRect(160,0,320,240,TFT_BLACK);
              fillcolor=TFT_BLACK;
              }
          }
       else {
            tft.setTextColor(TFT_RED, TFT_YELLOW);
            if (fillcolor == TFT_BLACK) {
              //tft.fillScreen(TFT_YELLOW);
              tft.fillRect(160,0,320,240,TFT_YELLOW);
              fillcolor=TFT_YELLOW;
              }
            }

   }
   else    tft.setTextColor(TFT_GREEN, TFT_BLACK);
  }
  
  //******************************Update if both timers are off */
  if ((LeftTimerON == false) && (RightTimerON == false)) {
      if (fillcolor != TFT_BLACK) {
       tft.fillScreen(TFT_BLACK);
       fillcolor=TFT_BLACK;
      } 
   tft.setTextColor(TFT_GREEN, TFT_BLACK);
 }

 //**********************************************if RIGHT Setup Enabled******************************** */
btn = button8check();
if ( (DisplaySetLeft || DisplaySetRight) && btn > 0 ) delay(500); //delay buton press to debounce or prevent double button tap
  if (DisplaySetRight) { //Draw Right Setup buttons
    
    
    tft.setTextColor(TFT_BLUE, TFT_YELLOW);
   // tft.setTextSize(2);
    
    tft.drawCentreString("   UP  ", 80, 25,  4);
    tft.drawCentreString(" DOWN", 80, 75,  4);
    tft.drawCentreString("RESET", 80, 135, 4);
    tft.drawCentreString(" DONE", 80, 185, 4);
    if (btn==1) { //Down}
     RightTimer=RightTimer+5*60000;
     if (RightTimer > 100*60000) RightTimer =100*60000;
    }
        if (btn==2) { //up}
     RightTimer=RightTimer- 5*60000;
     if (RightTimer < 0){
       RightTimer= 0 ;
       RightTimerON=false;
     }
    }
    if (btn==3)  { //reset}
      RightTimer=RightFuelTimerReset;
       DisplaySetRight=false;
         tft.fillRect(0,0,160,240,TFT_BLACK);
         RightTimerON=false;
        }
    if (btn==4 || btn==5)  { //Done}
      DisplaySetRight=false;
    tft.fillRect(0,0,160,240,TFT_BLACK);
    }
     if (btn==6 ){  //Use Set/Reset to Exit
          tft.fillRect(160,0,160,80,TFT_BLUE);delay(500);tft.fillRect(160,0,160,80,TFT_BLACK);
                DisplaySetRight =  false;
        tft.fillRect(0,0,160,240,TFT_BLACK);
      }

   } else {  //Draw Left Timer
   if (LeftTimerON) tft.setTextColor(TFT_GREEN, TFT_BLACK); else tft.setTextColor(TFT_WHITE,TFT_BLACK);
  if (LeftTimer >= 100*60000)  tft.drawCentreString(String(LeftTimer/60000), 80, centerY/2+10, 8 ); else  tft.drawCentreString(" "+String(LeftTimer/60000)+" ", 80, centerY/2+10, 8 );
  tft.drawCentreString(String(" ")+String(LeftSeconds/1000)+String(" "), 80, centerY/2+100, 6 );
    tft.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
    tft.drawCentreString("Set/Reset", 75, 25, 4);
  }
   //********************************if Left setup enabled***************************************

  if (DisplaySetLeft) { //Draw Right Setup buttons
   
    tft.setTextColor(TFT_BLUE, TFT_YELLOW);
   // tft.setTextSize(2);
       tft.drawCentreString("   UP  ", 240, 25,  4);
    tft.drawCentreString(" DOWN", 240, 75,  4);
    tft.drawCentreString("RESET", 240, 135, 4);
    tft.drawCentreString(" DONE", 240, 195, 4);
    if (btn==5 )  { //UP}
      LeftTimer = LeftTimer + 5 * 60000;
      if (LeftTimer > 100*60000) LeftTimer = 100*60000;
    }
        if (btn==6 )  { //Down}
        LeftTimer = LeftTimer - 5 * 60000;
        if (LeftTimer < 0){
          LeftTimer = 0;
          LeftTimerON= false;
        }
    }
        if (btn==7)  { //reset}
        LeftTimer=LeftFuelTimerReset;
        DisplaySetLeft =  false;
        tft.fillRect(160,0,160,240,TFT_BLACK);
        LeftTimerON = false;
    }
        if (btn==8 )  { //Done}
        DisplaySetLeft =  false;
        tft.fillRect(160,0,160,240,TFT_BLACK);
    }
      if (btn==1 ){  //Use Set/Reset to Exit
        tft.fillRect(0,0,160,80,TFT_BLUE);delay(500);tft.fillRect(0,0,160,80,TFT_BLACK);
                DisplaySetLeft =  false;
        tft.fillRect(160,0,160,240,TFT_BLACK);
      }
      


  } else {  //Draw Right Timer


  if (RightTimerON) tft.setTextColor(TFT_GREEN, TFT_BLACK); else tft.setTextColor(TFT_WHITE,TFT_BLACK);
  if (RightTimer >= 100 *60000)  tft.drawCentreString(String(RightTimer/60000)+" ", centerX+98, centerY/2+10, 8); else tft.drawCentreString(" "+String(RightTimer/60000)+" ", centerX+90, centerY/2+10, 8);
    if (RightTimer >= 100 *60000) tft.drawCentreString(" "+String(RightSeconds/1000)+" ", centerX+85, centerY/2+100, 6); else tft.drawCentreString(" "+String(RightSeconds/1000)+" ", centerX+90, centerY/2+100, 6);
  tft.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  tft.drawCentreString("Set/Reset", 245 ,25, 4);

  }
//  //********************************Update Right Timer display if ON ******************
// if ((RightTimer/60000) <= 3 ) {   //start flashing number as few minutes remaingin
//  if ((RightTimer/60000) < 54 ) {
//        if ((RightSeconds/1000)%2 == 0) {tft.setTextColor(TFT_RED, TFT_BLACK);}
//        else tft.setTextColor(TFT_RED, TFT_YELLOW);
//    }
//    else    tft.setTextColor(TFT_GREEN, TFT_BLACK);
//     }
//    else tft.setTextColor(TFT_GREEN, TFT_BLACK);
//   tft.drawCentreString(" "+String(RightTimer/60000)+" ", centerX+90, centerY/2+10, 8);
//   tft.drawCentreString(" "+String(RightSeconds/1000)+" ", centerX+100, centerY/2+100, 6);




  //---------------------------------------------------------------------------------------------------------------------
  //don't check touch screen if setup is enable
  if ( !DisplaySetLeft && !DisplaySetRight) {
     tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawCentreString("Tap # to Start/Stop", 170  , 215, 4);
  // Checks if Touchscreen was touched, and prints X, Y and Pressure (Z) info on the TFT display and Serial Monitor
  if (touchscreen.tirqTouched() && touchscreen.touched()) {
    // Get Touchscreen points
    TS_Point p = touchscreen.getPoint();
    // Calibrate Touchscreen points with map function to the correct width and height
    x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
    y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
    z = p.z;

    printTouchToSerial(x, y, z);
    printTouchToDisplay(x, y, z);
    //**************************Check/Show Set Buttons****************************

    //**************************Turn ON/Off Timers********************************
    if (x < 170) {  //Left Side of screen
      if (y >80){   //Lower part of screen
          tft.fillRect(0,60,160,240,TFT_BLUE);
          delay(500);
          tft.fillRect(0,60,160,240,TFT_BLACK);
        if (LeftTimerON){
          LeftTimerON= false;

        }
        else {
          LeftTimerON = true;
          RightTimerON = false;
        }

      }
      else {  //Upper left Screen (set/reset)
          tft.fillRect(0,0,160,80,TFT_BLUE);
          delay(500);
          tft.fillRect(0,0,160,80,TFT_BLACK);
          DisplaySetLeft = true;
          tft.fillRect(160,0,160,240,TFT_BLACK); //delay(1000);
          
   //   if (LeftTimer < (60000 * 5)) LeftTimer=LeftTimer + LeftFuelTimerReset; else LeftTimer = LeftFuelTimerReset;
      }
    
    }
    else {  //Right side of Screen
      if (y >80){  //Lower part of Screen
          tft.fillRect(160,60,320,240,TFT_BLUE);
          delay(500);
          tft.fillRect(160,60,320,240,TFT_BLACK);
      if (RightTimerON){
          RightTimerON= false;
        }
        else {
          RightTimerON = true;
          LeftTimerON=false;
        }
      }
      else {  //Upper Right part of screen (set/rest)
                tft.fillRect(160,0,320,80,TFT_BLUE);
          delay(500);
          tft.fillRect(160,0,320,80,TFT_BLACK);
          DisplaySetRight=true;
          tft.fillRect(0,0,160,240,TFT_BLACK);
    //  if (RightTimer < (60000 * 5)) RightTimer=RightTimer + RightFuelTimerReset; else RightTimer = RightFuelTimerReset;
      
      } 
    }

  }

    delay(100);
  }
  LoopTime=millis()-LoopStart;
  //  tft.drawCentreString(String(LeftTimer), 180,140, 2);
  // tft.drawCentreString(String(LoopTime), 180,160, 2);
}

