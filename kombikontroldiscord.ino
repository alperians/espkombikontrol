extern "C" {
#include "user_interface.h"
}
#define settingskanal 1
#define fieldno 1 // <<<ts kanaldaki komut alanı numarası
#define settingsapikey "xxxxxxxxxxxxxxxx"  // <<<ts kanaldaki komut alanı numarası

long sensorkanal = 0;
String sensorapikey = "XXXXXXXXXXXXX"; // <<<ts kanaldaki komut alanı numarası

//#include <strings.h>
#include <ThingSpeak.h>
#define versiyon 2
int tsstatus;
float hedefsicaklik;
float hassasiyet = 0.2;
float oldhassasiyet = 0;
float mevcutsicaklik;
float tempmevcutsicaklik;
long hedefsicaklikstatus;
float oldhedefsicaklik;
String sensorsonguncelleme = "";
uint8_t relay = D5;

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
WiFiClient  client;

ESP8266WiFiMulti WiFiMulti;
void setup() {
  pinMode(relay, OUTPUT);
  digitalWrite(relay, HIGH);
  // put your setup code here, to run once:
  Serial.begin(74880, SERIAL_8N1, SERIAL_TX_ONLY);
  delay(100);
  WiFi.mode(WIFI_OFF);
  delay(100);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // WiFi.softAP(apssid, appassword);
  WiFi.mode(WIFI_STA);
  wifi_station_set_hostname("ESP Kombi Kontrol");   //<<<<wifi network nde gözükmesi istenen kanal no
  ThingSpeak.begin(client);
  WiFiMulti.addAP("SERVIS", "XXXXXXXXXXX");   //<<<<<<<sistemin test edileceği yerdeki Wifi bilgileri
  WiFiMulti.addAP("EV", "XXXXXXXXXXXXXXX");     //<<<<Sistemin kullanılacağı yerdeki wifi bilgileri

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
  String sFullip = String(myip[0]) + "." + myip[1] + "." + myip[2] + "." + myip[3];
  Serial.println(sFullip);
  Serial.println(WiFi.hostname());
  Serial.println(" WiFi:" + String(WiFi.SSID()));
  delay(1000);
}
bool acik = false;
bool startup = true;
void loop() {
  digitalWrite(LED_BUILTIN, LOW);
  delay(40);
  digitalWrite(LED_BUILTIN, HIGH);
  sensorsonguncelleme = ThingSpeak.readRaw(sensorkanal, String(String("/fields/") + String(1) + String("/last_data_age")), sensorapikey);
  if (sensorsonguncelleme.toInt() < 1800)
  {
    tempmevcutsicaklik = ThingSpeak.readFloatField(sensorkanal, 1, sensorapikey);
    tsstatus = ThingSpeak.getLastReadStatus();
    if (tsstatus == 200) {
      mevcutsicaklik = tempmevcutsicaklik;
    }
    else
    {
      Serial.println("Mevcut Sicaklik okuma islemi sorunlu " + String(tsstatus));
      blinker(2);
      delay(500);
      blinker(3);
    }
  }
  else
  {
    Serial.println("Sensor mevcut sicaklik bilgisi eski okuma yapilmadi");
  }
  hassasiyet = ThingSpeak.readFloatField(settingskanal, 3, settingsapikey);
  tsstatus = ThingSpeak.getLastReadStatus();
  if (tsstatus != 200)
  {
    hassasiyet = oldhassasiyet;
    blinker(4);
    delay(500);
    Serial.println("Hassasiyet bilgisi getirme isleminde hata " + String(tsstatus));
  }
  else
  {
    oldhassasiyet = hassasiyet;
  }
  hedefsicaklik = ThingSpeak.readFloatField(settingskanal, 1, settingsapikey);
  hedefsicaklikstatus = ThingSpeak.getLastReadStatus();
  Serial.println("Hedef sicaklik okuma durum:" + String(hedefsicaklikstatus) + " Hedef sicaklik:" + String(hedefsicaklik) + " Hassasiyet;" + String(hassasiyet, 2));
  if (hedefsicaklikstatus != 200 ) {
    hedefsicaklik = oldhedefsicaklik;
    blinker(2);
    Serial.println("Hedef sicaklik i okuma isleminde hata " + String(hedefsicaklikstatus ));
  }
  else {
    oldhedefsicaklik = hedefsicaklik;
  }
  if (acik && mevcutsicaklik > (hedefsicaklik + hassasiyet))
  {
    Serial.println("ISITICI PASIF");
    acik = false;
    digitalWrite(relay, HIGH);
dene1:
    tsstatus = ThingSpeak.writeField(settingskanal, 2, (float)0, settingsapikey);
    if (tsstatus != 200)
    {
      digitalWrite(LED_BUILTIN, LOW);
      Serial.println("1.>> 200 degil koruması");
      delay(5000);
      goto dene1;
    }
  }
  if (!acik && mevcutsicaklik < (hedefsicaklik - hassasiyet))
  {
    Serial.println("ISITICI AKTIF");
    acik = true;
    digitalWrite(relay, LOW);
dene2:
    tsstatus = ThingSpeak.writeField(settingskanal, 2, (float)1, settingsapikey);
    if (tsstatus != 200) {
      digitalWrite(LED_BUILTIN, LOW);
      Serial.println("2.>> 200 degil koruması");
      delay(5000);
      goto dene2;
    }
  }
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("Hedef:" + String(hedefsicaklik, 2) + " Mevcut:" + String(mevcutsicaklik, 2) + " TSstatus:" + String(tsstatus) + " SensorGuncelleme:" + String(sensorsonguncelleme));
  delay(10000);
}
void blinker (int adet)
{
  for (int i = 0; i++; i < adet + 1)
  {
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
  }
}
