#include <LiquidCrystal.h>
#include <Servo.h>
#include <stdlib.h>
#include <string.h>
#include<IRremote.hpp>
// definire pini

#define RLED 2  // leduri pentru semnalizarea functionarii motorului
#define GLED 3
// obs: led rosu aprins cand motorul este in standby si red verde aprins cand motorul functioneaza

#define TRIG_SENS2 4  // senzor ultrasonic amplasat dupa motor
#define ECHO_SENS2 A4

#define TRIG_SENS1 5  // senzor ultrasonic amplasat inainte de motor
#define ECHO_SENS1 A5  

#define MOTOR 6 // activare motor

#define BUZZER 13 // buzzer

// DATE INITIALE

long readIRValue;

long INCREASE_PARKING_LOTS = 3927310080;
long DECREASE_PARKING_LOTS = 4161273600;
long ENABLE_MODE = 4127850240;
long PASS1 = 4077715200;
long PASS2 = 3877175040;
long PASS3 = 2707357440;
long PASS4 = 4144561920;
long PASS = 3158572800;

LiquidCrystal lcd(11, 12, 10, 9, 8, 7);  // setam pinurile ecranului

Servo myservo;

int space; // spariu disponibil in parcare (nr locuri de parcare)

#define MAX_DISTANCE 5 // distanta admisa pentru a nu detecta o masina
int INITIAL_SPACE = 10; // locuri de parcare

// functii

int read_distance(int trigPin, int outPin) // citire senzor ultrasonic
{
    digitalWrite(trigPin, LOW); // initial setam senzorul pe LOW
    delayMicroseconds(5);

    digitalWrite(trigPin, HIGH); // pornim senzorul
    delayMicroseconds(20);
    digitalWrite(trigPin, LOW);

    int result = pulseIn(outPin, HIGH) / 58;

    Serial.print("Distanta: ");
    Serial.print(trigPin);
    Serial.print(" - ");
    Serial.print(result);
    Serial.println();
    return result; // returnam distanta in cm
}

int detect_car(int trigSens, int echoSens) // detecteaza prezenta unei masini
{
    if (read_distance(trigSens, echoSens) < MAX_DISTANCE)
    {
        return 1;
    }
    return 0;
}

void open_gate() // deschide bariera de la intrare
{
    Serial.println("Activare motor pentru deschidere bariera.");
    digitalWrite(RLED, LOW);
    digitalWrite(GLED, HIGH); // porneste becul verde - acces masini
    myservo.attach(MOTOR); // porneste motorasul 
    myservo.write(85);
    delay(675);
    myservo.detach();
}

void close_gate() // inchide bariera de la intrare
{
    Serial.println("Activare motor pentru inchidere bariera.");
    digitalWrite(GLED, LOW);
    digitalWrite(RLED, HIGH); // opreste led rosu - masinile nu pot accesa
    myservo.attach(MOTOR); // invarte motoras
    myservo.write(100);
    delay(583);
    myservo.detach();  
}

void print_remaining_spots() // functie afisare locuri de parcare disponibile
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("LOCURI LIBERE:");
    lcd.setCursor(5, 1);
    lcd.print(space);
}

void print_message(char* message) // functie afisare mesaj pe ecran LCD
{
    lcd.clear();

    if(strchr(message, ' ') != NULL)
    {
        lcd.setCursor(0, 0);
        for(int i=0; i<=(strchr(message, ' ') - message); i++)
        {
          lcd.setCursor(i, 0);
          lcd.print(message[i]);
        }
        lcd.setCursor(0, 1);
        lcd.print(strchr(message, ' '));
    }
    else{
      lcd.setCursor(0, 0);
      lcd.print(message);
    }
    
}

int check_overpassing(int TRIG1, int ECHO1, int TRIG2, int ECHO2)
{
    while(detect_car(TRIG2, ECHO2))
    {
        if(!detect_car(TRIG1, ECHO1))
        {
            delay(1000);
            if(detect_car(TRIG1, ECHO1))
            {
                Serial.println("Overpass detectat!");
                return 1;
            }
        }
    }
    return 0;
}

void car_access() // functie principala acces masini - sosire in parcare
{
    if(detect_car(TRIG_SENS1, ECHO_SENS1) && !detect_car(TRIG_SENS2, ECHO_SENS2))
    {
        if(space)
        {
            char message[25] = "BUN VENIT";
            print_message(message);
            open_gate();
            delay(50);
            while(1)
            {
              if(!detect_car(TRIG_SENS1, ECHO_SENS1) && !detect_car(TRIG_SENS2, ECHO_SENS2))
              {
                  break;
              }
              // pornire alarma in caz de overpassing
              if(check_overpassing(TRIG_SENS1, ECHO_SENS1, TRIG_SENS2, ECHO_SENS2))
              {
                tone(BUZZER, 1000);;
                while(1)
                {
                    char message45[25]="ASTEPTATI FINALIZAREA";
                    print_message(message45);
                    if(!detect_car(TRIG_SENS1, ECHO_SENS1))
                    {
                        noTone(BUZZER);
                        break;
                    }
                }
              }
            }
            close_gate();
            delay(50);
            space--;
            print_remaining_spots();
        }
        else
        {
            char message[25] = "LOCURI INSUFICIENTE";
            print_message(message);
        }
    }
}


