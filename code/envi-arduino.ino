//Include Libraries

//Library for temperature and humidity sensor
#include "DHT.h"               
//Library for motor driver
#include <AFMotor.h>

//Motor connection 
AF_DCMotor motorFL(1);
AF_DCMotor motorFR(4);
AF_DCMotor motorBL(2);
AF_DCMotor motorBR(3);

//Bluetooth is connected to RX3 and TX3
#define RXPin RX3             
#define TXPin TX3             
//Gas sensor is connected to A7
#define gasPin A7             
//Dust sensor is connected to A8 and its led to digital pin 31
#define dustPin A8            
#define ledPin 31             
//Temperature-humidity sensor is connected to digital pin 33
#define DHTPIN 33             

//constants for estimating the dust concentration
#define COV_RATIO 0.2                   
#define NO_DUST_VOLTAGE 400
#define SYS_VOLTAGE 5000
#define DHTTYPE DHT11

//Create a DHT object for the temperature-humidity sensor
DHT dht(DHTPIN, DHTTYPE);
//Variables for estimating time passed 
unsigned long prevTime, curTime;
//Variables for estimating the dust concentration
int adcvalue; 
float density, voltage;

//The function that estimates the dust concentration
int Filter(int m) {
  static int flag_first = 0, _buff[10], sum;
  const int _buff_max = 10;
  int i;
  
  if(flag_first == 0)
  {
    flag_first = 1;

    for(i = 0, sum = 0; i < _buff_max; i++)
    {
      _buff[i] = m;
      sum += _buff[i];
    }
    return m;
  }
  else
  {
    sum -= _buff[0];
    for(i = 0; i < (_buff_max - 1); i++)
    {
      _buff[i] = _buff[i + 1];
    }
    _buff[9] = m;
    sum += _buff[9];
    
    i = sum / 10.0;
    return i;
  }
}

//Setup only runs once
void setup() {
  //set the pinmode of the led of the dust sensor to output
  pinMode(ledPin,OUTPUT);               
  //Turn of the led of the dust sensor
  digitalWrite(ledPin, LOW);          
  //Begin the DHT object for the temperature-humidity sensor
  dht.begin();
  //Begin serial communication for debugging
  Serial.begin(9600); 
  //Begin serial communication with bluetooth module
  Serial3.begin(9600);  
  //Debugging messages
  Serial.print("Serial3 started at "); 
  Serial.println(9600);                 
  Serial.println(" ");
  //Set the time of the begining of the program
  prevTime = millis();
}

//Loop runs every time
void loop() {
  //Variable that holds the incoming command from the bluetooth
  String command;
  //Get the current time
  curTime = millis();
  
  //Every 10 minutes we test to see if there is a high concentration of dangerous gases in the room
  if (curTime - prevTime > 600000) {
    int gas = analogRead(gasPin);
    //We map the value to a 1-100 scale
    int mappedGas = map(gas, 1, 1024, 1, 100);
    //Debugging messages
    Serial.print("gas:");                           
    Serial.print(mappedGas);
    Serial.println("%");
    //If the value is over 20% we send it to the Smartphone via bluetooth
    if (mappedGas > 20) {
      Serial3.println(mappedGas);  
    }
    prevTime = curTime;
  }

  //Check for a possible incoming command from bluetooth
  if (Serial3.available() > 0) {  
    //Store the incoming command to the command variable
    command = Serial3.readString();   
    //Debugging messages
    Serial.print("Command:");         
    Serial.println(command);
    //Envi wakes up and moves around for a few seconds
    rotate();
    delay(1300);
    Stop();
    //Actions for get temperature command
    if (command == "temp") {
      Serial.print("Temperature:");
      float t = dht.readTemperature();                              
      Serial3.println(t);
      //Debugging messages
      Serial.print(t);
      Serial.println("C");
    }
    //Actions for get humidity command
    if (command == "hum") {
      float h = dht.readHumidity(); 
      Serial3.println(h);
      //Debugging messages
      Serial.print("Humidity:");                           
      Serial.print(h);
      Serial.println("%");
    }
    //Actions for get dust command
    if (command == "getDust") {
      digitalWrite(ledPin, HIGH);
      delayMicroseconds(280);
      adcvalue = analogRead(dustPin);
      digitalWrite(ledPin, LOW);
      adcvalue = Filter(adcvalue);
      voltage = (SYS_VOLTAGE / 1024.0) * adcvalue * 11;
      if(voltage >= NO_DUST_VOLTAGE)
      {
        voltage -= NO_DUST_VOLTAGE;
        density = voltage * COV_RATIO;
      }
      else 
        density = 0;
      Serial3.println(density);
      //Debugging messages
      Serial.println("microparticles concentration: ");
      Serial.print(density);
      Serial.println("mg/m3");
    }
    
    //Future commands for moving envi arround. Not implemented yet in smartphone
    if (command == "F") {
      forward();
      delay(1000);
      Stop();
    }
    if (command == "B") {
      back();
      delay(1000);
      Stop();
    }
    if (command == "L") {
      left();
      delay(1000);
      Stop();
    }
    if (command == "R") {
      right();
      delay(1000);
      Stop();
    }
    if (command == "RT") {
      rotate();
      delay(1000);
      Stop();
    }
  }
  //delay(100);
} 

