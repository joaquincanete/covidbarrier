/* Termometro de personas V1.0
 * PICAIO SAS
 * reforma del programa por Joaquin Cañete el 21/04/2021 en Santa Fe Argentina
 * sensa temperatura, cuenta, activa bomba si la temp es normal y envia datos a la matriz led/
 */
//Librerias
#include <Wire.h> //comms
#include <Adafruit_MLX90614.h> //Termico
#include <LiquidCrystal_I2C.h> //Pantalla
#include <SR04.h> //Ultrasonido
#include <MD_Parola.h> //Matriz Led
#include <MD_MAX72xx.h> //Matriz Led
#include <SPI.h>
const uint16_t WAIT_TIME = 1000;
//Definicion de Pines
#define Echo 6 //Echo del Ultrasonido
#define Trig 5 //Trig del Ultrasonido
#define LP 9 //Salida LED
#define BOMBA 7 //Salida BOMBA

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW   //FC16_HW
//#define HARDWARE_TYPE MD_MAX72XX::PAROLA_HW   //PAROLA_HW
#define MAX_DEVICES 8
#define CLK_PIN   13
#define DATA_PIN  11
#define CS_PIN    10
//MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);


//Inicializar
Adafruit_MLX90614 mlx = Adafruit_MLX90614(); //Sensor Termico
//LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); // Pantalla LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);
SR04 sr04=SR04(Echo,Trig); //Ultrasonido

//Variables
int Espera1=300; //Espera en el loop

//Distancia
int Dist; //Distancia del ultrasonico
int DistMin=10; //Distancia minima para detectar al sujeto (mm)
int Presente=0; //Si hay alguien frente al Termometro
int Espera=500; //Tiempo de espera para verificar sujeto
unsigned long Tiempo=0; //Tiempo que lleva detectado para Millis
int Ahora=0; //Millis en el momento que se inicia

int counter = 0;
int currentState = 0;
int previousState = 0;


//Temperatura
float TempObj; //Temperatura del sujeto
int TempMax=38; //Temperatura maxima permitida
int TpoAlarma=3000; //Tiempo de Alarma por alta temperatura

void setup() 
{
  Serial.begin (9600);
  Wire.begin();
  pinMode(LP,OUTPUT); //LED
  pinMode(BOMBA,OUTPUT); //BOMBA
  pinMode(Trig,OUTPUT);
  pinMode(Echo,INPUT);
  mlx.begin(); //Termico

  Serial.begin(115200);
  //Sonido inicial
  //tone(12,NOTE_C5,3000);
  //digitalWrite(LP,HIGH);
  //delay(3000);
  //digitalWrite(LP,LOW);

//Mensaje inicial
  
  lcd.begin();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.setCursor(0,0);
  lcd.print("AMB: ");
  lcd.setCursor(0,1);
  lcd.print("PER: ");
  P.begin();
}

void loop() 
{
  //CONTADOR DE AFORO CON HC-SR04
 long duration, distance;
 digitalWrite(Trig, LOW); 
 delayMicroseconds(2); 
 digitalWrite(Trig, HIGH);
 delayMicroseconds(10); 
 digitalWrite(Trig, LOW);
 duration = pulseIn(Echo, HIGH);
 distance = (duration/2) / 29.1;
 if (distance <= 10){ 
 currentState = 1;
 }
 else {
 currentState = 0;
 }
 delay(1000);
 if(currentState != previousState){
if(currentState == 1){
counter = counter + 1;
Serial.println(counter);
    }
  }
//TERMINA CONTADOR DE AFORO

  //P.print("BARRERA DE TESTEO ABS");
//Distancia
Dist=sr04.Distance();
if(Dist>DistMin) //No hay nadie
{
  Presente=0;
  Tiempo=millis();
}
if(Dist<=DistMin && Presente==0) //Llego alguien, tomemos el tiempo
{
  Presente=1;
  Tiempo=millis();
}

if(Presente==1)
{
  if(millis()-Tiempo>Espera) //Se completo el tiempo
  {
    Presente=2;
  }
}

//Actualizar pantalla
lcd.setCursor(12,3);
//Temperaturas
lcd.setCursor(4,0);
lcd.print(mlx.readAmbientTempC());
lcd.setCursor(9,0);
lcd.print("c");
Serial.print("A");
Serial.println(mlx.readAmbientTempC());

TempObj=mlx.readObjectTempC();
switch(Presente)
{
  case 0: //No hay nadie
  lcd.setCursor(4,1);
  lcd.print("--.--c");
  lcd.setCursor(10,1);
  lcd.print(counter);
  Serial.println("BUSCANDO...  "); 
  Serial.println("B");// Se envia B para el dato
  break;

  case 1: //Llego alguien
  lcd.setCursor(4,2);
  lcd.print("--.--c");
  lcd.setCursor(4,1);
  lcd.print("MIDO...");
  Serial.println("LEYENDO...");
  //tone(12,NOTE_C5,TpoAlarma);
  break;

  case 2: //Se Completo el tiempo
  lcd.setCursor(4,1);
  lcd.print(TempObj);
  lcd.setCursor(9,1);
  lcd.print("c");
  Serial.print("T"); // temperatura medida
  Serial.println(TempObj);
  P.setTextAlignment(PA_CENTER);
  P.print (char&TempObj);
  if(TempObj>TempMax)
  {
    //tone(12,NOTE_G5,TpoAlarma);
   lcd.setCursor(10,1);
   lcd.print(" ALTA!");
   digitalWrite(LP,HIGH);
   delay(TpoAlarma);
   digitalWrite(LP,LOW);
   Serial.println("D"); // temperatura alta
  
  }
  else
  {
  lcd.setCursor(10,1);
  lcd.print(" NORMAL");
  digitalWrite(BOMBA,HIGH);
  delay(TpoAlarma);
  digitalWrite(BOMBA,LOW);
  
  }
  break;
}


delay(Espera1);

}
