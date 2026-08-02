#include "options.h"
#include <ESPmDNS.h>
#include "time.h"
#include "rtcsupport.h"
#include "network.h"
#include "display.h"
#include "config.h"
#include "telnet.h"
#include "netserver.h"
#include "player.h"
#include "mqtt.h"
#include "timekeeper.h"
#include "../pluginsManager/pluginsManager.h"
#include <esp_wifi.h>

#ifndef WIFI_ATTEMPTS
  #define WIFI_ATTEMPTS  16
#endif

#ifndef SEARCH_WIFI_CORE_ID
  #define SEARCH_WIFI_CORE_ID  0
#endif
MyNetwork network;
TaskHandle_t wifiReconnectTaskHandle = nullptr;

bool beginStrongestAccessPoint(const char *ssid, const char *password) {
#if WIFI_LOCK_STRONGEST_BSSID
  const int16_t count = WiFi.scanNetworks(false, false, false, 300, 0, ssid);
  int16_t strongest = -1;
  int32_t strongestRssi = -1000;
  for (int16_t index = 0; index < count; ++index) {
    if (WiFi.SSID(index) == ssid && WiFi.RSSI(index) > strongestRssi) {
      strongest = index;
      strongestRssi = WiFi.RSSI(index);
    }
  }

  if (strongest >= 0) {
    uint8_t bssid[6];
    memcpy(bssid, WiFi.BSSID(strongest), sizeof(bssid));
    const int32_t channel = WiFi.channel(strongest);
    Serial.printf("Locking WiFi to strongest Nest point %02X:%02X:%02X:%02X:%02X:%02X, channel %d, RSSI %d dBm\n",
                  bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5],
                  channel, strongestRssi);
    WiFi.scanDelete();

    // Configure the selected AP without connecting yet, then turn off the
    // roaming features that can bounce a stationary ESP32 between mesh nodes.
    WiFi.begin(ssid, password, channel, bssid, false);
    wifi_config_t stationConfig;
    if (esp_wifi_get_config(WIFI_IF_STA, &stationConfig) == ESP_OK) {
      stationConfig.sta.rm_enabled = 0;
      stationConfig.sta.btm_enabled = 0;
      stationConfig.sta.mbo_enabled = 0;
      esp_wifi_set_config(WIFI_IF_STA, &stationConfig);
    }
    esp_wifi_connect();
    return true;
  }
  WiFi.scanDelete();
#endif
  WiFi.begin(ssid, password);
  return false;
}

void retryWiFiConnection(void *parameter) {
  (void)parameter;
  while (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
    vTaskDelay(pdMS_TO_TICKS(WIFI_RECONNECT_INTERVAL_MS));
  }
  wifiReconnectTaskHandle = nullptr;
  vTaskDelete(nullptr);
}

void MyNetwork::WiFiReconnected(WiFiEvent_t event, WiFiEventInfo_t info){
  network.beginReconnect = false;
  player.lockOutput = false;
  delay(100);
  display.putRequest(NEWMODE, PLAYER);
  if(config.getMode()==PM_SDCARD) {
    network.status=CONNECTED;
    display.putRequest(NEWIP, 0);
  }else{
    display.putRequest(NEWMODE, PLAYER);
    if (network.lostPlaying) {
      network.lostPlaying = false;
      player.sendCommand({PR_PLAY, config.lastStation()});
    }
  }
  #ifdef MQTT_ROOT_TOPIC
    connectToMqtt();
  #endif
}

void MyNetwork::WiFiLostConnection(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.printf("WiFi disconnected, reason %u\n", info.wifi_sta_disconnected.reason);
  if(!network.beginReconnect){
    Serial.printf("Lost connection, reconnecting to %s...\n", config.ssids[config.store.lastSSID-1].ssid);
    if(config.getMode()==PM_SDCARD) {
      network.status=SDREADY;
      display.putRequest(NEWIP, 0);
    }else{
      network.lostPlaying = player.isRunning();
      if (network.lostPlaying) { player.lockOutput = true; player.sendCommand({PR_STOP, 0}); }
      display.putRequest(NEWMODE, LOST);
    }
  }
  network.beginReconnect = true;
  if (wifiReconnectTaskHandle == nullptr) {
    xTaskCreatePinnedToCore(retryWiFiConnection, "wifiRetry", 3072, nullptr, 1,
                            &wifiReconnectTaskHandle, SEARCH_WIFI_CORE_ID);
  }
}

