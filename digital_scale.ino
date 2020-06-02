
#include <HX711.h>
#include <SPI.h>
#include <SD.h>

#include <LiquidCrystal_I2C.h>
#define HX711_DOUT  8
#define HX711_CLK  9
#define calibration_factor 1070
#define GREEN_LED 11
#define RED_LED 10
#define BUTTON 12
#define UNIT_CHANGE_TIME 5000
#define TIME_TO_TARE 2000
#define KG 1000
#define G 1
#define DG 10
#define LOG_LIMIT 10
#define LIMIT 100

#include <Wire.h> 
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2);
HX711 scale;

static float limit = LIMIT; //w g 
static long button_start_hold_time = 0;
static long button_push_time = 0;
static int current_unit = G;
static bool is_sd_card = false;
static String log_buffer = "";
static unsigned int measurements = 0;
static long start_time = millis();
static unsigned int log_limit = LOG_LIMIT;
static unsigned int time_to_tare = TIME_TO_TARE;
static unsigned int unit_change_time = UNIT_CHANGE_TIME;

File config;
File logs;
const int chipSelect = SS;

static void config_set(Stream& line){
  String option = line.readStringUntil('=');
  if(option == "LIMIT") limit = line.readStringUntil('\n').toInt();
  else if(option == "UNIT_CHANGE_TIME") unit_change_time = line.readStringUntil('\n').toInt();
  else if(option == "TIME_TO_TARE") time_to_tare = line.readStringUntil('\n').toInt();
  else if(option == "LOG_LIMIT") log_limit = line.readStringUntil('\n').toInt();
  else option += "="+line.readStringUntil('\n')+" (unrecognized)";
  Serial.println(option);
}

void setup() {
  Serial.begin(9600);

  pinMode(BUTTON, INPUT);
  scale.begin(HX711_DOUT, HX711_CLK);
  scale.set_scale(calibration_factor);
  scale.tare();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Czekaj... "); //wait

  delay(2000);
  scale.tare();

  lcd.setCursor(0,0);
  lcd.print("Odczyt:  "); //read
  lcd.setCursor(14, 1);
  lcd.print("g ");
  lcd.setCursor(0, 1);


  if(!SD.begin(chipSelect)){
    Serial.println("There is a problem with SD card initialization");
  }
  else{
    Serial.println("Initialization of SD CARD - OK");
    is_sd_card = true;
  
  
    config = SD.open("config.txt");
    if (!config) {
      Serial.println("Config file not found");
    }
    else{
      Serial.println("Reading config file: ");
        while(config.available()){
          config_set(config);
        }
        config.close();
    }
  }

}
 
void loop() {
  
  if (digitalRead(BUTTON) != HIGH) {
    button_start_hold_time = millis();
    if (button_push_time >= time_to_tare && button_push_time < unit_change_time) scale.tare();
    button_push_time = 0;
  }
  
  button_push_time = millis() - button_start_hold_time;
  delay(1000);
  lcd.setCursor(0, 1); 
  lcd.print("         ");
  lcd.setCursor(0, 1);
  lcd.print((float)((int)scale.get_units()) / current_unit);
  Serial.print("!");
  Serial.println((float)((int)scale.get_units()) / current_unit);

  if(button_push_time > unit_change_time){
    lcd.setCursor(14, 1);
    switch (current_unit){
      case G:
        current_unit = DG;
        lcd.print("dg");
        break;
      case DG:
        current_unit = KG;
        lcd.print("kg");
        break;
      case KG:
        current_unit = G;
        lcd.print("g ");
        break;
    }
    button_start_hold_time = millis();
  }

  if(scale.get_units() > limit){
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
  }
  else if(scale.get_units() > 2 && (abs(scale.get_units() - scale.get_units())<1)){
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
    measurements += 1;
    logs = SD.open("datalog.txt", FILE_WRITE);
    if (logs && LOG_LIMIT > 0){
      if (measurements > log_limit){
        log_buffer += "\nTime: " + String(((millis()-start_time)/1000))+"\n";
        logs.println(log_buffer);
        measurements = 0;
        log_buffer = "";
      }
      else {
        log_buffer += "\n";
        log_buffer += String((int)scale.get_units());
      }
      logs.close();
    }
  }
  else{
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
  }
  
  if(Serial.available())
  {
    config_set(Serial);
  }
  
}
