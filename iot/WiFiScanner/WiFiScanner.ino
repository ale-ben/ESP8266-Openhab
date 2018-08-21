#include <ESP8266WiFi.h> //https://github.com/esp8266/Arduino/tree/master/doc/esp8266wifi

//WiFi
#define nOfKnownWifi 2
const String wifi_ssid[]={"Vodafone - Sant'Artemio 16/C","apsa16c"};
const String wifi_password[]={"Magicoale_1234","magicoale"};

int nOfNetworks;

void prinScanResult(int networksFound)
{
    nOfNetworks = networksFound;
    Serial.printf("%d network(s) found\n", networksFound);
    for (int i = 0; i < networksFound; i++)
    {
        Serial.printf("%d: %s, Ch:%d (%ddBm) %s\n", i + 1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.encryptionType(i) == ENC_TYPE_NONE ? "open" : "");
    }
}

int getWifiId(){
    int netStrength[nOfNetworks];
    bool ordered = false;
    for (int i = 0; i<nOfNetworks; i++){
        netStrength[i]=i;
    }
    while (!ordered){
        ordered = true;
        for (int i = 0; i<nOfNetworks-1; i++){
            if (WiFi.RSSI(netStrength[i])<WiFi.RSSI(netStrength[i+1])){
                int tmp = netStrength[i];
                netStrength[i] = netStrength[i+1];
                netStrength[i+1] = tmp;
                ordered = false;
            }
        }
    }
    for (int i = 0; i<nOfNetworks-1; i++){
        for (int j = 0; j<nOfKnownWifi;j++){
            if(wifi_ssid[j]==WiFi.SSID(i).c_str()) return j;
        }
    }
    return -1;
}

void setup(){
    Serial.begin(115200);
    Serial.println();

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    WiFi.scanNetworksAsync(prinScanResult);

    Serial.println();
    int netId = getWifiId();
    if (netId!=-1){
        Serial.println("Strongest known wifi:" + wifi_ssid[netId]);
    } else Serial.println("No known wifi in range");

}

void loop(){
    
}
