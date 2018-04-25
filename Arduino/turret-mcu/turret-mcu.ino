/*     Turret Control
 *      Created by: Naufalino Fadel Hutomo, anharaf
 *     Apr2018 
 */

#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include <string.h>
#include <Wire.h>
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define stepPin 2 //pulse pin for controlling turret stepper
#define dirPin 4 //pulse pin for controlling turret stepper

//CONSTANT
const float yaw_RES = 1.8/4 ; //nema 17 resolution is 1.8 degree, with gear ratio between motor and turret is 1:4

String readStr;
float yaw_set = 0;   //for turret yaw set condition
float yaw_act=0;  //heading measurement of yaw

bool step_dir = HIGH; //CW is HIGH
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Update these with values suitable for your network.

const char* ssid = "Titanic";
const char* password = "samakayakhotspotitb";
const char* mqtt_server = "192.168.0.111";
const char* mqtt_userName = "anharaf";
const char* mqtt_password = "anhar1234";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  //Serial.print("Message arrived [");
  //Serial.print(topic);
  //Serial.print("] ");
  for (int i = 0; i < length; i++) {
    //Serial.print((char)payload[i]);
  }
  Serial.println();
  //payload[length] = '\0';
  //Serial.print("payload : [");
  //Serial.print((char *)payload);
  //Serial.print("]");
  StaticJsonBuffer<300> JSONBuffer;   //Memory pool
  JsonObject& parsed = JSONBuffer.parseObject((char *)payload); //Parse message
  
  if (!parsed.success()) {
    Serial.println("parseObject() failed");
    return;
  }
  
  int yaw = parsed["yaw"];
  int pitch= parsed["pitch"];
  int fire=parsed["fire"];
  yaw_set= yaw;
  move_all();   // move all actuator
  
  Serial.println(yaw_set);
//  Serial.println(pitch);
//  Serial.println(fire);
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Sets pins Mode
  //pinMode(stepPin,OUTPUT); 
  pinMode(stepPin,OUTPUT); 
  pinMode(dirPin,OUTPUT);
  
  Serial.print("Setup start");
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  client.setCallback(callback);

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client", mqtt_userName, mqtt_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf(msg, 75, "hello world #%ld", value);
    Serial.print("Publish message: ");
    /*
    snprintf(msg,8,"y%d\n",yaw_act);    //the message transmitted to raspi is the alue of yaw act as a feedback
    */
    Serial.println(msg);
    client.publish("outTopic", msg);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//bool get_setPoint(){ }

void move_turret(float degree){
  //Function to move turret with nema 17 stepper
  float err = normalDeg(degree - yaw_act);
  int step_count;
  
  if (err>0)
  {
    digitalWrite(dirPin,LOW); // Enables the motor to move with vector direction upward the axis. In this case, move gear ccw -> move turret in clockwise direction
    step_count = round(err / yaw_RES);
  }else
  {
    digitalWrite(dirPin,HIGH); // Enables the motor to move turret in counterclockwise direction
    step_count = round(-err / yaw_RES);
  }
  
  
  for (int i=0; i < step_count; i++)
  {
    digitalWrite(stepPin,HIGH); 
    delayMicroseconds(1000); 
    digitalWrite(stepPin,LOW); 
    delayMicroseconds(1000);
  }
  
  if (err>0)
  {
    yaw_act = normalDeg(yaw_act + step_count * yaw_RES);  
  }else
  {
    yaw_act = normalDeg(yaw_act - step_count * yaw_RES);
  }
  /*
  Serial.print("yaw_act = ");
  Serial.print(yaw_act);
  */
}

void move_all(){
  if(yaw_set != yaw_act) //if the value change
  {
    move_turret(yaw_set);
  }
}

float normalDeg(float deg)
{
  //normalized degree value into -179.99 until 180
  while (deg <= -180)
  {
    deg += 360;
  }
  
  while (deg > 180)
  {
    deg -= 360;
  }
  return deg;
}

bool isTolerant(float ex, float real, float tol)
{
  // check whether the value is under accuracy of measurement tool
  bool ret = false;
  if ( ((real - ex) < tol) && ((ex-real) < tol) )
  {
    ret = true;
  }
  return ret;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////