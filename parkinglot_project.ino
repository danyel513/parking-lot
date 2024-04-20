#include <LiquidCrystal.h>
#include <Servo.h>
#include <stdlib.h>
#include <string.h>

// definire pini

#define RLED 13  // leduri pentru semnalizarea functionarii motorului
#define GLED 12
// obs: led rosu aprins cand motorul este in standby si red verde aprins cand motorul functioneaza

#define TRIG_SENS2 11  // senzor ultrasonic amplasat dupa motor
#define ECHO_SENS2 10

#define TRIG_SENS1 9  // senzor ultrasonic amplasat inainte de motor
#define ECHO_SENS1 8  

#define MOTOR 6 // activare motor

// DATE INITIALE

LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);  // setam pinurile ecranului
int space; // spariu disponibil in parcare (nr locuri de parcare)
Servo myservo;

#define MAX_DISTANCE 100 // distanta admisa pentru a nu detecta o masina
#define INITIAL_SPACE 10 // locuri de parcare

// functii

int read_distance(int trigPin, int outPin) // citire senzor ultrasonic
{
    digitalWrite(trigPin, LOW); // initial setam senzorul pe LOW
    delayMicroseconds(5);

    digitalWrite(trigPin, HIGH); // pornim senzorul
    delayMicroseconds(20);
    digitalWrite(trigPin, LOW);

    return pulseIn(outPin, HIGH) / 58; // returnam distanta in cm
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
    digitalWrite(RLED, LOW);
    digitalWrite(GLED, HIGH); // porneste becul verde - acces masini
    for(int position=0; position <= 90; position++)
    {
        myservo.write(position); // ridica bariera vertical (90grade)
        delay(15);
    }
}

void close_gate() // inchide bariera de la intrare
{
    digitalWrite(GLED, LOW);
    digitalWrite(RLED, HIGH); // opreste led rosu - masinile nu pot accesa
    print_message("INCHIDERE BARIERA");
    for(int position=90; position >= 0; position--)
    {
        myservo.write(position);
        delay(15);
    }
    
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

void car_access() // functie principala acces masini - sosire in parcare
{
    if(detect_car(TRIG_SENS1, ECHO_SENS1) && !detect_car(TRIG_SENS2, ECHO_SENS2))
    {
        if(space)
        {
            print_message("BUN VENIT");
            open_gate();
            while(1)
            {
               if(!detect_car(TRIG_SENS1, ECHO_SENS1) && !detect_car(TRIG_SENS2, ECHO_SENS2))
               {
                  break;
               }
               //
               // ------------------------
               // - tratare cazuri de deschidere sau alarma in caz de depasire numar masini & stuff
               //

            }
            close_gate();
            space--;
            print_remaining_spots();
        }
        else
        {
            print_message("LOCURI INSUFICIENTE");
        }
    }
}

void car_leaving()
{
    if(!detect_car(TRIG_SENS1, ECHO_SENS1) && detect_car(TRIG_SENS2, ECHO_SENS2))
    {
        print_message("LA REVEDERE");
        open_gate();
        while(1)
        {
            if(!detect_car(TRIG_SENS1, ECHO_SENS1) && !detect_car(TRIG_SENS2, ECHO_SENS2))
            {
              break;
            }
            //
            // ------------------------
            // - tratare cazuri de deschidere sau alarma in caz de depasire numar masini & stuff
            //

        }
        close_gate();
        space++;
        print_remaining_spots();
    }
}

void setup() 
{
    pinMode(RLED, OUTPUT);
    pinMode(GLED, OUTPUT); //iesirea arduino intrarea de aprindere a becului

    pinMode(TRIG_SENS1, OUTPUT);
    pinMode(ECHO_SENS1, INPUT);   // intrarea senzorilor ultrasonici e un semnal de iesire al placii arduino
    pinMode(TRIG_SENS2, OUTPUT); // iar valoarea returnata este un semnal high daca se indeplineste conditia de distanta a senzorului
    pinMode(ECHO_SENS2, INPUT);

    myservo.attach(MOTOR); // pornirea motorasului
    myservo.write(0);

    Serial.begin(9600);

    lcd.begin(16, 2);     // APRINDERE ECRAN
    print_message("INITIALIZARE"); // AFISARE MESAJ INITIAL

    space = INITIAL_SPACE;
    
}

void loop() 
{
    car_access();
    delay(300);
    car_leaving();
}