bool MyNetwork::wifiBegin(bool silent){
  uint8_t ls = (config.store.lastSSID == 0 || config.store.lastSSID > config.ssidsCount) ? 0 : config.store.lastSSID - 1;
  uint8_t errcnt = 0;
  uint16_t failedNetworks = 0;
  //WiFi.mode(WIFI_STA);
  while (true) {
    if(!silent){
      Serial.printf("##[BOOT]#\tAttempt to connect to %s\n", config.ssids[ls].ssid);
      Serial.print("##[BOOT]#\t");
      display.putRequest(BOOTSTRING, ls);
    }
    //WiFi.disconnect(true, true); //disconnect & erase internal credentials https://github.com/e2002/yoradio/pull/164/commits/89d8b4450dde99cd7930b84bb14d81dab920b879
    //delay(100);
    WiFi.mode(WIFI_STA);
    #ifdef WIFI_TX_POWER
      WiFi.setTxPower(WIFI_TX_POWER);
    #endif
    beginStrongestAccessPoint(config.ssids[ls].ssid, config.ssids[ls].password);
    while (WiFi.status() != WL_CONNECTED) {
      if(!silent) Serial.print(".");
      delay(500);
      if(REAL_LEDBUILTIN!=255 && !silent) digitalWrite(REAL_LEDBUILTIN, !digitalRead(REAL_LEDBUILTIN));
      errcnt++;
      if (errcnt > WIFI_ATTEMPTS) {
        errcnt = 0;
        ls++;
        if (ls > config.ssidsCount - 1) ls = 0;
        failedNetworks++;
        if(!silent) Serial.println();
        WiFi.mode(WIFI_OFF);
        break;
      }
    }
    if (WiFi.status() != WL_CONNECTED &&
        failedNetworks >= static_cast<uint16_t>(config.ssidsCount) * WIFI_STARTUP_CYCLES) {
      return false; break;
    }
    if (WiFi.status() == WL_CONNECTED) {
      config.setLastSSID(ls + 1);
      return true; break;
    }
  }
  return false;
}

void searchWiFi(void * pvParameters){
  if(!network.wifiBegin(true)){
    delay(10000);
    xTaskCreatePinnedToCore(searchWiFi, "searchWiFi", 1024 * 4, NULL, 0, NULL, SEARCH_WIFI_CORE_ID);
  }else{
    network.status = CONNECTED;
    netserver.begin(true);
    telnet.begin(true);
    network.setWifiParams();
    display.putRequest(NEWIP, 0);
    #ifdef MQTT_ROOT_TOPIC
      mqttInit();
    #endif
  }
  vTaskDelete( NULL );
}

#define DBGAP false

void MyNetwork::begin() {
  BOOTLOG("network.begin");
  config.initNetwork();
  if (config.ssidsCount == 0 || DBGAP) {
    raiseSoftAP();
    return;
  }
  if(config.getMode()!=PM_SDCARD){
    if(!wifiBegin()){
      raiseSoftAP();
      Serial.println("##[BOOT]#\tdone");
      return;
    }
    Serial.println(".");
    status = CONNECTED;
    setWifiParams();
    #ifdef MQTT_ROOT_TOPIC
      mqttInit();
    #endif
  }else{
    status = SDREADY;
    xTaskCreatePinnedToCore(searchWiFi, "searchWiFi", 1024 * 4, NULL, 0, NULL, SEARCH_WIFI_CORE_ID);
  }
  
  Serial.println("##[BOOT]#\tdone");
  if(REAL_LEDBUILTIN!=255) digitalWrite(REAL_LEDBUILTIN, LOW);
  
#if RTCSUPPORTED
  if(config.isRTCFound()){
    rtc.getTime(&network.timeinfo);
    mktime(&network.timeinfo);
    display.putRequest(CLOCK);
  }
#endif
  if (network_on_connect) network_on_connect();
  pm.on_connect();
}

void MyNetwork::setWifiParams(){
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);
  #ifdef WIFI_TX_POWER
    WiFi.setTxPower(WIFI_TX_POWER);
  #endif
  WiFi.onEvent(WiFiReconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(WiFiLostConnection, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  //config.setTimeConf(); //??
  if(strlen(config.store.mdnsname)>0)
    MDNS.begin(config.store.mdnsname);
}

void MyNetwork::requestTimeSync(bool withTelnetOutput, uint8_t clientId) {
  if (withTelnetOutput) {
    char timeStringBuff[50];
    strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%dT%H:%M:%S", &timeinfo);
    if (config.store.tzHour < 0) {
      telnet.printf(clientId, "##SYS.DATE#: %s%03d:%02d\n> ", timeStringBuff, config.store.tzHour, config.store.tzMin);
    } else {
      telnet.printf(clientId, "##SYS.DATE#: %s+%02d:%02d\n> ", timeStringBuff, config.store.tzHour, config.store.tzMin);
    }
  }
}

void rebootTime() {
  ESP.restart();
}

void MyNetwork::raiseSoftAP() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(apSsid, apPassword);
  Serial.println("##[BOOT]#");
  BOOTLOG("************************************************");
  BOOTLOG("Running in AP mode");
  BOOTLOG("Connect to AP %s with password %s", apSsid, apPassword);
  BOOTLOG("and go to http:/192.168.4.1/ to configure");
  BOOTLOG("************************************************");
  status = SOFT_AP;
  if(config.store.softapdelay>0)
    timekeeper.waitAndDo(config.store.softapdelay*60, rebootTime);
}

void MyNetwork::requestWeatherSync(){
  display.putRequest(NEWWEATHER);
}
