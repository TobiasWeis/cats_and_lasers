#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>
/* needs to be placed in separate header,
 *  otherwise the arduino ide will "escape"/create prototypes
 *  for javascript "function" statements and destroy the javascript-part
 */
#include "index.h"

#define LASER_PIN D4
#define SERVO_PAN_PIN D2
#define SERVO_TILT_PIN D3

const char* ssid = "ROUTER_SSID_HERE"; //Hier SSID eures WLAN Netzes eintragen
const char* password = "ROUTER_PWD_HERE"; //Hier euer Passwort des WLAN Netzes eintragen

ESP8266WebServer server ( 80 );
const char* www_username = "laser";
const char* www_password = "laser";

Servo servo1; // base rotation
Servo servo2; // up/down

bool isLaserOn = false;

//------------------------- Arduino-related code
void setup() {
  pinMode(LASER_PIN, OUTPUT);
  digitalWrite(LASER_PIN,LOW);

  Serial.begin(115200);
  delay(10);

  WiFi.hostname("catlaser");
  WiFi.begin(ssid,password);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  server.on ( "/", handleRoot );
  server.begin();
  Serial.println ( "HTTP server started" );

  Serial.println();
  Serial.println(WiFi.localIP());

  attach_servos();
  test_functionality();
  detach_servos(); // turn the pwm-pulse off to avoid noises and power consumption
}

void loop(){
  server.handleClient();
  delay(1000);
}

//------------------------- web-server-related code
String getPage(){
  return MAIN_page;
}

void handleSubmitAction(){
  String action = server.arg("execute_action");
  Serial.print(action);

  if(action == "sweep_once"){
    attach_servos();
    sweep_laser(1);
    detach_servos();
    server.send(200, "text/html", "OK");
  }else if(action == "sweep_five"){
    attach_servos();
    sweep_laser(5);
    detach_servos();
    server.send(200, "text/html", "OK");
  }else if(action == "test"){
    attach_servos();
    test_functionality();
    detach_servos();
    server.send(200, "text/html", "OK");
  }else if(action == "toggle_laser"){
    toggle_laser();
    server.send(200, "text/html", "OK");
  }else if(action == "apply_servo_value"){
    Serial.println("Servo values are ");
    Serial.println(server.arg("servo_value1"));
    Serial.println(server.arg("servo_value2"));
    
    int value1 = server.arg("servo_value_1").toInt();
    int value2 = server.arg("servo_value_2").toInt();
    
    attach_servos();
    
    servo1.write(value1);
    servo2.write(value2);
    
    delay(2000);
    
    detach_servos();
    server.send(200, "text/html", "OK");
    //server.send ( 200, "text/html", getPage() );
  }else{
    Serial.println("Error Servo Value");
  }
}

void handleRoot() {
  if(!server.authenticate(www_username, www_password))
    return server.requestAuthentication();
  
  if (server.hasArg("execute_action")){
    Serial.println("Executing action");
    handleSubmitAction();
  } else{
    server.send (200, "text/html", getPage());
  }
}


//------------------------- hardware related code
void detach_servos(){
  servo1.detach();
  servo2.detach();
}

void attach_servos(){
  servo1.attach(SERVO_PAN_PIN);
  servo2.attach(SERVO_TILT_PIN);
}

void laser_on(){
  digitalWrite(LASER_PIN, HIGH);
  isLaserOn=true;
}

void laser_off(){
  isLaserOn=false;
  digitalWrite(LASER_PIN, LOW);
}

void toggle_laser(){
  if(isLaserOn){
    digitalWrite(LASER_PIN,LOW);
    isLaserOn=false; 
  }else{
    digitalWrite(LASER_PIN,HIGH);
    isLaserOn=true;
  }
}

// sweep each servo from 0 to 180
// and blink the laser to test functionality
void test_functionality(){
  // servo 1
  for(int angle=0; angle<180;angle++){
    servo1.write(angle);
    delay(10);
  }
    for(int angle=180; angle>0;angle--){
    servo1.write(angle);
    delay(10);
  }

  // servo 2
  for(int angle=0; angle<180;angle++){
    servo2.write(angle);
    delay(10);
  }
    for(int angle=180; angle>0;angle--){
    servo2.write(angle);
    delay(10);
  }

  // laser
  for(int num=0;num<5;num++){
    laser_on();
    delay(500);
    laser_off();
    delay(500);
  }
}

void sweep_laser(int num){
  laser_on();
  
  int base = 50;
  int scale = 10;

  for(int k=0; k<num; k++){
    for(int i=0; i<180;i++){
      servo1.write(i);
      // 0 at 90 degrees, 
      // 90 at 0 and 180
      servo2.write(base + sin(deg2rad(i)*2)*scale); // @180: 90
      delay(30);
    }
      for(int i=180; i>0;i--){
      servo1.write(i);
      servo2.write(base-sin(deg2rad(i)*2)*scale); // @180: 0
      delay(30);
    }
  }
  laser_off();
}

//------------------------- utility functions
int rad2deg(float rad){
  return (rad*4068)/71;
}

float deg2rad(int deg){
  return (deg*71)/4068.0;
}
