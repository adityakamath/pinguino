#include <ArduinoHttpClient.h>
#include <BlynkSimpleWiFiNINA.h>

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

// Define NeoPixel pin and initialize object
#define PIN 6

Adafruit_NeoPixel strip = Adafruit_NeoPixel(24, PIN, NEO_GRB + NEO_KHZ800);

const char ssid[] = SECRET_SSID;    // Network SSID (name)
const char pass[] = SECRET_PASS;    // Network password (use for WPA, or use as key for WEP)
char       auth[] = SECRET_AUTH;    // Blynk API token

int onoff         = 1;  // default value of on/off button = 1 (On)
int lampface      = 4;  // default lamp face = 4 (Solid Color)
int discord_onoff = 0;  // default discord notifications = 0 (Disabled)
int slider        = 50; // default intensity = 50 (0-255)
int red           = 255;  // default red intensity = 0
int green         = 255;  // default green intensity = 0
int blue          = 255;  // default blue intensity = 0
int prev_lampface = lampface;
int prev_onoff    = onoff;
int prev_discord  = discord_onoff;
int hasRun        = 0;

// Blynk functions to retrieve values
BLYNK_WRITE(V0){
  onoff = param.asInt();
}
BLYNK_WRITE(V1){
  lampface = param.asInt();
}
BLYNK_WRITE(V2){
  discord_onoff = param.asInt();
}
BLYNK_WRITE(V3){
  slider = param.asInt();
}
BLYNK_WRITE(V4){
  red   = param[0].asInt();
  green = param[1].asInt();
  blue  = param[2].asInt();
}

const char server[] = "discordapp.com";
const int port = 443;
const String discord_webhook = SECRET_WEBHOOK;

int status = WL_IDLE_STATUS;
WiFiSSLClient client;
HttpClient http_client = HttpClient(client, server, port);

//NEOFIRE STUFF -----------https://github.com/RoboUlbricht/arduinoslovakia/blob/master/neopixel/neopixel_fire01/neopixel_fire01.ino----------------------------------
uint32_t fire_color   = strip.Color ( 80,  35,  00);
uint32_t off_color    = strip.Color (  0,  0,  0);

///
/// Fire simulator
///
class NeoFire
{
  Adafruit_NeoPixel &strip;
 public:

  NeoFire(Adafruit_NeoPixel&);
  void Draw();
  void Clear();
  void AddColor(uint8_t position, uint32_t color);
  void SubstractColor(uint8_t position, uint32_t color);
  uint32_t Blend(uint32_t color1, uint32_t color2);
  uint32_t Substract(uint32_t color1, uint32_t color2);
};

///
/// Constructor
///
NeoFire::NeoFire(Adafruit_NeoPixel& n_strip)
: strip (n_strip)
{
}

///
/// Set all colors
///
void NeoFire::Draw()
{
Clear();

for(int i=0;i<24;i++)
  {
  AddColor(i, fire_color);
  int r = random(80);
  uint32_t diff_color = strip.Color ( r, r/2, r/2);
  SubstractColor(i, diff_color);
  }
  
strip.show();
}

///
/// Set color of LED
///
void NeoFire::AddColor(uint8_t position, uint32_t color)
{
uint32_t blended_color = Blend(strip.getPixelColor(position), color);
strip.setPixelColor(position, blended_color);
}

///
/// Set color of LED
///
void NeoFire::SubstractColor(uint8_t position, uint32_t color)
{
uint32_t blended_color = Substract(strip.getPixelColor(position), color);
strip.setPixelColor(position, blended_color);
}

///
/// Color blending
///
uint32_t NeoFire::Blend(uint32_t color1, uint32_t color2)
{
uint8_t r1,g1,b1;
uint8_t r2,g2,b2;
uint8_t r3,g3,b3;

r1 = (uint8_t)(color1 >> 16),
g1 = (uint8_t)(color1 >>  8),
b1 = (uint8_t)(color1 >>  0);

r2 = (uint8_t)(color2 >> 16),
g2 = (uint8_t)(color2 >>  8),
b2 = (uint8_t)(color2 >>  0);

return strip.Color(constrain(r1+r2, 0, 255), constrain(g1+g2, 0, 255), constrain(b1+b2, 0, 255));
}

///
/// Color blending
///
uint32_t NeoFire::Substract(uint32_t color1, uint32_t color2)
{
uint8_t r1,g1,b1;
uint8_t r2,g2,b2;
uint8_t r3,g3,b3;
int16_t r,g,b;

r1 = (uint8_t)(color1 >> 16),
g1 = (uint8_t)(color1 >>  8),
b1 = (uint8_t)(color1 >>  0);

r2 = (uint8_t)(color2 >> 16),
g2 = (uint8_t)(color2 >>  8),
b2 = (uint8_t)(color2 >>  0);

r=(int16_t)r1-(int16_t)r2;
g=(int16_t)g1-(int16_t)g2;
b=(int16_t)b1-(int16_t)b2;
if(r<0) r=0;
if(g<0) g=0;
if(b<0) b=0;

return strip.Color(r, g, b);
}

