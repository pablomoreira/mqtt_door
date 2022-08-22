#include <Wire.h>
#include <TaskScheduler.h>
#include <OtaUtil.hpp>
#include <ESP8266WiFi.h>
#include <MyKeyPadi2c.h>
#include <Digital.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
#include <LittleFS.h>
#include <TimeLib.h>
#include <WiFiClientSecureBearSSL.h>
#include <CertStoreBearSSL.h>
#include <NTPClient.h>
#include <Buffer.h>
#include <config.h>

Scheduler runner;
void cb_wifi();
Task task_wifi(TASK_SECOND * 5, TASK_FOREVER, &cb_wifi, &runner);

void cb_getKey();
Task task_getKey(TASK_MILLISECOND * 20, TASK_FOREVER, &cb_getKey, &runner);

void cb_keyPadCheck();
Task task_KeyPadCheck(TASK_SECOND * 1 , TASK_FOREVER, &cb_keyPadCheck, &runner);

void cb_ota();
Task task_Ota(TASK_MILLISECOND * 100, TASK_FOREVER, &cb_ota, &runner);
void cb_led();
Task task_led(TASK_MILLISECOND * 200, TASK_FOREVER, &cb_led, &runner);
void cb_mqtt_connection();
Task task_mqtt_connection(TASK_SECOND * 5, TASK_FOREVER, &cb_mqtt_connection, &runner);
void cb_mqtt_loop();
Task task_mqtt_loop(TASK_MILLISECOND * 50, TASK_FOREVER, &cb_mqtt_loop, &runner);
void cb_getTime();
Task task_getTime(TASK_SECOND * 2, TASK_FOREVER, &cb_getTime, &runner);
void cb_mqtt_sendTime();
Task task_mqtt_sendTime(TASK_SECOND * 60, TASK_FOREVER, &cb_mqtt_sendTime, &runner);
void cb_relayClose();
Task task_relayClose(TASK_SECOND * 5, TASK_FOREVER, &cb_relayClose, &runner);



const uint8_t KEYPAD_ADDRESS = 0x20;

PCF8574 pcf(KEYPAD_ADDRESS);
MyKeyPadi2c* keyPad = new MyKeyPadi2c(&pcf);

Signal led(D8,LOW);
Signal relay(D3,LOW);

WiFiUDP ntpUDP;
NTPClient ntpClient(ntpUDP);


BearSSL::WiFiClientSecure *bear;// = new BearSSL::WiFiClientSecure();
BearSSL::CertStore certStore;

PubSubClient *mqttclient;
void OnMqttReceived(char *topic, byte *payload, unsigned int length);

Buffer *buffer = new Buffer(8);

void setup()
{
  
  Serial.begin(115200);
  Serial.println(__FILE__);
  
  LittleFS.begin();
  
  int numCerts = certStore.initCertStore(LittleFS, PSTR("/certs.idx"), PSTR("/certs.ar"));
  Serial.printf("Number of CA certs read: %d\n", numCerts);
  if (numCerts == 0) {
    Serial.printf("No certs found. Did you run certs-from-mozilla.py and upload the LittleFS directory before running?\n");
      // Can't connect to anything w/o certs!
  }
  //LittleFS.end();

  Wire.begin();
  Wire.setClock(400000);
  
  WiFi.begin(WIFISSID, WIFIPSSWD);
  
  keyPad->begin();
  
  OTAConf();
  task_KeyPadCheck.enable();
  task_getKey.enable();
  task_wifi.enable();
  task_led.enable();
  
  ntpClient.begin();

  bear = new WiFiClientSecure();  
    
  bear->setCertStore(&certStore);
  
  mqttclient = new PubSubClient(*bear);
  mqttclient->setServer(MQTT_SERVER, MQTT_PORT);
  setSyncInterval((u32_t)3600);
  led.upGrade(1);
}
void loop(){runner.execute();}

void cb_getKey()
{
  Serial.printf(".");
  if(keyPad->read() == true ){
    Serial.printf("%c",keyPad->getKey());
    if(keyPad->getKey() == '#'){
      if(buffer->len() > 0){
        if(task_mqtt_loop.isEnabled()){
          mqttclient->publish("/frcon/informatica/door_ext/key", (char*)buffer->toArrayChar());//Send password
        }
        Serial.printf("%s\n",buffer->toArrayChar());
        buffer->clear();
      }
    }
    else{
      if((millis() / 1000 - buffer->ttl())  > KEY_TTL) buffer->clear();
      buffer->add(keyPad->getKey()); 
    }
    task_getKey.delay(200);
  }  
}

