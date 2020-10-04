#include <Adafruit_NeoPixel.h>
#include "pitches.h"

#define PIN 6
#define MAX_LED 5
 
#define ADD true
#define SUB false
  
int val = 0;
boolean stat = ADD;
 
// Parameter 1 = ws2811级联数量
// Parameter 2 = arduino PIN
// Parameter 3 = pixel type flags, add together as needed:
// NEO_KHZ800 800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
// NEO_KHZ400 400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
// NEO_GRB Pixels are wired for GRB bitstream (most NeoPixel products)
// NEO_RGB Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel( MAX_LED, PIN, NEO_RGB + NEO_KHZ800 );


#include <Wire.h>  // Wire library - used for I2C communication
int ADXL345 = 0x53; // The ADXL345 sensor I2C address
double X_out, Y_out, Z_out, G_out = 0, FG_out = 0;  // Outputs
double peak = 0;
double b = 0.01;  // 低通过滤器系数

double filter(double oldValue, double newValue)
{
    return b * oldValue + (1 - b) * newValue;
}


// notes in the melody:
int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};

signed int feqTone = 0;
bool isAlarm = false;
bool isUpTone = true;

void beep() {
  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < 8; thisNote++) {

    // to calculate the note duration, take one second divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(8, melody[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(8);
  }
}

void alarm() 
{
    if (isAlarm) {
        if (isUpTone) {
            tone(8, 700 + feqTone * 10, 19);
            delay(2);
            feqTone++;
            if (feqTone > 80) {
                isUpTone = false;
                feqTone = 0;
            }
        } else {
            tone(8, 1500 - feqTone * 10, 44);
            delay(4);
            feqTone--;
            if (feqTone <= 0) {
                isUpTone = true;
                feqTone = 0;
            }
        }
    }  
}

void setupLed()
{
    strip.begin();           //初始化Adafruit_NeoPixel；
    strip.show();           //显示所有LED为关状态;
}

void setupADXL()
{
    Serial.begin(9600); // Initiate serial communication for printing the results on the Serial monitor
    Wire.begin(); // Initiate the Wire library
    // Set ADXL345 in measuring mode
    Wire.beginTransmission(ADXL345); // Start communicating with the device 
    Wire.write(0x2D); // Access/ talk to POWER_CTL Register - 0x2D
    // Enable measurement
    Wire.write(8); // (8dec -> 0000 1000 binary) Bit D3 High for measuring enable 
    Wire.endTransmission();
    delay(10);

    // This code goes in the SETUP section
    // Off-set Calibration
    //X-axis
    /*
    Wire.beginTransmission(ADXL345);
    Wire.write(0x1E);  // X-axis offset register
    Wire.write(1);
    Wire.endTransmission();
    delay(10);
    */
    //Y-axis
    Wire.beginTransmission(ADXL345);
    Wire.write(0x1F); // Y-axis offset register
    Wire.write(-1);
    Wire.endTransmission();
    delay(10);
  
    //Z-axis
    Wire.beginTransmission(ADXL345);
    Wire.write(0x20); // Z-axis offset register
    Wire.write(7);
    Wire.endTransmission();
    delay(10);
}

void setup()
{
    setupLed();
    setupADXL();
    //beep();
    //alarm();
}

uint8_t a = 0;
double oldValue = 0;
bool isUp = false;
double peakValue = 0;

int time_left = 0;

void runADXL()
{
    // === Read acceleromter data === //
    Wire.beginTransmission(ADXL345);
    Wire.write(0x32);                           // Start with register 0x32 (ACCEL_XOUT_H)
    Wire.endTransmission(false);
    Wire.requestFrom(ADXL345, 6, true);         // Read 6 registers total, each axis value is stored in 2 registers
    
    X_out = ( Wire.read() | Wire.read() << 8);  // X-axis value
    X_out = X_out / 256;                        // For a range of +-2g, we need to divide the raw values by 256, according to the datasheet
    Y_out = ( Wire.read()| Wire.read() << 8);   // Y-axis value
    Y_out = Y_out / 256;
    Z_out = ( Wire.read()| Wire.read() << 8);   // Z-axis value
    Z_out = Z_out / 256;

    /*
    Serial.print("Xa= ");
    Serial.print(X_out);
    Serial.print("   Ya= ");
    Serial.print(Y_out);
    Serial.print("   Za= ");
    Serial.println(Z_out);
    */
    G_out = sqrt(square(X_out) + square(Y_out) + square(Z_out));
    FG_out = filter(FG_out, G_out);


    if (oldValue != 0) {
        if (oldValue <= G_out) {
            isUp = true;
            Serial.println("Up......");
        } else {
            if (isUp == false) {
                peakValue = oldValue;
            }
            
            isUp = false;
            Serial.println("Down......");
        }
    }

    oldValue = G_out;

    Serial.print("Ga= ");
    Serial.print(G_out);
    Serial.print(",     Peak= ");
    Serial.println(peakValue+2);

    if (G_out > 1.8) {
        time_left = 3000; // light for 3000ms
        isAlarm = true;
    }
    else if (G_out < 0.5) {
        //time_left = 0;
    }
}

uint32_t redColor = strip.Color(0,128,0);         //选择所显示的颜色
uint32_t blueColor = strip.Color(0,0,128);         //选择所显示的颜色
uint32_t greenColor = strip.Color(128,0,0);         //选择所显示的颜色

void runLed()
{
    if (time_left == 0) {
        for (uint8_t i = 0;i < MAX_LED; i++)  {
            strip.setPixelColor(i, 0);             //使其他LED全灭;
        }
        strip.show();
        isAlarm = false;
        return;
    }
    
    uint8_t i;                                      
    

    for (i = 0; i < MAX_LED; i++) {
        uint32_t color = 0;
        switch ( a % 3) {
            case 0: color = redColor; break;
            case 1: color = blueColor; break;
            case 2: color = greenColor; break;
            default: break;
        }
        if (i == a)  {
            strip.setPixelColor(i, color);     //第几个LED点亮;
        } else {
            strip.setPixelColor(i, 0);             //使其他LED全灭;
        }
    }
    
    strip.show();                                //是LED显示所选的颜色;
    a = (a+1) % 5;  

    time_left -= 50;
}

void loop()
{
    runADXL();
    runLed();
    alarm();
    //delay(5);                                   //延时20ms；
}
