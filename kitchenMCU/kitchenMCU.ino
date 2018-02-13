#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <time.h>
int tempPin= A0;

long previousMillis = 0;
long interval = 1000;
float tempVal = 0.0;

//Valve control variable
int sensor = D2;
int solin = D0;
int sensorState = 0;
int stoveState;

int track = 1;
int track2 = 1;

//Time config variable
int timezone = 6 * 3600;
int dst = 0;

//app
String  i;
WiFiServer server(80);

void setup() {
 i = "";
 Serial.begin(115200);   //Serial connection

  //stove control
  pinMode(sensor, INPUT);
  pinMode(solin, OUTPUT);
  digitalWrite(solin, LOW);

  //disconnect wifi
  WiFi.disconnect();
  delay(3000);

  //connect to wifi
  WiFi.begin("morshed", "132152767");
  //waiting for connection
  Serial.println("Connecting......");
  while ((!(WiFi.status() == WL_CONNECTED))){
    delay(300);
    Serial.print(".");
  }

  Serial.println("Connected!!");
  Serial.println((WiFi.localIP()));
  server.begin();
  
  //Get DateTime from Server
  configTime(timezone, dst, "pool.ntp.org","time.nist.gov");
  Serial.println("\nWaiting for Internet time");

  while(!time(nullptr)){
     Serial.print("*");
     delay(1000);
  }
  Serial.println("\nTime response....OK");   
 
}

//Temperature
float temperature(){
  float tempVal = 0.0;
  float temp = analogRead(tempPin);
  float millivolts = ( temp/1024.0)*3300;
  float celsius = millivolts/10;
  tempVal = tempVal + celsius;
  return tempVal;
}

void loop() {
  //Serial.println(".........");
  //reading instructions from android
  WiFiClient client = server.available();
  if (!client) {  }
  //Serial.println(".........");
  //while(!client.available()){  delay(1); }
  i = (client.readStringUntil('\r'));
  i.remove(0, 5);
  i.remove(i.length()-9,9);
  
  //controlling according to app instruction
  
    //turning off the stove
    if (i == "OFF") {
      track = 0;
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html");
      client.println("");
      client.println("<!DOCTYPE HTML>");
      client.println("<html>");
      client.println("LED OFF");
      client.println("</html>");
      client.stop();
      delay(1);
    }
    
    //turning off the stove
    if (i == "OFFT") {
      track2 = 0;
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html");
      client.println("");
      client.println("<!DOCTYPE HTML>");
      client.println("<html>");
      client.println("LED OFF");
      client.println("</html>");
      client.stop();
      delay(1);
    }
  
  //controlling the stove according to the sensor value
    sensorState = digitalRead(sensor);  //read sensor data
    if(sensorState == 1){
      if(track == 0 || track2 == 0){
        digitalWrite(solin, LOW);
        stoveState = 0;
      }
      else{
        digitalWrite(solin, HIGH);
        stoveState = 1;
      }
    }
    else{
      digitalWrite(solin, LOW);
      stoveState = 0;
      track = 1;
      track2 = 1;
    }
    //Serial.print(sensorState);
  
  //............................
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis > interval){
    previousMillis = currentMillis;
    //Time from server 
    time_t now = time(nullptr);
    struct tm* p_tm = localtime(&now);
    String day = String(p_tm->tm_mday);
    String mon = String(p_tm->tm_mon+1);
    String year = String(p_tm->tm_year + 1900);
    String hour = String(p_tm->tm_hour);
    String mint = String(p_tm->tm_min);
    String sec = String(p_tm->tm_sec);
    String curTime = hour+":"+mint+":"+sec;
    String daymon = day+"/"+mon+"/"+year+"  "+curTime;
    Serial.println(daymon);
  
    if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status
      StaticJsonBuffer<300> JSONbuffer;   //Declaring static JSON buffer
      JsonObject& JSONencoder = JSONbuffer.createObject(); 
   
      JSONencoder["sensorType"] = "Temperature";
      JSONencoder["tempvalue"] = temperature();
      JSONencoder["stoveState"] = stoveState;
      JSONencoder["timestamp"] = daymon;
   
      char JSONmessageBuffer[300];
      JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
      Serial.println(JSONmessageBuffer);
  
       HTTPClient http;    //Declare object of class HTTPClient
     
       http.begin("http://192.168.43.147:5000/sensor");      //Specify request destination
       http.addHeader("Content-Type", "application/json");  //Specify content-type header
     
       int httpCode = http.POST(JSONmessageBuffer);   //Send the request
       String payload = http.getString();                  //Get the response payload
     
       Serial.println(httpCode);   //Print HTTP return code
       Serial.println(payload);    //Print request response payload
     
       http.end();  //Close connection
   
   }
   else{
      Serial.println("Error in WiFi connection");
   }
  }
}

