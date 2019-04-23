// This is the code to subscribe to the Treasure/Fluke topic. The code parses through the JSON
// message from MQTT and prints as a string.

#include "config.h" // edit the config.h tab and enter your credentials

// Required libraries for code to work
#include "Wire.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <PubSubClient.h>   
#include <ArduinoJson.h>    
#include <ESP8266WiFi.h> 

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1  // Reset pin # (or -1 if sharing Arduino reset pin)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); 

WiFiClient espClient;            
PubSubClient mqtt(espClient);     

char mac[6]; //A MAC address is a 'truly' unique ID for each device, lets use that as our 'truly' unique user ID!!!

char message[201]; //201, as last character in the array is the NULL character, denoting the end of the array

void setup() {

  Serial.begin(115200); // start the serial connection
  
  // Prints the results to the serial monitor
  Serial.print("This board is running: ");  //Prints that the board is running
  Serial.println(F(__FILE__));
  Serial.print("Compiled: "); //Prints that the program was compiled on this date and time
  Serial.println(F(__DATE__ " " __TIME__));

  while(! Serial); // wait for serial monitor to open

  setup_wifi();
  mqtt.setServer(mqtt_server, 1883);
  mqtt.setCallback(callback); //register the callback function

  //setup for the display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //initailize with the i2c addre 0x3C
  display.clearDisplay();                    //Clears any existing images or text
  display.setTextSize(1);                    //Set text size
  display.setTextColor(WHITE);               //Set text color to white
  display.setCursor(0,0);                    //Puts cursor on top left corner
  display.println("Starting up...");         //Test and write up
  display.display();                         //Displaying the display
}

/////SETUP_WIFI/////
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); // wait 5 ms
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");  //get the unique MAC address to use as MQTT client ID, a 'truly' unique ID.
  Serial.println(WiFi.macAddress());  //.macAddress returns a byte array 6 bytes representing the MAC address
}                                     

/////CONNECT/RECONNECT/////Monitor the connection to MQTT server, if down, reconnect
void reconnect() {
  // Loop until we're reconnected
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqtt.connect(mac, mqtt_user, mqtt_password)) { //<<---using MAC as client ID, always unique!!!
      Serial.println("connected");
      mqtt.subscribe("Treasure/+"); //we are subscribing to 'Treasure' and all subtopics below that topic
    } else {                       
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      delay(5000); // Wait 5 seconds before retrying
    }
  }
}

void loop() {

  if (!mqtt.connected()) {
    reconnect();
  }

  mqtt.loop(); //this keeps the mqtt connection 'active'
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println();
  Serial.print("Message arrived [");
  Serial.print(topic); //'topic' refers to the incoming topic name, the 1st argument of the callback function
  Serial.println("] ");

  DynamicJsonBuffer  jsonBuffer; 
  JsonObject& root = jsonBuffer.parseObject(payload); //parse it!

  if (!root.success()) { 
    Serial.println("parseObject() failed, are you sure this message is JSON formatted.");
    return;
  }

  display.clearDisplay(); //Clears any existing images or text
  display.setCursor(0,0); //Puts cursor on top left corner

  double temp = root["Temperature (F)"]; // reads the temperature JSON key and holds the value
  double pres = root["Pressure"]; // reads the pressure JSON key and holds the value
  double hum = root["Humidity"]; // reads the humidity JSON key and holds the value

  // Prints statement to display
  display.print("Temp is ");
  display.print(temp);
  display.println(" F");

  display.print("Pressure is ");
  display.print(pres);
  display.println("kPA");

  display.print("Humidity is ");
  display.print(hum);
  display.println(" %");

  display.display(); //Displaying the display

  Serial.print("Temperature is ");
  Serial.print(temp);
  Serial.println(" F");

  Serial.print("Pressure is ");
  Serial.print(pres);
  Serial.println(" kPA");

  Serial.print("Humidity is ");
  Serial.print(hum);
  Serial.println(" %");
}