/* TODO
 * Study Arduino ide debug 
 * When mqtt is not connected first controll wifi connection then mqtt connection
 * ESP connects to the strongest known wifi
 */


//DHT22
#include <DHT.h>
#define DHTPIN 14     // DHT Pin Gpio14 (D5)
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE);


//BMP280
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
 
//SCK on D1 & SDI on D2

#define BMP_SCK 13 
#define BMP_MISO 12
#define BMP_MOSI 11
#define BMP_CS 10
 
Adafruit_BMP280 bmp; // I2C


//ESP8266
#include <ESP8266WiFi.h> //https://github.com/esp8266/Arduino/tree/master/doc/esp8266wifi
#include <PubSubClient.h> //Api guide https://pubsubclient.knolleary.net/api.html


//WiFi
#define wifi_ssid "Vodafone - Sant'Artemio 16/C"
#define wifi_password "Magicoale_1234"
//#define wifi_ssid "apsa16c"
//#define wifi_password "Magicoale_1234"


//MQTT
#define mqtt_server "192.168.1.228"
#define mqtt_user "iot"      // if exist
#define mqtt_password "IOT2018"  //idem
#define mqtt_clientId "cameraAle"
#define logTopic "iot/log/rooms/cameraAle"
#define mqtt_willMessage "Camera Ale wrongly disconnected from MQTT Broker"
#define boardTopic "iot/rooms/cameraAle"
//#define boardTopic "testing"




//Buffer to decode MQTT messages
char message_buff[100];


//MQTT variables
long lastMsg = 0;   
long lastRecu = 0;
bool debug = false;  //Display log message if True


// Create objects   
WiFiClient espClient;
PubSubClient client(espClient);


//Sensor variables
float tempBMP;
float tempDHT;
float hum;
float pres;
String msg;    


//Time variables
unsigned long currentMillis;
unsigned long lastUpdateTemp;
unsigned long lastUpdatePress;
unsigned long lastUpdateHum;


//Refresh time variables in millis (1 min = 60000 millis)
unsigned long refreshRateTemp;
unsigned long refreshRatePress;
unsigned long refreshRateHum;


void setup() {
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  
    //Variable initialization
    msg = ""; 
    //refreshRateTemp = 30000;
    //refreshRatePress = 30000;
    //refreshRateHum = 30000;
    refreshRateTemp = 300000;
    refreshRatePress = 1800000;
    refreshRateHum = 900000;
    lastUpdateTemp = 0;
    lastUpdatePress = 0;
    lastUpdateHum = 0;

    //Sensor initialization
    dht.begin();
    
    if (!bmp.begin()) { 
        Serial.println("Could not find a valid BMP280 sensor, check wiring!");
        digitalWrite(LED_BUILTIN, HIGH); 
        while (1);
    }
     
    digitalWrite(LED_BUILTIN, HIGH); 
    setup_wifi();           //Connect to Wifi network
    client.setServer(mqtt_server, 1883);    // Configure MQTT connexion
    client.setCallback(callback);           // callback function to execute when a MQTT message   
}


//Connection to the wifi network
void setup_wifi() {
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(wifi_ssid);
    
    WiFi.begin(wifi_ssid, wifi_password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi OK ");
    Serial.print("=> ESP8266 IP address: ");
    Serial.println(WiFi.localIP());
}


//Reconnect to MQTT Broker
void reconnect() {
    Serial.println("Reconnecting to mqtt");
    while (!client.connected()) {
        Serial.print("Connecting to MQTT broker ...");
        if (client.connect(mqtt_clientId, mqtt_user, mqtt_password, logTopic, 1, true, mqtt_willMessage)) {
            Serial.println("OK");
            client.publish(logTopic, "Connected to MQTT Broker!" , true);
        } else {
            Serial.print("KO, error : ");
            Serial.print(client.state());
            Serial.println(" Wait 5 secondes before to retry");
            delay(5000);
        }
    }
}


void loop() {
    
    if (!client.loop()){ //This should be called regularly to allow the client to process incoming messages and maintain its connection to the server.
        reconnect();
    }

    
    currentMillis = millis();
    if ((unsigned long)(currentMillis-lastUpdateTemp)>=refreshRateTemp){
        tempBMP = bmp.readTemperature();
        tempDHT = dht.readTemperature();
        String tmp;
        if(!isnan(tempDHT)){
            tmp = "Temperature:" + String(tempDHT) + ";";
        } else if(tempBMP>-100){
            client.publish(logTopic, "DHT22 TEMPERATURE SENSOR NOT WORKING" , true);
            tmp = "Temperature:" + String(tempBMP) + ";";
        } else{
            tmp = "Temperature:NULL;";
            client.publish(logTopic, "DHT22 AND BMP280 TEMPERATURE NOT WORKING" , true);
        }
        msg+=tmp;
        lastUpdateTemp=currentMillis;
    }
    if ((unsigned long)(currentMillis-lastUpdateHum)>=refreshRateHum){
        hum = dht.readHumidity();
        String tmp;
        if(!isnan(hum)){
            tmp = "Humidity:" + String(hum) + ";";
        } else {
          tmp = "Humidity:NULL;";
          client.publish(logTopic, "DHT22 HUMIDITY NOT WORKING" , true);
        }
        msg+=tmp;
        lastUpdateHum = currentMillis;
    }
    if ((unsigned long)(currentMillis-lastUpdatePress)>=refreshRatePress){
        pres = bmp.readPressure();
        String tmp;
        if(pres<127363){
            tmp = "Pressure:" + String(pres/100) + ";";
        } else {
            tmp = "Pressure:NULL;";
            client.publish(logTopic, "BMP280 PRESSURE NOT WORKING" , true);
        }
        msg+=tmp;
        lastUpdatePress = currentMillis;
    }
    if (msg!=""){
        char messg[msg.length()];
        strcpy(messg, msg.c_str());
        client.publish(boardTopic, messg , true);
        Serial.print("Published: ");
        Serial.println(msg);
        msg="";
        Serial.print("Is Connected to the mqtt: ");
        Serial.println(client.connected());
    }
    delay(500);
    
}


// MQTT callback function
// example http://m2mio.tumblr.com/post/30048662088/a-simple-example-arduino-mqtt-m2mio
void callback(char* topic, byte* payload, unsigned int length) {

    int i = 0;
    if ( debug ) {
        Serial.println("Message recu =>  topic: " + String(topic));
        Serial.print(" | longueur: " + String(length,DEC));
    }
    // create character buffer with ending null terminator (string)
    for(i=0; i<length; i++) {
        message_buff[i] = payload[i];
    }
    message_buff[i] = '\0';
    
    String msgString = String(message_buff);
    if ( debug ) {
        Serial.println("Payload: " + msgString);
    }
    
    if ( msgString == "ON" ) {
        digitalWrite(D2,HIGH);  
    } else {
        digitalWrite(D2,LOW);  
    }
}