void forward()
{
  motorFL.setSpeed(100); 
  motorFL.run(FORWARD); 
  motorFR.setSpeed(100); 
  motorFR.run(FORWARD); 
  motorBL.setSpeed(100);
  motorBL.run(FORWARD); 
  motorBR.setSpeed(100);
  motorBR.run(FORWARD); 
}
 
void back()
{
  motorFL.setSpeed(100); 
  motorFL.run(BACKWARD); 
  motorFR.setSpeed(100); 
  motorFR.run(BACKWARD); 
  motorBL.setSpeed(100); 
  motorBL.run(BACKWARD); 
  motorBR.setSpeed(100); 
  motorBR.run(BACKWARD); 
}
 
void left()
{
  motorFL.setSpeed(200); 
  motorFL.run(FORWARD); 
  //motorFR.setSpeed(100); 
  //motorFR.run(BACKWARD); 
  //motorBL.setSpeed(100);
  //motorBL.run(BACKWARD);  
  motorBR.setSpeed(200); 
  motorBR.run(FORWARD); 
}
 
void right()
{
  motorFL.setSpeed(200); 
  motorFL.run(FORWARD); 
  motorFR.setSpeed(200); 
  motorFR.run(FORWARD); 
  motorBL.setSpeed(200); 
  motorBL.run(BACKWARD); 
  motorBR.setSpeed(200); 
  motorBR.run(BACKWARD); 
} 

void forward_leftturn()
{
  motorFL.setSpeed(0); 
  motorFL.run(RELEASE); 
  motorFR.setSpeed(80); 
  motorFR.run(FORWARD); 
  motorBL.setSpeed(80); 
  motorBL.run(FORWARD); 
  motorBR.setSpeed(0); 
  motorBR.run(RELEASE); 
}

void forward_rightturn()
{
  motorFL.setSpeed(80); 
  motorFL.run(FORWARD); 
  motorFR.setSpeed(0); 
  motorFR.run(RELEASE); 
  motorBL.setSpeed(0); 
  motorBL.run(RELEASE); 
  motorBR.setSpeed(80); 
  motorBR.run(BACKWARD); 
}

void rotate()
{
  motorFL.setSpeed(130); 
  motorFL.run(FORWARD); 
  motorFR.setSpeed(130); 
  motorFR.run(BACKWARD); 
  motorBL.setSpeed(130); 
  motorBL.run(FORWARD); 
  motorBR.setSpeed(130); 
  motorBR.run(BACKWARD); 
}


void Stop()
{
  motorFL.setSpeed(0); 
  motorFL.run(RELEASE); 
  motorFR.setSpeed(0); 
  motorFR.run(RELEASE); 
  motorBL.setSpeed(0); 
  motorBL.run(RELEASE); 
  motorBR.setSpeed(0); 
  motorBR.run(RELEASE); 
}
