/**
  Wiring:  Maybe I have a VS1003, I don't know.  VS1003/VS1053 MP3 Decoder is writen on the board.
  ---------------------
  | VS1053  |  ESP32  |
  ---------------------
  |   SCK   |  IO18   |
  |   MISO  |  IO19   |
  |   MOSI  |  IO23   |
  |   XRST  |  EN     |
  |   CS    |  IO5    |
  |   DCS   |  IO16   |
  |   DREQ  |  IO4    |
  |   5V    |  5V     |
  |   GND   |  GND    |
  ---------------------

  IRremote Wiring
  ----------------------
  | OS-1838  |  ESP32  |
  ----------------------
  |   SCK   |   IO17   |
  |   5V    |   5V     |
  |   GND   |   GND    |
  ----------------------
  
  DHT11 Wiring
  ----------------------
  | DHT11   |  ESP32   |
  ----------------------
  |   SCK   |   IO2    |
  |   5V    |   5V     |
  |   GND   |   GND    |
  ----------------------

  OLED Wiring:  
  ----------------------
  | SSD1306 |  ESP32   |
  ----------------------
  |   SCL   |   IO22   |
  |   SDA   |   IO21   |
  |   3.3V  |   3.3V   |
  |   GND   |   GND    |
  ----------------------

  This needs to be in the platformio.ini:

  [env:esp32dev]
  platform = espressif32
  board = esp32dev
  framework = arduino
  lib_deps =
    ESP_VS1053_Library
    adafruit/Adafruit GFX Library @ 1.11.5
    adafruit/Adafruit SSD1306 @ 2.5.7
    adafruit/DHT sensor library @ 1.4.4
    arduino-libraries/Arduino_JSON @ 0.2.0
    z3t0/IRremote @ 4.1.2
    adafruit/Adafruit Unified Sensor @ 1.1.9

**/
#include <Wire.h>             /*Include wire communication library*/
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h> /*OLED display library*/
#include <Adafruit_Sensor.h>
#include <DHT.h>  
#include <IRremote.h>
#include <VS1053.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>


//-------------------------------------------------------------------------------------------------//
//--------------------------------------------- OLED PART -----------------------------------------//
//-------------------------------------------------------------------------------------------------//
//Screen part.
#define SCREEN_WIDTH 128    /*OLED screen width 128 pixels*/
#define SCREEN_HEIGHT 64   /*OLED screen height 64 pixels*/
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); /*SSD1306 I2C Display initialization*/
//-------------------------------------------------------------------------------------------------//



//-------------------------------------------------------------------------------------------------//
//------------------------------------------ Temp sensor PART -------------------------------------//
//-------------------------------------------------------------------------------------------------//
//Temp sensor part.
#define DHTPIN 2          /*Signal pin for DHT11 Sensor*/
#define DHTTYPE    DHT11  
DHT dht(DHTPIN, DHTTYPE);
//-------------------------------------------------------------------------------------------------//



//-------------------------------------------------------------------------------------------------//
//---------------------------------------------- Menu PART ----------------------------------------//
//-------------------------------------------------------------------------------------------------//
//IR reciver part.
#define RECIVER_PIN 17
IRrecv reciver(RECIVER_PIN);
decode_results results;
/*Serial
int menuOption;
int menuBuffer;
*/
//IR remote
String irNum = "";
int irNumEq = 4;
//-------------------------------------------------------------------------------------------------//



//-------------------------------------------------------------------------------------------------//
//---------------------------------------------- RADIO PART ---------------------------------------//
//-------------------------------------------------------------------------------------------------//
// VS1003/VS1053 pins
#define VS1053_CS     5
#define VS1053_DCS    16
#define VS1053_DREQ   4


// Default volume
#define VOLUME  80

VS1053 player(VS1053_CS, VS1053_DCS, VS1053_DREQ);
WiFiClient client;

// WiFi settings example, substitute your own
const char *ssid = "magor";
const char *password = "19960716";

//  http://comet.shoutca.st:8563/1
const char *host = "icast.connectmedia.hu";
const char *path = "/5201/live.mp3";
int httpPort = 80;

// The buffer size 64 seems to be optimal. At 32 and 128 the sound might be brassy.
uint8_t mp3buff[64];
//-------------------------------------------------------------------------------------------------//



//-------------------------------------------------------------------------------------------------//
//----------------------------------------- WeatherStation PART -----------------------------------//
//-------------------------------------------------------------------------------------------------//
const float K = 273.15;  //To convert Kelvin to Celsiusra.
// Your API key
String openWeatherMapApiKey = "239b17a6199131b63a65a9b8b567563c";

// Your country code and city
String city = "Őcsény";
String countryCode = "HU";

