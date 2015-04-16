/*
lumiere_pin = 6;
eau_pin = 7;
moisture_sensor = A2;
rtc pin 2 sda et 3 clk 
ph A0 ph_ref A1
lux pin 2 sda et 3 clk
*/
#include <Bridge.h>
#include <Process.h>
#include <Wire.h>
#include "RTClib.h"
#include <dht11.h>
#define DHT11_PIN 8
//#include <Digital_Light_TSL2561.h>

#define PH_GAIN_STAGE1_PH      4.8262 
#define PH_GAIN_STAGE2_REF      2.0
#define CLE_ARDUINO "" //Ajouté ICI la cle ARDUINO //Add here your ARDUINO key. 
#define CLE_PROWL "" //Ajouté ICI la cle PROWL si vous en avez une. //Add here your PROWL key if you have one.

dht11 DHT;
RTC_DS1307 RTC;

bool prowl = true ; 
int twitter = 0 ;

//int ppfd;
//float ph;
String stemperature ;
String humidite ;
int Moisture =0;
String probleme ; 
char dht_char[1];
int dht_state=3;
int secheresse =0;

DateTime now ;
DateTime future ; 
DateTime dailyreport;

int pass = 0;
int lock = 0;

int etape = 0;

void setup() {
  // Bridge takes about two seconds to start up
  // it can be helpful to use the on-board LED
  // as an indicator for when it has initialized
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  Bridge.begin();
  digitalWrite(13, HIGH);
  //fin initialisation bridge 
  //inialisation des capteur et des relay 
  pinMode(7,OUTPUT);//eau
  pinMode(6,OUTPUT);//lumiere
  pinMode(A2,INPUT);//moisture
  Wire.begin();
  RTC.begin();
  if (! RTC.isrunning()) {
     Serial.println("RTC is NOT running!");
     // following line sets the RTC to the date & time this sketch was compiled
    // RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  
  //TSL2561.init(); //====================> LUX 
  
  now = RTC.now();
  
  future = DateTime(now.unixtime() + 3200L) ;
  dailyreport = DateTime(1389078000L);
  Serial.begin(9600);
  if(now.day() > dailyreport.day()){
      dailyreport = DateTime(now.unixtime()+86400L);
  }
  delay(1000);
  while (!Serial); // wait for a serial connection
  Serial.println("DEBUT ICI");
  secretgarden();
}

void loop() {
  String test;
  get_cycle();
  test=roue_url();
  Serial.println(test);
  if(test.startsWith("http://votre-secret-garden.fr")){
    marlert(test);
  }
  else{
    if(test.startsWith("wait")){
      Serial.println(" ");
    }
    else{
      to_cloud(test);
     }
    
  }
  delay(5000);
}

void marlert(String url){
  Process secretgarden;
  secretgarden.begin("curl");
  secretgarden.addParameter(url);
  secretgarden.run();
  
  while (secretgarden.available() > 0) {
    char c = secretgarden.read();
    Serial.write(c);
  } 
}

void to_cloud(String url){
  Process secretgarden;
  secretgarden.begin("curl");
  secretgarden.addParameter("--data");
  secretgarden.addParameter(url);
  secretgarden.addParameter("http://votre-secret-garden.fr/arduino.php");
  secretgarden.run();
  
  while (secretgarden.available() > 0) {
    char c = secretgarden.read();
    Serial.write(c);
  }
}

void get_cycle(){
  String Url,rep;
  rep="";
  
  int heure_l=0,minute_l=0,heure_e=0,minute_e=0,heure_fin=0,minute_fin=0,fin_h=0,fin_m=0,securite=0;
  
  Url="http://votre-secret-garden.fr";
  Url+="/xml/action_";
  Url+=CLE_ARDUINO;
  Url+=".xml";
 
  
  Process secretgarden;
  secretgarden.begin("curl");
  secretgarden.addParameter(Url);
  secretgarden.run();
  
  Serial.println(Url);
  while (secretgarden.available()>0) {
      char c = secretgarden.read();
        if(c == '\n'){
          if(rep.startsWith("    <heure_l>")) {
              rep.replace("<heure_l>", "");
              rep.replace("</heure_l>", "");
              heure_l=rep.toInt();
          }
          else if(rep.startsWith("    <min_l>")) {
            rep.replace("<min_l>", "");
            rep.replace("</min_l>", "");
            minute_l=rep.toInt();
          }
          else if(rep.startsWith("    <heure_fin>")) {
            rep.replace("<heure_fin>", ""); 
            rep.replace("</heure_fin>", "");
            heure_fin=rep.toInt();
          }
          else if(rep.startsWith("    <minute_fin>")) {
            rep.replace("<minute_fin>", "");
            rep.replace("</minute_fin>", "");
            minute_fin=rep.toInt();
          }
          else if(rep.startsWith("    <heure_e>")) {
            rep.replace("<heure_e>", ""); 
            rep.replace("</heure_e>", "");
            heure_e=rep.toInt();
          }
          else if(rep.startsWith("    <minute_e>")) {
            rep.replace("<minute_e>", "");
            rep.replace("</minute_e>", "");
            minute_e=rep.toInt();
          }
          else if(rep.startsWith("    <fin_h>")) {
            rep.replace("<fin_h>", "");  
            rep.replace("</fin_h>", "");  
            fin_h=rep.toInt();
          }
          else if(rep.startsWith("    <fin_m>")) {
            rep.replace("<fin_m>", "");  
            rep.replace("</fin_m>", "");  
            fin_m=rep.toInt();
          }
          else if(rep.startsWith("  <securite>")) {
            rep.replace("<securite>", "");   
            rep.replace("</securite>", "");  
            securite=rep.toInt();
          }
          else if(rep.startsWith("  <twitter>")) {
            rep.replace("<twitter>", "");   
            rep.replace("</twitter>", "");  
            twitter=rep.toInt();
          }
          rep="";
        }
        else {
          rep+=c;
        } 
     
  }
  Serial.println("now");
  Serial.print("le ");
  Serial.print(now.day(), DEC);
  Serial.print("/");
  Serial.print(now.month(), DEC);
  Serial.print("/");
  Serial.print(now.year(), DEC);
  Serial.print("  "); 
  Serial.print(now.hour(), DEC);
  Serial.print(" h ");
  Serial.println(now.minute(), DEC);
  cycle_allumage(heure_l,minute_l,heure_fin,minute_fin,securite,6);//lumiere
  cycle_allumage(heure_e,minute_e,fin_h,fin_m,securite,7);// eau 
  delay(5000);
}

String roue_url(){
  now = RTC.now();
  String url="";
  
  if(etape == 0){
    pass=0;
    etape=1;
    url="temp=";
    url+=stemperature;
    url+="&humi=";
    url+=humidite;
    url+="&moist=";
    url+=Moisture;
    /*url+="&ph="; //si vous avez une sonde pH décomenté ici ; 
    url+=ph;*/
    /*url+="&ppfd="; // si vous avez un capteur de luminosité decomenté ici ;
    url+=ppfd;*/
    url+="&pb=";
    url+=probleme;
    url+="&cle=";
    url+=CLE_ARDUINO;
    return url ;      
  }
  else if ((now.unixtime() > future.unixtime()) && etape == 1){
    secretgarden();
    now = RTC.now();
    future = DateTime(now.unixtime() + 3200L);
    url="temp=";
    url+=stemperature;
    url+="&humi=";
    url+=humidite;
    url+="&moist=";
    url+=Moisture;
    /*url+="&ph="; //si vous avez une sonde pH décomenté ici ; 
    url+=ph;*/
    /*url+="&ppfd="; // si vous avez un capteur de luminosité decomenté ici ;
    url+=ppfd;*/
    url+="&pb=";
    url+=probleme;
    url+="&cle=";
    url+=CLE_ARDUINO;
    return url ;
  }
  else if ((now.day() == dailyreport.day()) && (now.hour() == dailyreport.hour()) && pass == 0 && twitter == 1){
    pass=1;
    dailyreport = DateTime(dailyreport.unixtime()+ 86400L);
    return "http://votre-secret-garden.fr/tweet/tweet.php";
  }
  else if(secheresse == 2 && prowl ){
    url="http://votre-secret-garden.fr/prowl/alert.php?message=2&id=";
    url+=CLE_PROWL;
    return url;
  }
  else if(DHT.temperature > 40 && lock == 0 && prowl ){
    lock=1;
    url="http://votre-secret-garden.fr/prowl/alert.php?message=0&id=";
    url+=CLE_PROWL;
    return url;
  }
  else{
    return "wait";
  }  
}

void miseenRoute(int test,int appareil,bool inversion){
  if(test == 1){
    if(inversion == true){
      digitalWrite(appareil,LOW);
    }
    else{
      digitalWrite(appareil,HIGH);
    }
  }
  else if(test == 0){
    if(inversion == true){
      digitalWrite(appareil,HIGH);
    }
    else{
      digitalWrite(appareil,LOW);
    }
  }
}

void cycle_allumage(int heure_debut,int minute_debut,int heure_fin,int minute_fin,int securite,int appareil){
  
  now=RTC.now();
  if(appareil == 6 ){
    Serial.println("lumiere");
  }
  else if(appareil == 7){
    Serial.println("eau");
  }
  Serial.print(heure_debut);
  Serial.print(" h ");
  Serial.print(minute_debut);
  Serial.print(" =====> ");
  Serial.print(heure_fin);
  Serial.print(" h ");
  Serial.println(minute_fin);
  
  int tmp;
  bool inversion=false;
  
  if ((securite == 1 || securite == 2 ) && DHT.temperature > 40){
    miseenRoute(0,appareil,false);
  }
  else if((securite == 2) && (secheresse == 2)){
    miseenRoute(0,7,false);//securite eau
  }
  else {
    if(heure_debut>heure_fin){
      tmp=heure_debut;
      heure_debut=heure_fin;
      heure_fin=tmp;
      inversion=true;
    }
    if((heure_debut == heure_fin) && (now.hour() == heure_debut)){
      if((now.minute() >= minute_debut) && (now.minute() < minute_fin)){
        miseenRoute(1,appareil,inversion);
      }
      else{
        miseenRoute(0,appareil,inversion);
      }
    }
    else{
      if((now.hour() > heure_debut) && (now.hour() < heure_fin)){
        miseenRoute(1,appareil,inversion);
      }
      else if(now.hour() == heure_debut){
        if(now.minute() >= minute_debut){
          miseenRoute(1,appareil,inversion);
        }
        else {
          miseenRoute(0,appareil,inversion);
        }
      }
      else if(now.hour() == heure_fin){
        if(now.minute() >= minute_fin){
          miseenRoute(0,appareil,inversion);
        }
        else {
          miseenRoute(1,appareil,inversion);
        }
      }
      else{
        miseenRoute(0,appareil,inversion);
      }
    }
  }
}


void temperature(void){
  int chk;
  chk = DHT.read(DHT11_PIN);    // READ DATA
  switch (chk){
    case DHTLIB_OK:  
               dht_state=0;
                break;
    case DHTLIB_ERROR_CHECKSUM: 
                dht_state=1;
                dht_char[0]='1';
                break;
    case DHTLIB_ERROR_TIMEOUT: 
                dht_state=2;
                dht_char[0]='2';
                break;
    default: 
                dht_state=3;
                dht_char[0]='3';
                break;
  }
  delay(500);
}

void secretgarden(void) {
  //ppfd = light();
  //ph = Ph();
  Serial.println("SG");
  temperature();
  Moisture = analogRead(A2);//moisture sensor 
  if(Moisture>700 && Moisture < 800){
    /*terre normal*/
    secheresse = 0;   
  }
  else if(Moisture<700){
    probleme= "tes_plantes_ont_soif";
    secheresse += 1;
    if(secheresse >2 ){
      secheresse = 2;
    }  
  }
  else if(Moisture>800){
     probleme="je_me_noie";  
  }
  if(dht_state == 0){
  probleme= "ok" ; 
  } 
  else if(dht_state == 1){
  probleme= "err_temperature_1" ; 
  } 
  else if(dht_state == 2){
    probleme= "err_temperature_2" ; 
  } 
  else if(dht_state == 3){
  probleme= "err_temperature_3" ; 
  } 
  if(DHT.temperature>40){
    probleme="il_fait_trop_chaud"; 
  }
  else{
    lock=0;    
  }
  stemperature=(String)DHT.temperature;
  humidite = (String) DHT.humidity;
}
/*
int Light(){ 
   unsigned long  lux;
   int ppfd;
   TSL2561.getLux();
   lux = TSL2561.calculateLux(0,0,1);
   ppfd = lux / 60 ;
   return ppfd ;
}
float Ph(){
  int val_E;
  int val_R;
  int T ;
  float E;
  float R;
  float pH;
  T= DHT.temperature;
  
  val_R = readADC(A1, 16666);
  R = PH_GAIN_STAGE2_REF * (val_R * 5.0 / 1024);
  
  val_E = readADC(A0, 16666);
  E = -1 * ((val_E * 5.0 / 1024) - R) / PH_GAIN_STAGE1_PH;
  
  pH = (float)((-5038.8 * E / (273.15 + T)) + 7.0);
  return pH;
}
unsigned int readADC(unsigned char channel, unsigned int reading_time) {

  double d;
  int i;
  long t0_us;

  d = 0.0;
  i = 0;
  t0_us = micros();
  while((micros()-t0_us)<reading_time){
    i++;
    d += analogRead(channel);
  }
  d /= i;

  return (unsigned int)(d);
}
*/