void car_leaving()
{
    if(!detect_car(TRIG_SENS1, ECHO_SENS1) && detect_car(TRIG_SENS2, ECHO_SENS2))
    {
        char message[25] = "LA REVEDERE";
        print_message(message);
        open_gate();
        while(1)
        {
            if(!detect_car(TRIG_SENS1, ECHO_SENS1) && !detect_car(TRIG_SENS2, ECHO_SENS2))
            {
              break;
            }
        }
        close_gate();
        if(space < INITIAL_SPACE) space++;
        print_remaining_spots();
    }
}

int verify_pass()
{
    int i=0;
    long code[4];
    while(1)
    {
      IrReceiver.resume();
      if(IrReceiver.decode())
      {
        if(IrReceiver.decodedIRData.decodedRawData == 0)
        {
          continue;
        }
        else
        {
          if(i == 0)
          {
            char mes[25] = "PAROLA: *___";
            print_message(mes);
          }
          if(i == 1)
          {
            char mes[25] = "PAROLA: **__";
            print_message(mes);
          }
          if(i == 2)
          {
            char mes[25] = "PAROLA: ***_";
            print_message(mes);
          }
          if(i == 3)
          {
            char mes[25] = "PAROLA: ****";
            print_message(mes);
          }
          Serial.println(IrReceiver.decodedIRData.decodedRawData);
          code[i] = IrReceiver.decodedIRData.decodedRawData;
          i++;
          if(i == 4) break;
        }
      } 
    }
    if ((code[0] == PASS1) && (code[1] == PASS2) && (code[2] == PASS3) && (code[3] == PASS4))
    {
      char mes2[25] = "PAROLA CORECTA";
      print_message(mes2);
      return 1;
    }
    else
    {
      char mes2[25] = "PAROLA INCORECTA";
      print_message(mes2);
      return 0;
    }
}

void edit_spots()
{
  while(1)
  {
      IrReceiver.resume();
      if(IrReceiver.decode())
      {
        if(IrReceiver.decodedIRData.decodedRawData == INCREASE_PARKING_LOTS)
        {
          if(space == INITIAL_SPACE)
          { 
            space++;
            INITIAL_SPACE++;
          }
          else
          {
            space++;
          }
          print_remaining_spots();
        }
        
        if(IrReceiver.decodedIRData.decodedRawData == DECREASE_PARKING_LOTS)
        {
          space--;
          print_remaining_spots();
        }

        if(IrReceiver.decodedIRData.decodedRawData == ENABLE_MODE)
        {
          break;
        }
      }
  }
  return;
}

void enable_admin()
{
    lcd.clear();
    char message[25] = "ADMIN MODE";
    print_message(message);
    open_gate();
    delay(4000);
    lcd.clear();
    digitalWrite(GLED, LOW);
    while(1)
    {
       IrReceiver.resume();
       if(IrReceiver.decode())
       {
          if(IrReceiver.decodedIRData.decodedRawData == ENABLE_MODE)
          {
            break;
          }
          if(IrReceiver.decodedIRData.decodedRawData == PASS)
          {
            char mes[25] = "PAROLA: ____";
            print_message(mes);
            if(verify_pass())
            {
              print_remaining_spots();
              edit_spots();
            }
          }
       }
    }
    char message2[25] = "ADMIN MODE OFF";
    print_message(message2);
    close_gate();
    delay(1000);
    print_remaining_spots();
}

void setup() 
{
    lcd.begin(16, 2);     // APRINDERE ECRAN
    char message[25] = "INITIALIZARE";
    print_message(message); // AFISARE MESAJ INITIAL
    
    pinMode(RLED, OUTPUT);
    pinMode(GLED, OUTPUT); //iesirea arduino intrarea de aprindere a becului

    pinMode(TRIG_SENS1, OUTPUT);
    pinMode(ECHO_SENS1, INPUT);   // intrarea senzorilor ultrasonici e un semnal de iesire al placii arduino
    pinMode(TRIG_SENS2, OUTPUT);  // iar valoarea returnata este un semnal high daca se indeplineste conditia de distanta a senzorului
    pinMode(ECHO_SENS2, INPUT);

    pinMode(BUZZER, OUTPUT);  // setare pin de output 

    Serial.begin(9600);
    IrReceiver.begin(A3, 0);

    space = INITIAL_SPACE;
    digitalWrite(RLED, HIGH); // activare bec rosu
    print_remaining_spots();
}

void loop() 
{
      if(IrReceiver.decode())
      {
        readIRValue = IrReceiver.decodedIRData.decodedRawData;
        Serial.print(readIRValue);
        if(readIRValue == ENABLE_MODE)
        {
            enable_admin();
        }
        IrReceiver.resume();
      }
      car_access();
      delay(300);
      car_leaving();
}