// THE DEFAULT TIMER IS SET TO 10 SECONDS FOR TESTING PURPOSES
// For a final application, check the API call limits per hour/minute to avoid getting blocked/banned
//unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 10 seconds (10000)
//unsigned long timerDelay = 10000;
String jsonBuffer;
//-------------------------------------------------------------------------------------------------//





void setup() {
  Serial.begin(115200);
  //-------------------------------------------------------------------------------------------------//
  //----------------------------------------- RADIO and WIFI SETUP ----------------------------------//
  //-------------------------------------------------------------------------------------------------//
  // Wait for VS1053 and PAM8403 to power up
  // otherwise the system might not start up correctly
  delay(3000);

  // This can be set in the IDE no need for ext library
  //system_update_cpu_freq(160);

  Serial.println("\n\nmeeeaCH's Web Radio");
  reciver.enableIRIn();
  SPI.begin();
  player.begin();
  //This part doesn't work with my VS1003/VS1053, God knows what it is. :D
   /* if (player.getChipVersion() == 4) { // Only perform an update if we really are using a VS1053, not. eg. VS1003
        player.loadDefaultVs1053Patches(); 
    }*/
  player.loadDefaultVs1053Patches(); 
  player.switchToMp3Mode();
  player.setVolume(VOLUME);

  Serial.print("Connecting to SSID ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.print("connecting to ");
  Serial.println(host);

  if (!client.connect(host, httpPort)) {
      Serial.println("Connection failed");
      return;
  }

  Serial.print("Requesting stream: ");
  Serial.println(path);

  client.print(String("GET ") + path + " HTTP/1.1\r\n" +
                "Host: " + host + "\r\n" +
                "Connection: close\r\n\r\n");
  //-------------------------------------------------------------------------------------------------//
  //------------------------------------------DHT & OLED setup---------------------------------------//
  //-------------------------------------------------------------------------------------------------//
  //DHT11 setup.
  dht.begin();

  //Setting up display.
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {   /*I2C Address at which OLED is connected*/
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();
  display.setTextColor(WHITE);  /*Output text color white */ 
  //-------------------------------------------------------------------------------------------------//
}


//--------------------------------------------------------------------------------------------------------------------//
//------------------------------------------------------Functions-----------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//Show Menu.
void Menu(){
  display.clearDisplay();
  display.setTextSize(1); 
  display.setCursor(20,5);
  display.print("1 - Web Radio");
  display.setCursor(20,25);
  display.print("2 - Homerseklet");
  display.setCursor(20,45);
  display.print("3 - Homerseklet");
  display.display();
  //Serial.println("Menu Open");
  delay(1000);

//IR switch
  if (IrReceiver.decode())
    {
    irNum = String(IrReceiver.decodedIRData.command,HEX);
    if(irNum == "c"){
      irNumEq = 1;
    }
    if(irNum == "18"){
      irNumEq = 2;
    }
    if(irNum == "5e"){
      irNumEq = 3;
    }
		Serial.println(irNum);
    reciver.resume();
    }
}
//--------------------------------------------------------------------------------------------------------------------//

//--------------------------------------------------------------------------------------------------------------------//
//InsideTemp shown, data got from DHT11 sensor.
void InsideTemp(){
  float t = dht.readTemperature();  /*read temperature*/
  float h = dht.readHumidity();    /*read humidity*/  
  //float t = 20;  /*read temperature test*/
  //float h = 30;   /*read humidity test*/
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    display.clearDisplay();  /*clear OLED display before displaying reading*/
    display.setTextSize(1); /*OLED text font size*/
    display.setCursor(0,0);
    display.print("Failed to read from DHT sensor!");
    display.display();
    delay(3000);
    irNumEq = 4;
  }
  else{
    display.clearDisplay();  /*clear OLED display before displaying reading*/
    display.setTextSize(1); /*OLED text font size*/
    display.setCursor(0,0);
    display.print("Homerseklet: ");  // <------- Change this to your language.
    display.setTextSize(2);
    display.setCursor(0,10);
    display.print(t);       /*print temperature in Celsius*/
    display.print(" ");
    display.setTextSize(1);
    display.cp437(true);
    display.write(167);
    display.setTextSize(2);
    display.print("C");  

    display.setTextSize(1);
    display.setCursor(0, 35);
    display.print("Paratartalom: ");  // <------- Change this to your language.
    display.setTextSize(2);
    display.setCursor(0, 45);
    display.print(h);      /*prints humidity percentage*/
    display.print(" %");
    display.display();
  }
  delay(1000);
//IR switch
  if (IrReceiver.decode())
    {
    irNum = String(IrReceiver.decodedIRData.command,HEX);
    if(irNum == "c"){
      irNumEq = 1;
    }
    if(irNum == "5e"){
      irNumEq = 3;
    }
    if(irNum == "8"){
      irNumEq = 4;
    }
    reciver.resume();
    }
}
//--------------------------------------------------------------------------------------------------------------------//

//--------------------------------------------------------------------------------------------------------------------//
//OutsideTemp, data got from the internet. OpenWeatherMap API.
float convertStringtoFloat(String temp){
  float floatTemp = temp.toFloat();
  float cTemp = floatTemp - K;  //The value is in Kelvin, here it is converted to Celsius.
  return cTemp;
}

String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
    
  // Your Domain name with URL path or IP address with path
  http.begin(client, serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();
  return payload;
}

void OutsideTemp(){
  // Send an HTTP GET request
    // Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey;
      jsonBuffer = httpGETRequest(serverPath.c_str());
      Serial.println(jsonBuffer);
      JSONVar myObject = JSON.parse(jsonBuffer);
  
      // JSON.typeof(jsonVar) can be used to get the type of the var
      if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
      }

      String sTemp =  JSON.stringify(myObject["main"]["temp"]);

      display.clearDisplay();  /*clear OLED display before displaying reading*/
      display.setTextSize(1); /*OLED text font size*/
      display.setCursor(0,0);
      display.print("Homerseklet: ");   // <------- Change this to your language.
      display.setTextSize(2);
      display.setCursor(0,10);
      display.print(convertStringtoFloat(sTemp));       /*print temperature in Celsius*/
      display.print(" ");
      display.setTextSize(1);
      display.cp437(true);
      display.write(167);
      display.setTextSize(2);
      display.print("C");  

      display.setTextSize(1);
      display.setCursor(0, 35);
      display.print("Paratartalom: ");    // <------- Change this to your language.
      display.setTextSize(2);
      display.setCursor(0, 45);
      display.print(myObject["main"]["humidity"]);      /*prints humidity percentage*/
      display.print(" %");
      display.display();
    
    }
    else {
      Serial.println("WiFi Disconnected");
    } 
  
  //IR switch
  if (IrReceiver.decode())
    {
    irNum = String(IrReceiver.decodedIRData.command,HEX);
    if(irNum == "c"){
      irNumEq = 1;
    }
    if(irNum == "18"){
      irNumEq = 2;
    }
    if(irNum == "8"){
      irNumEq = 4;
    }
    reciver.resume();
    }
    delay(60000);  //10sec delay between reqests. It is advised to increase this number so you won't run out of API calls.
}

