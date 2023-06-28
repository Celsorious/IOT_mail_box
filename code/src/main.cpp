///////////////// LIBRERIAS /////////////////
#include <Arduino.h>

// GMAIL //
#include <ESP_Mail_Client.h>

// WIFI //
#include <WiFi.h>

// SONIDO //
#include "XT_DAC_Audio.h"
#include <sonido.h>

////////////////// VARIABLES /////////////////

/*WIFI*/
#define WIFI_SSID ""
#define WIFI_PASSWORD ""

/*SMTP SERVER*/
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

/*GMAIL CREDENTIALS SENDER*/
#define AUTHOR_EMAIL ""
#define AUTHOR_PASSWORD ""

/*GMAIL CREDENTIALS RECIPIENT*/
#define RECIPIENT_EMAIL ""

unsigned long previo, actual;
unsigned long intervalo_mensaje = 5000;

///////////////// FUNCIONES /////////////////

//INTERRUPTION//
bool button_pressed = false;
const int button_pin = 21;

ICACHE_RAM_ATTR void if_button_is_pressed(){
  button_pressed = true;
}

//SMTP SESSION CONFIG//
SMTPSession smtp;

//SESSION CONFIG//
ESP_Mail_Session session;

void check_server_connection(){
    if (!smtp.connect(&session)){
    Serial.println("Imposible conectar con el servidor, reinicio del sistema.");
    ESP.restart();
  }
}

void session_configuration(){
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = AUTHOR_EMAIL;
  session.login.password = AUTHOR_PASSWORD;
}

//MESSAGE CONFIG//
SMTP_Message conection_message;

void conection_message_config(){
  conection_message.sender.name = F("Buzón");
  conection_message.sender.email = AUTHOR_EMAIL;
  conection_message.subject = F("Conexión con servidor realizada correctamente");
  conection_message.text.charSet = F("us-ascii");
  conection_message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
  conection_message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
  conection_message.addRecipient("", RECIPIENT_EMAIL);
}

SMTP_Message mail_reception_message;

void mail_reception_message_config(){
  mail_reception_message.sender.name = F("Buzón");
  mail_reception_message.sender.email = AUTHOR_EMAIL;
  mail_reception_message.subject = F("Correo en el buzón");
  mail_reception_message.text.charSet = F("us-ascii");
  mail_reception_message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
  mail_reception_message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
  mail_reception_message.addRecipient("", RECIPIENT_EMAIL);
}

void send_mail(SMTP_Message mensaje){
  int number_message = 0;
  bool email_status = MailClient.sendMail(&smtp, &mensaje);

  if (email_status == false){
    email_status = MailClient.sendMail(&smtp, &mensaje);
    number_message ++;
    if(number_message == 3){
      number_message = 0;
      check_server_connection();
    }
  }
}

//SOUND FILE CONFIG//
XT_Wav_Class Post_sound(play_sound);

//DAC CONFIG//
const int PIN_DAC = 25;
const int timer = 0;

XT_DAC_Audio_Class DAC_audio(PIN_DAC, timer);


void setup() {
  Serial.begin(115200);

  //INTERRUPCION//
  pinMode(button_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(button_pin), if_button_is_pressed, FALLING);

  //WIFI//
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(200);
  }

  // RECONNECT WIFI IF LOST CONNECTION //
  MailClient.networkReconnect(true);

  // SESSION CONFIG //
  session_configuration();

  // SERVER CONNECTION //
  check_server_connection();

  // SEND CONNECTION_OK_MESSAGE //
  conection_message_config();
  send_mail(conection_message);
  Serial.print("Mensaje enviado");

  // MAIL MESSAGE CONFIG //
  mail_reception_message_config();

}
 
void loop() {
  actual = millis();
  
  DAC_audio.FillBuffer();

  if (button_pressed && ((actual - previo) > intervalo_mensaje)){
    Serial.println("Mando mensaje");
    send_mail(mail_reception_message);


    if(Post_sound.Playing==false){     
      DAC_audio.Play(&Post_sound);
    }

    button_pressed = false;
    previo = actual;
  }
}