///
/// Every LED to black
///
void NeoFire::Clear()
{
for(uint16_t i=0; i<strip.numPixels (); i++)
  strip.setPixelColor(i, off_color);
}

NeoFire fire(strip);
//END OF NEOFIRE STUFF --------------------------------------------------------------------

void setup() {
  strip.begin();
  strip.setBrightness(slider);
  strip.show(); // Initialize all pixels to 'off'
  Blynk.begin(auth, ssid, pass); // start Blynk functionalities and connect to WiFi
}

void loop() {
  
  // Variables to temporarily store the combination
  int temp_onoff    = onoff;
  int temp_lampface = lampface;
  int temp_slider   = slider;
  int temp_r        = red;
  int temp_g        = green;
  int temp_b        = blue;
  int temp_discord  = discord_onoff;
  
  Blynk.run();
    
  if (temp_onoff == 1){
    if(prev_onoff != temp_onoff || hasRun == 0){ discord_send("Pinguino On");}
    hasRun = 1;
    
    strip.setBrightness(temp_slider);
    
    switch (temp_lampface) {
      case 1: // clock
        if(prev_lampface != temp_lampface){ discord_send("Lampface: Clock");}
        //Serial.println();
        colorWipe(strip.Color(255, 0, 0), 1);
        delay(1000);
        colorWipe(strip.Color(0, 0, 0), 1);
        prev_lampface = 1;
        break;
      
      case 2: // discord input
        if(prev_lampface != temp_lampface){ discord_send("Lampface: Discord");}
        if(temp_discord == 0){ 
          if(prev_discord != temp_discord){ discord_send("Discord input disabled");}
          discordStatusWipe();
          prev_discord = temp_discord;
        }
        else if(temp_discord == 1){
          if(prev_discord != temp_discord){ discord_send("Discord input enabled");}
          discordInput();
          //discordStatusWipe();
          prev_discord = temp_discord;
        }
        prev_lampface = 2;
        break;
        
      case 3: //spotify viz
        if(prev_lampface != temp_lampface){ discord_send("Lampface: Spotify");}
        colorWipe(strip.Color(0, 0, 255), 1);
        prev_lampface = 3;
        break;
        
      case 4: // solid color
        if(prev_lampface != temp_lampface){ discord_send("Lampface: Solid Color");}
        colorWipe(strip.Color(temp_r, temp_g, temp_b), 1);
        prev_lampface = 4;
        break;
        
      case 5: //fire
        if(prev_lampface != temp_lampface){ discord_send("Lampface: Fire");}
        fireface(50);
        prev_lampface = 5;
        break;
        
      case 6: // theatre chase
        if(prev_lampface != temp_lampface){ discord_send("Lampface: Theatre Chase");}
        theaterChase(strip.Color(temp_r, temp_g, temp_b), 50);
        prev_lampface = 6;
        break;
        
      case 7: // rainbow
        if(prev_lampface != temp_lampface){ discord_send("Lampface: Rainbow");}
        rainbow(10);
        prev_lampface = 7;
        break;
        
      case 8: // rainbow cycle
        if(prev_lampface != temp_lampface){ discord_send("Lampface: Rainbow Cycle");}
        rainbowCycle(10);
        prev_lampface = 8;
        break;
        
      case 9: // theatre chase rainbow
        if(prev_lampface != temp_lampface){ discord_send("Lampface: Rainbow Theatre Chase");}
        theaterChaseRainbow(50);
        prev_lampface = 9;
        break;
        
      default:
        if(prev_lampface != temp_lampface){ discord_send("Lampface: Off (Default)");}
        colorWipe(strip.Color(0, 0, 0), 1);
        prev_lampface = 0;
        break;
    }
    prev_onoff = 1;
  }
  else if(temp_onoff == 0){
    if(prev_onoff != temp_onoff){ discord_send("Pinguino Off");}
    colorWipe(strip.Color(0, 0, 0), 1);
    prev_onoff = 0;
  }
}

// Fill dots with color of Discord status
void discordStatusWipe(){
  
  colorWipe(strip.Color(0, 255, 0), 1); //online
  colorWipe(strip.Color(192, 112, 0), 1); //idle
  colorWipe(strip.Color(255, 0, 0), 1); //do not disturb
  colorWipe(strip.Color(70, 0, 190), 1); //offline
}

// Fill dots with color of Discord status
void discordInput(){
  colorWipe(strip.Color(0, 255, 0), 1); //online
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

// Fill all dots with same color, change colors in a loop
void fireface(uint8_t wait) {
  fire.Draw();
  delay(wait);
}

// Fill all dots with same color, change colors in a loop
void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
      for(i=0; i< strip.numPixels(); i++) {
        strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
      }
      strip.show();
      delay(wait);
  }
}

// Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

// Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void discord_send(String content) {
  http_client.post(discord_webhook, "application/json", "{\"content\":\"" + content + "\", \"tts\":" + false + "}");
  //Serial.println("[HTTP] Sending message: " + content);
  
}
