#include <Adafruit_NeoPixel.h>
 
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
float X_out, Y_out, Z_out;  // Outputs

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
}

uint8_t a = 0;

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
    
    Serial.print("Xa= ");
    Serial.print(X_out);
    Serial.print("   Ya= ");
    Serial.print(Y_out);
    Serial.print("   Za= ");
    Serial.println(Z_out);
}

void runLed()
{
    uint8_t i;                                        
    uint32_t color = strip.Color(60, 255, 120);         //选择所显示的颜色

    for (i = 0;i < MAX_LED; i++) {
        if (i == a)  {
            strip.setPixelColor(i, color);     //第几个LED点亮;
        } else {
            strip.setPixelColor(i, 0);             //使其他LED全灭;
        }
    }
    
    strip.show();                                //是LED显示所选的颜色;
    a = (a+1) % 5;                    
}

void loop()
{
    runADXL();
    runLed();
    delay(50);                                   //延时20ms；
}
