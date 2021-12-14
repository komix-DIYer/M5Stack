/*
 * Wi-Fi AP for Tello Edu
 */
 
#include <M5Stack.h>
#include <WiFi.h>
#include "esp_wifi.h"

const char *ssid = "TelloEdu";
const char *password = "TelloEdu";

void setup()
{
  M5.begin();
  M5.update();
  
  WiFi.softAPConfig(IPAddress(192, 168,   4, 1), // IP
                    IPAddress(192, 168,   4, 1), // GW
                    IPAddress(255, 255, 255, 0)); // NM
  
  WiFi.softAP(ssid, password);
  
  M5.Lcd.clear(BLACK); // 320x240
  M5.Lcd.setTextColor(WHITE,BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0,  0);
  M5.Lcd.println("AP mode");
  M5.Lcd.println();
  M5.Lcd.print("SSID      : ");  M5.Lcd.println(ssid); // String WiFi.SSID()
  M5.Lcd.print("IP address: ");  M5.Lcd.println(WiFi.softAPIP()); // IPAddress
  M5.Lcd.drawLine( 0, 70, 320, 70, WHITE);
}

void loop()
{
  wifi_sta_list_t wifi_sta_list; // List of devices that connected to the softAP (without IP address info.)
  tcpip_adapter_sta_list_t adapter_sta_list; // Same list (with IP address info.)
 
  memset(&wifi_sta_list, 0, sizeof(wifi_sta_list));
  memset(&adapter_sta_list, 0, sizeof(adapter_sta_list));
 
  esp_wifi_ap_get_sta_list(&wifi_sta_list);
  tcpip_adapter_get_sta_list(&wifi_sta_list, &adapter_sta_list);
  
  M5.Lcd.setCursor(0, 100);
  M5.Lcd.setTextSize(2);
//  M5.Lcd.print(WiFi.softAPgetStationNum()); /* uint8_t */  M5.Lcd.println(" clients are connected.");
  M5.Lcd.print(adapter_sta_list.num);  M5.Lcd.println(" clients are connected.");
  M5.Lcd.println("");  
  M5.Lcd.fillRect(0, 120, 320, 120, BLACK);

  for (int i = 0; i < adapter_sta_list.num; i++)
  {
    tcpip_adapter_sta_info_t station = adapter_sta_list.sta[i];
// 
//    Serial.print("station nr ");
//    Serial.println(i);
// 
//    Serial.print("MAC: ");
// 
//    for(int i = 0; i< 6; i++)
//    {
//      Serial.printf("%02X", station.mac[i]);  
//      if(i<5)Serial.print(":");
//    }
//    
//    Serial.print("\nIP: ");
    M5.Lcd.print(i + 1);
    M5.Lcd.print(": ");
    M5.Lcd.println(ip4addr_ntoa(&(station.ip)));
  }
  
  delay(2000); // update every 2 [s]
}
