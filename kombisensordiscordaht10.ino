extern "C" {
#include "user_interface.h"
}
#include <Adafruit_AHT10.h>
#include <U8g2lib.h>

#define kanal 1569039//<<<ts kanal no 
#define writeAPIkey "WB69WJZ5IZIJI6XH"
#define settingsapikey "CKWJUMQSDMNADU1W"
#define settingskanal 1569043
#define fieldno 1 // <<<ts kanaldaki komut alanı numarası
#define versiyon 2
#include <ThingSpeak.h>


int tsstatus;

float sicaklik = 0;
float sicaklik2 = 0;
double sicaklikduzelt = 0.35;
float humb = 0;
bool humerror = false;

uint8_t relay = D5;

#include <ESP8266WiFi.h>
#include <Wire.h>
#include <ESP8266WiFiMulti.h>
WiFiClient  client;
int bat = 0;
int batcounter = 0;
float oldhedefsicaklik;
Adafruit_AHT10 aht;
float hedefsicaklik;
long hedefsicaklikstatus;
String sFullip = "";
ESP8266WiFiMulti WiFiMulti;


void setup() {


  Serial.begin(74880, SERIAL_8N1, SERIAL_TX_ONLY);

  delay(100);
  wifi_station_set_hostname("ESP KombiSens");
  WiFi.mode(WIFI_OFF);
  delay(100);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Wire.begin();
  while (! aht.begin()) {
    Serial.println("Could not find AHT10? Check wiring");
    digitalWrite(LED_BUILTIN, LOW);
    delay(5000);
  }
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("AHT10 found");
  pinMode(relay, OUTPUT);
  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);
  WiFiMulti.addAP( "zzzzzzzzz", "yyyyyyyyyyyyyy");//<<<<wifi bilgileriniz
  //WiFiMulti.addAP("zzzzz", "zzzzzz");//>>>> eğer varsa menzildeki ikinci bir wifi nin bilgileri satırın önündeki // yi kaldırın

  Serial.println("wifi");
  int conc = 15;
  while (WiFiMulti.run() != WL_CONNECTED && conc > 0)
  {
    delay(900);
    conc--;
    Serial.println(String(conc));
    digitalWrite(LED_BUILTIN, LOW);
    delay(20);
    digitalWrite(LED_BUILTIN, HIGH);
  }
  if ((WiFiMulti.run() != WL_CONNECTED)) {
    digitalWrite(LED_BUILTIN, LOW); //led sinyali ver
    delay(750);
    digitalWrite(LED_BUILTIN, HIGH);
    ESP.restart();  //internet yok ise esp restart edilecek.
  }
  int i = 0;
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  digitalWrite(LED_BUILTIN, LOW);
  delay(40);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(40);
  digitalWrite(LED_BUILTIN, HIGH);
  IPAddress myip = WiFi.localIP();
  sFullip = String(myip[0]) + "." + myip[1] + "." + myip[2] + "." + myip[3];
  Serial.println(sFullip);
  Serial.println(WiFi.hostname());
  delay(1000);
}
//ht tps://api.thingspeak.com/update?api_key=sdfsfsdfsdf&field1=0 komut sistemi örnek yazma kullanıcıdan

int laststate = -1;
sensors_event_t humidity, temp;
void loop() {
  hedefsicaklik = ThingSpeak.readFloatField(settingskanal, 1, settingsapikey);
  hedefsicaklikstatus = ThingSpeak.getLastReadStatus();
  Serial.println("Hedef sicaklik okuma durum:" + String(hedefsicaklikstatus) + " Hedef sicaklik:" + String(hedefsicaklik));
  if (hedefsicaklikstatus != 200 ) hedefsicaklik = oldhedefsicaklik;
  else {
    oldhedefsicaklik = hedefsicaklik;
  }
oku: aht.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
  Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" degrees C");
  Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% rH");
  sicaklik = temp.temperature;
  humerror = true;
  if (humidity.relative_humidity != 0) {
    humb = humidity.relative_humidity;
    humerror = false;
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH);
  }
  if (humidity.relative_humidity > 99 || humidity.relative_humidity < 2 ) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(1500);
    digitalWrite(LED_BUILTIN, HIGH);
    goto oku;
  }
  sicaklik = sicaklik + sicaklikduzelt;
 
  Serial.println(String(sicaklik, 2) + " derece " + (humerror ? "!" : "") + String(humb, 2) + " %nem " );
  yazulan();
  if (tsstatus == -401) {
    delay(15000);
    yazulan();
  }
  Serial.println("kanal guncelleme op sonucu:" + String(tsstatus));
  if (tsstatus != 200) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
    digitalWrite(LED_BUILTIN, HIGH);
  }
  delay(14700);
  digitalWrite(LED_BUILTIN, LOW);
  delay(20);
  digitalWrite(LED_BUILTIN, HIGH);
}
void yazulan()
{
  ThingSpeak.setField( 1, sicaklik);
  ThingSpeak.setField( 2, humb);
  ThingSpeak.setField( 3, WiFi.RSSI());

  ThingSpeak.setField( 4, hedefsicaklik);
  ThingSpeak.setStatus("V" + String(versiyon) + " WiFi:" + String(WiFi.SSID()) + " IP:" + sFullip + " HS durum:" + String(hedefsicaklikstatus) +  " Hedef:" + String(hedefsicaklik, 2) );
  tsstatus = ThingSpeak.writeFields(kanal, writeAPIkey);
}
