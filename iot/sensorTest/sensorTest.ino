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


//Sensor variables
float tempBMP;
float tempDHT;
float hum;
float pres;
String msg;   
const float bmpTempOffset = 1.92;

void setup() {
    Serial.begin(115200);

    //Sensor initialization
    dht.begin();
    
    if (!bmp.begin()) { 
        Serial.println("Could not find a valid BMP280 sensor, check wiring!");
        digitalWrite(LED_BUILTIN, HIGH); 
        while (1);
    }
}


void loop() {

    tempBMP = bmp.readTemperature()-bmpTempOffset;
    tempDHT = dht.readTemperature();
    pres = bmp.readPressure();

    Serial.print("BMP Temperature: ");
    if (v){
        Serial.println(tempBMP);
    } else Serial.println("NULL");
    Serial.print("BMP Pressure: ");
    if (pres<127363){
        Serial.println(pres);
    } else Serial.println("NULL");
    Serial.print("DHT Temperature: ");
    if (!isnan(tempDHT)){
        Serial.println(tempDHT);
    } else Serial.println("NULL");
    
    Serial.print("Temperature offset: ");
    Serial.println(tempBMP-tempDHT);    

    Serial.println();

    delay(1000);

}