void cb_wifi(){
  if (!WiFi.isConnected()){
    Serial.printf("No wifi connect.\n");
    led.downGrade(1);
    
    if(task_Ota.isEnabled() == true) task_Ota.disable();
    if(task_mqtt_connection.isEnabled() == true) task_mqtt_connection.disable();
    if(task_getTime.isEnabled() == true) task_getTime.disable();
  }
  else{
    if(task_Ota.isEnabled() == false) task_Ota.enable();
    if(task_getTime.isEnabled() == false) {
        task_getTime.enable();
        task_getTime.enableDelayed(3000);
      }

    if(task_mqtt_connection.isEnabled() == false && timeStatus() == 2) task_mqtt_connection.enable();
    led.upGrade(2);
    }
}

void cb_ota(){
  ArduinoOTA.handle();
}

void cb_led(){
  led.togle();
}

void cb_mqtt_connection(){
  if (mqttclient != nullptr){
    if (!mqttclient->connected())
    {
      Serial.printf("Attempting MQTT connection...");
      // Create a random client ID
      String clientId = "ESP8266Client-";
      clientId += WiFi.macAddress();
      // Attempt to connect
      if (mqttclient->connect(clientId.c_str(), MQTT_USER, MQTT_PSSWD))
      {
        if(task_mqtt_sendTime.isEnabled() == false) task_mqtt_sendTime.enable();
        if(task_mqtt_loop.isEnabled() == false) task_mqtt_loop.enable();
        led.upGrade(3);
        Serial.printf("connected\n");
        mqttclient->subscribe("/frcon/informatica/door_ext/open");//1 or 0 to open the door
        mqttclient->setCallback(OnMqttReceived);

      }
      else
      {
        Serial.printf("failed, rc= %d\n",mqttclient->state());
        if(task_mqtt_sendTime.isEnabled() == true) task_mqtt_sendTime.disable();
        if(task_mqtt_loop.isEnabled() == true) task_mqtt_loop.disable();
        task_mqtt_connection.enableDelayed(10000);
        led.downGrade(2);
      }
    }
  }
}
void cb_mqtt_loop(){
  mqttclient->loop();
}
 

void cb_getTime(){
  time_t _t = now();

  
  if((timeStatus() == timeNotSet ) || (timeStatus() == timeNeedsSync)){ 
    ntpClient.forceUpdate();
    setTime(ntpClient.getEpochTime());
    task_getTime.setInterval(TASK_SECOND * 2);
    if(timeStatus() == 2){
      bear->setX509Time(now());
      task_getTime.setInterval(TASK_MINUTE * 2);
    }
    _t = now();
    Serial.printf("Date Time post tatus: %d %04d-%02d-%02d %02d:%02d:%02d\n",timeStatus(),year(_t),month(_t),day(_t),hour(_t),minute(_t),second(_t));
  }

}



void cb_mqtt_sendTime(){
      if (task_mqtt_loop.isEnabled()){
        time_t _t = now();
        char  msg[256];

        sprintf(msg,"Time and tatus: %d %04d-%02d-%02d %02d:%02d:%02d",timeStatus(),year(_t),month(_t),day(_t),hour(_t),minute(_t),second(_t));

        mqttclient->publish("/frcon/informatica/door_ext/time", msg);//Pub the time
      
      }
}

void cb_keyPadCheck(){

  if (!keyPad->isConnected()){
    time_t _t = now();
    char msg[256];
    
    sprintf(msg,"%04d-%02d-%02d %02d:%02d:%02d KeyPad no Connect\n",year(_t),month(_t),day(_t),hour(_t),minute(_t),second(_t));

    Serial.printf(msg);
  }
}

void OnMqttReceived(char *topic, byte *payload, unsigned int length){
  switch (payload[0])
  {
    case '1':
      relay.high();
      task_relayClose.enable();
      task_relayClose.enableDelayed(5000);
      led.upGrade(25);
      printf("%c\n",payload[0]);
      task_getKey.disable();
      break;
    case '0':
      task_getKey.delay(5000);
      task_led.delay(5000);
      led.high();
      printf("%c\n",payload[0]);
      break;
    default:
      printf("Default/n");
    break;
  }
}

void cb_relayClose(){
  relay.low();
  task_relayClose.disable();
  led.downGrade(3);
  task_led.enableDelayed(1000);
  task_getKey.enable();
}