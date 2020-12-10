#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>
#include "RTClib.h"
 
#define SS_PIN 10
#define RST_PIN 9
#define SENSOR 7
#define RELE 4
#define SERVO 8
#define SR_COMED 6


MFRC522 mfrc522(SS_PIN, RST_PIN);   
RTC_DS3231 rtc;
Servo myservo; 

int sim = 0;
int nao =0;
int numCtd = 0;
int tipoAlarme  = 0;
int qt = 0;


void setup() 
{
  Serial.begin(115200);   // Inicia a serial
  
  myservo.attach(SERVO);
  myservo.write(80); 
  
  SPI.begin();      // Inicia  SPI bus
  mfrc522.PCD_Init();   // Inicia MFRC522
  //Serial.println("Aproxime o seu cartao do leitor...");
  
  pinMode(SENSOR,INPUT);
  pinMode(RELE,OUTPUT);
  digitalWrite(RELE,HIGH);
 
  if (!rtc.begin()) {
    //Serial.println("RTC nao encontrado");
    Serial.flush();
    //abort();
  }
  
  if (rtc.lostPower()) {
   // Serial.println("RTC perdeu energia, defina a hora");
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }


}//fim setup


 
void loop() {//delay(1000);

    tipoAlarme = EEPROM.read(0);
    qt = EEPROM.read(1);
    DateTime now = rtc.now();
    
  if(tipoAlarme == 4){

      if( now.hour()== 9|| now.hour()== 12|| now.hour()== 16 || now.hour()== 20){ 
        if(now.minute()== 0){
          if(now.second()== 0|| now.second()== 1|| now.second()== 2){
              //Serial.println("ALARME 4 DISPARADO");
              alimentar(qt); 
          }
        }
      }
            
  }//tipo 4
    
    if(tipoAlarme == 3){

      if( now.hour()== 8|| now.hour()== 12|| now.hour()== 18){ 
        if(now.minute()== 0){
          if(now.second()== 0|| now.second()== 1|| now.second()== 2){
              //Serial.println("ALARME 3 DISPARADO");
              alimentar(qt); 
          }
        }
      }
              
  }//tipo 3

     if(tipoAlarme == 2){

      if( now.hour()== 7|| now.hour()== 18){ 
        if(now.minute()== 0){
          if(now.second()== 0|| now.second()== 1|| now.second()== 2){
             // Serial.println("ALARME 2 DISPARADO");
              alimentar(qt); 
          }
        }
      }
            
  }//tipo 2
  

    if (mfrc522.PICC_IsNewCardPresent()) {sim ++;}
        else{ nao ++; }

     if(nao >= 5){
       //Serial.println(">>>>FECHOU");
       myservo.write(80);
       delay(100); 
       nao = 0; sim = 0;
     }

     if(sim >= 1){
      // Serial.println(">>>>ABRIU");
       myservo.write(140); 
       delay(100);
       sim = 0; nao = 0;
     }
     
//--------------------COM SERIAL------------------------
   if(Serial.available()){
    
   String conteudo = Serial.readString();
   delay(100);

   
   char orig = conteudo.charAt(0);

 if(orig == 'x'){
      estadoComed();
      delay(1000);
      temperatura();
  }
  
 if(orig == 'r'){
     conteudo.remove(0,1);
     numCtd = conteudo.toInt();
        
     alimentar(numCtd);
   }

 if(orig == 'h'){
     String r = conteudo.substring(2);

     conteudo.remove(0,1);
     numCtd = conteudo.toInt();
     EEPROM.write(0,numCtd);
     
   //qtd racao
     r.remove(0,1);
     numCtd = r.toInt(); 
     EEPROM.write(1,numCtd);
    
    }
    
  }//fim serial
} //fim loop

//------------------------FUNÇOES-------------------
void alimentar(int q){
    digitalWrite(RELE,LOW);                 
    delay(q*1000);//tempo ativo
    digitalWrite(RELE,HIGH); 
   
    nivel(); 
    Serial.flush();
}

void nivel(){
   int n = digitalRead(SENSOR);
   delay(50);
   String nv = "n"+String(n);
   Serial.print(nv); 
   Serial.flush(); 
}
void temperatura(){
   int tmpSens = rtc.getTemperature();
   String tmp = "t"+String(tmpSens);
   Serial.print(tmp);
   Serial.flush();
}


void  estadoComed(){
   int srC = digitalRead(SR_COMED);
   delay(50);
   String sc = "c"+String(srC);
   Serial.print(sc); 
   Serial.flush(); 
}
 