//--------------------------------------------------------------------------------------------------------------------//

//--------------------------------------------------------------------------------------------------------------------//
//Internet Radio
void Radio(){
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(10,5);
  display.print("Web Radio");
  display.setTextSize(1);
  display.setCursor(45,40);
  display.print("Radio 1"); 
  display.display();
  do{
   //Radio playing music. Sound data from website. 
   if (!client.connected()) {
        Serial.println("Reconnecting...");
        if (client.connect(host, httpPort)) {
            client.print(String("GET ") + path + " HTTP/1.1\r\n" +
                         "Host: " + host + "\r\n" +
                         "Connection: close\r\n\r\n");
        }
    }
    if (client.available() > 0) {
        // The buffer size 64 seems to be optimal. At 32 and 128 the sound might be brassy.
        uint8_t bytesread = client.read(mp3buff, 64);
        player.playChunk(mp3buff, bytesread);
    }

  if (IrReceiver.decode())
    {
    irNum = String(IrReceiver.decodedIRData.command,HEX);
    if(irNum == "18"){
      irNumEq = 2;
    }
    if(irNum == "5e"){
      irNumEq = 3;
    }
    if(irNum == "8"){
      irNumEq = 4;
    }
    reciver.resume();
    }
  }while(irNumEq == 1);  //This runs only the the radio part and do not refresh the screen. It can be removed.
}
//--------------------------------------------------------------------------------------------------------------------//



//--------------------------------------------------------------------------------------------------------------------//
//---------------------------------------------------------Loop-------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
void loop() {
  if (IrReceiver.decode())
    {
    irNum = String(IrReceiver.decodedIRData.command,HEX);
    if(irNum == "c"){
      irNumEq = 1;
    }
    if(irNum == "18"){
      irNumEq = 2;
    }
    if(irNum == "5e"){
      irNumEq = 3;
    }
    if(irNum == "8"){
      irNumEq = 4;
    }
    reciver.resume();
    }

  //Menu system, based on IR recivers data. It starts at the menu by default.
  switch(irNumEq) {
  case 1:
    Radio();
    break;
  case 2:
    InsideTemp();
    break;
  case 3:
    OutsideTemp();
    break;
  case 4:
    Menu();
    break;
  }
}