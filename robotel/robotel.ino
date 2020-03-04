#include <Servo.h>
#include <NewPing.h>
#include <SoftwareSerial.h>
// Pinii motor 1 - dreapta
#define mpin00 5
#define mpin01 6
// Pinii motor 2 - stanga
#define mpin10 3
#define mpin11 11
#define stateBlt 12  //enable bluetooth pin 
#define echoPin 10
#define trigPin 9
#define servoPin 8

bool autoC = false;
char char1;

int forward = 0;
int typeMove = -1;

Servo srv;

SoftwareSerial BTSerial(2, 7); // RX | TX

#define TRIGGER_PIN  10  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     9  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 200 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

void setup() {
 // configurarea pinilor motor ca iesire, initial valoare 0
 digitalWrite(mpin00, 0);
 digitalWrite(mpin01, 0);
 digitalWrite(mpin10, 0);
 digitalWrite(mpin11, 0);
 pinMode (mpin00, OUTPUT);
 pinMode (mpin01, OUTPUT);
 pinMode (mpin10, OUTPUT);
 pinMode (mpin11, OUTPUT);

 // pin LED
 pinMode(13, OUTPUT);
 
 pinMode (echoPin, INPUT);
 pinMode (trigPin, OUTPUT);
 digitalWrite(trigPin,LOW);

 BTSerial.begin(9600);  // HC-05 default speed in AT command more
 Serial.begin(9600);
 srv.attach(servoPin);
}
 
int calcDistance(){
   digitalWrite(trigPin,HIGH);
   delayMicroseconds(10);
   digitalWrite(trigPin,LOW);
   long duration = pulseIn(echoPin, HIGH, 50000); //read return echo pulse lenght
   int cmdec = (duration/2)/29.1; 
   return cmdec;
}

// FuncČ›ie pentru controlul unui motor
// Intrare: pinii m1 Č™i m2, direcČ›ia Č™i viteza
void StartMotor (int m1, int m2, int forward, int speed)
{

   if (speed==0) // oprire
   {
    digitalWrite(m1, 0);
    digitalWrite(m2, 0);
   }
   else
   {
    if (forward)
   {
    analogWrite(m1, speed); // folosire PWM
    digitalWrite(m2, 0);
   }
   else
   {
    analogWrite(m2, speed);
    digitalWrite(m1, 0);
   }
   }
}

void playWithServo(int pin)
{
 srv.attach(pin);
 for (int i =0; i <= 180 ; i+=45){
  srv.write(i);
  calcDistance();
  delay(1000);
 }
}

void controlMove(){
    char c;
    while(BTSerial.available() > 0 ){
       c = (char)BTSerial.read();
       if (c == '5'){ //auto
          autoC = !(autoC);     
       } 
       else if ( !autoC ){
           if (c == '0'){ //inainte
              inainte();
           }
           else if (c == '1')  //inapoi
              inapoi();     
          else if (c == '2') //stanga     
              intoarceStanga();    
          else if (c == '3')  //drepta
              intoarceDreapta();
          else if (c == '4') //stop
              stopp();
       } 
       if (c == '5' && autoC == 0){
          stopp();
       }
    }
}

// Executa oprire motoare, urmata de delay
void delayStopped(int ms)
{
 StartMotor (mpin00, mpin01, 0, 0);
 StartMotor (mpin10, mpin11, 0, 0);
 delay(ms);
}

void inainte(){
  delayStopped(500);  //oprim motoarele inainte de a schimba directia
  for (int s=0; s<= 128; s+=10){
    StartMotor (mpin00, mpin01, 1, s);
    StartMotor (mpin10, mpin11, 1, s);
    delay(30);
  }
}

void inapoi(){
  delayStopped(500);
  for (int s=0; s<= 128; s+=5){
    StartMotor (mpin00, mpin01, 0, s);
    StartMotor (mpin10, mpin11, 0, s);
    delay(30);
  }
}

void stopp(){
  for (int s=128; s>=0; s-=5){
    StartMotor (mpin00, mpin01, 0, 0);
    StartMotor (mpin10, mpin11, 0, 0); 
    delay(30);
  }  
}

void intoarceDreapta(){
  delayStopped(500);
  for (int s=0; s<= 128; s+=5){
    StartMotor (mpin00, mpin01, 1, s);
    StartMotor (mpin10, mpin11, 0, s); 
    delay(10);
  }
  delay(100);
  StartMotor (mpin00, mpin01, 1, 128);
  StartMotor (mpin10, mpin11, 1, 128);
}

void intoarceStanga(){
 delayStopped(500);
  for (int s=0; s<= 128; s+=5){
    StartMotor (mpin00, mpin01, 0, s);
    StartMotor (mpin10, mpin11, 1, s); 
    delay(10);
  }
  delay(100);
  StartMotor (mpin00, mpin01, 1, 128);
  StartMotor (mpin10, mpin11, 1, 128);
}

void schimbaDirectia(int dist){
  stopp();
  srv.write(30); //se uita la dreapta
  delay(150);
  int dreapta = calcDistance();
  srv.write(120); //se uita la dreapta
  delay(150);
  int stanga = calcDistance();
  if (dreapta >= stanga)
    intoarceDreapta();
  else if (dreapta < stanga)
    intoarceStanga();
}
void loop() {
   controlMove();
   
   if (autoC ){  
     srv.write(90);
     delay(50); 
     int dist = calcDistance();
     BTSerial.println(dist);
     if (dist > 20)
        inainte();
     else 
        schimbaDirectia(dist);
   }
}
