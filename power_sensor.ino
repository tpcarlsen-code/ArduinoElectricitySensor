#include <BH1750FVI.h>
#include <MemoryFree.h>
#include <WiFiNINA.h>
#include "metrics.h"
#include "settings.h"

#define LIGHT_THRESHOLD 4 // Number of lux to consider the led flashing.
#define PUSH_INTERVAL_MINUTES 5 // number of minutes between Prometheus push.

BH1750FVI sensor(BH1750FVI::k_DevModeContHighRes);
uint16_t lux;
uint16_t blinks = 0;

unsigned long now = 0;
unsigned long lastPush = 0;

bool on = false;

PrometheusMetricPush prom(prometheusGateway, 9091, "powermeter_sensor");

void setup()
{
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  sensor.begin();
  connectWiFi();
}

void loop()
{
  lux = sensor.GetLightIntensity();
  //Serial.print("LUX: ");
  //Serial.println(lux);
  if (lux >= LIGHT_THRESHOLD && !on)
  {
    on = true;
    blinks++;
    //Serial.println("********************************************************************************");
    //Serial.print("BLINKS: ");
    //Serial.println(blinks);
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else if (lux < LIGHT_THRESHOLD)
  {
    on = false;
    digitalWrite(LED_BUILTIN, LOW);
  }

  if (millis() - lastPush >= PUSH_INTERVAL_MINUTES * 60000)
  {
    pushMetrics();
    blinks = 0;
    lastPush = millis();
    return;
  }
  delay(5);
}

void connectWiFi()
{
  int status = WiFi.status();
  if (status != WL_CONNECTED)
  {
    Serial.print("WiFI not connected: ");
    Serial.println(status);
    // wiFiDrv.wifiDriverDeinit();
    // wiFiDrv.wifiDriverInit();
    WiFi.config(local, dns, gw, subnet);
    status = WiFi.begin(ssid, pass);
    while (status != WL_CONNECTED)
    {
      Serial.print(F("Could not WiFi connect: "));
      Serial.println(WiFi.reasonCode());
      status = WiFi.begin(ssid, pass);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(250);
      digitalWrite(LED_BUILTIN, LOW);
      delay(250);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(250);
      digitalWrite(LED_BUILTIN, LOW);
      delay(1000);
    }
  }
  //WiFi.noLowPowerMode();
}

void pushMetrics()
{
  Serial.println(F("Starting metrics push..."));
  // Make sure we have WiFi connection.
  connectWiFi();

  // 1 blink = 1 Wh
  int watts = blinks * (60 / PUSH_INTERVAL_MINUTES);
  //Serial.print("Watts: ");
  //Serial.println(watts);

  Metrics metrics;
  metrics.add("power_watts", watts);
  metrics.add("memory_free_bytes", freeMemory());
  if (!prom.push(metrics))
  {
    Serial.println(F("Failed!"));
    return;
  }
  Serial.println(F("Done"));
}