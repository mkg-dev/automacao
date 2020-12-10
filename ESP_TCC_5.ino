
#include <DNSServer.h> //Local DNS Server used for redirecting all requests to the configuration portal ( https://github.com/zhouhan0126/DNSServer---esp32 )
#include <WebServer.h> //Local WebServer used to serve the configuration portal ( https://github.com/zhouhan0126/WebServer-esp32 )
#include <WiFiManager.h>   // WiFi Configuration Magic ( https://github.com/zhouhan0126/WIFIMANAGER-ESP32 ) >> https://github.com/tzapu/WiFiManager (ORIGINAL)
#include <WiFi.h>
#include <WiFiClientSecure.h>// so funciona com as 3 biliotecas juntas 
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "Base64.h"
#include "esp_camera.h"
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>
#include <MQTT.h> 


 

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26 
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22


WiFiClient net;
MQTTClient client;
WiFiManager wifiManager;


String imageFile = "";




void setup() ///>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
{
 
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
 
 delay(10);

 pinMode(2,INPUT_PULLUP);
 pinMode(4,OUTPUT);

 
//callback para quando entra em modo de configuração AP
  wifiManager.setAPCallback(configModeCallback); 
//callback para quando se conecta em uma rede, ou seja, quando passa a trabalhar em modo estação
  wifiManager.setSaveConfigCallback(saveConfigCallback); 

  wifiManager.autoConnect("ESP_AP","12345678"); //cria uma rede sem senha

  while (WiFi.status() != WL_CONNECTED) {
   //>> Serial.print("ops nao conectou...");
    delay(1000);
  } 
//..................................

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_VGA;  // UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA
  config.jpeg_quality = 10;
  config.fb_count = 1;
  
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
     //>> Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }
//.....................................                                                      


     client.begin("broker.mqttdashboard.com", net);
     client.onMessage(messageReceived);
     conectMQTT();

 Serial.flush();
 Serial.begin(115200);
}//setup



void loop() { /////<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

    
 client.loop();

   if(Serial.available()){
   
    
    String conteudo =  Serial.readString();
    delay(100);
    char orig = conteudo.charAt(0);
   
  if(orig == 'n'){
    conteudo.remove(0,1);
    client.publish("mk_nivel",conteudo);
  }
    if(orig == 't'){
    conteudo.remove(0,1);
    client.publish("mk_temp",conteudo);
    digitalWrite(4,HIGH);delay(2000);
    digitalWrite(4,LOW);
  }

  if(orig == 'c'){
    conteudo.remove(0,1);
    client.publish("mk_comed",conteudo);
  }
   
   }//serial
                 

  //se o botão foi pressionado
   if ( digitalRead(2) == LOW ) {
       //>> Serial.println("NOVA configuracao"); //tenta abrir o portal
      
      if(!wifiManager.startConfigPortal("ESP_AP", "12345678") ){
        //>>  Serial.println("Falha ao conectar no modo AP");
        delay(2000);
        ESP.restart();
        delay(1000);
      }
       //>> Serial.println("Conectado ao wifi");
   }


}
//......................WIFI MANANGER...................................


//callback que indica que o ESP entrou no modo AP
void configModeCallback (WiFiManager *myWiFiManager) {  
   //>> Serial.println("MODO AP");
   //>> Serial.println(WiFi.softAPIP()); //imprime o IP do AP
   //>> Serial.println(myWiFiManager->getConfigPortalSSID()); //imprime o SSID criado da rede

}

//callback que indica que salvamos uma nova rede para se conectar (modo estação)
void saveConfigCallback () {
   //>> Serial.println("Configuração de rede salva");
   //>> Serial.println("MODO ESTACAO");
   //>> Serial.println(WiFi.softAPIP()); //imprime o IP do AP
}





//...............................INICIO MQTT........................................



void messageReceived(String &topic, String &payload) {
  //Serial.println("RESPOSTA: " + topic + " - " + payload);//ENVIANDO ESTA FRASE
  
  if(payload=="captura"){
  saveCapturedImage();
  }
    Serial.print(payload);
        
       /* if(payload=="r5"){
         Serial.print("r5");
        delay(100);
        }*/  
}


void conectMQTT(){
     //>> Serial.print("\nconnecting no broker");
   
  while (!client.connect("mk_2020_BROKER", "user", "pass")) {
     //>> Serial.print(".");
    delay(1000);
  }
  
  client.subscribe("mk_recebe");
  client.subscribe("mk_racao");
  client.subscribe("mk_hora");
  client.subscribe("mk_estado");
  
    //>>Serial.println("\nCONECATDO NO BROKER");
}




//...............................FIM MQTT........................................


void saveCapturedImage() {
    //>> Serial.println("MTD  ENVIA");
   if (!client.connected()) 
      conectMQTT(); 
          
    
    camera_fb_t * fb = NULL;
    fb = esp_camera_fb_get();  
    if(!fb) {
       //>> Serial.println("Captura FALHOU reiniciando ESP...");
      delay(1000);
      ESP.restart();
      return;
    }
  
    char *input = (char *)fb->buf;
    char output[base64_enc_len(3)];
    int tam = 5000;//maximo 7000
    int parte = 0;
    int i =0;
 




    
  //***************************************************************
     
    
    for (i=0;i < fb->len;i++) {
  
      base64_encode(output, (input++), 3);
       if (i%3==0)
          imageFile += urlencode (String(output));     

            if(imageFile.length()>= 110){
            client.publish("mk_envia",imageFile);
           // delay(10);
             //>> Serial.print("\nFOTO ENVIADA  =>  ");Serial.println(imageFile.length());Serial.println(imageFile);
            imageFile="";
            }

    }//for tamanho maximo 116

           client.publish("mk_envia",imageFile);
         //  delay(10);
            //>> Serial.print("\nFINAL  =>  ");Serial.println(imageFile.length());Serial.println(imageFile);
           imageFile="";
       
              
           esp_camera_fb_return(fb);
   
}//metdodo

//***************************************************************


//https://github.com/zenmanenergy/ESP8266-Arduino-Examples/
String urlencode(String str)
{
    String encodedString="";
    char c;
    char code0;
    char code1;
    char code2;
    for (int i =0; i < str.length(); i++){
      c=str.charAt(i);
      if (c == ' '){
        encodedString+= '+';
      } else if (isalnum(c)){
        encodedString+=c;
      } else{
        code1=(c & 0xf)+'0';
        if ((c & 0xf) >9){
            code1=(c & 0xf) - 10 + 'A';
        }
        c=(c>>4)&0xf;
        code0=c+'0';
        if (c > 9){
            code0=c - 10 + 'A';
        }
        code2='\0';
        encodedString+='%';
        encodedString+=code0;
        encodedString+=code1;
        //encodedString+=code2;
      }
      yield();
    }
    return encodedString;
}
