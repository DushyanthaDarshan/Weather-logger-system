#include <dht.h>    //Library for DHT11 temperature and humidity sensor

#include <Wire.h>    //Library for the I2C communication

#include <LiquidCrystal_I2C.h>    //Library for the lcd display

#define dhtPin A1   //Analog pin define as a constant for temperature and humidity sensor

dht DHT;    //dht define as a DHT for temperature and humidity sensor

LiquidCrystal_I2C lcd(0x3F, 16, 2);

#include <SPI.h>
#include <SD.h>

/*@author: Dushyantha Darshan Rubasinghe
  @Date: 2020-08-26
  @gmail: dushyantha1208@gmail.com
  */

const int modePin = 9;    //declare pin no 9 for pushbutton for select reset when auto selected
const int manualPin = 8;    //declare pin no 8 for pushbutton for select manual mode
const int selectSensorPin = 7;    //declare pin no 7 for pushbutton for select sensor

//variables
float humidity;
float temp;
String rainStatus;
float windSpeed;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ;
  }
  Serial.println("Weather Logger System\n\n");
  
  lcd.init();
  lcd.backlight();
  delay(500);

  if (!SD.begin(10)) {
    lcd.print("SD not found");
    while (1);
  }
  Serial.println("initialization done.");

  pinMode(modePin, INPUT);   // declare pushbutton as input for reset
  pinMode(manualPin, INPUT);   // declare pushbutton as input for manual
  pinMode(selectSensorPin, INPUT);   // declare pushbutton as input for change sensor

  lcd.print("Welcome");
  delay(1000);
  lcd.clear();
  lcd.print("Default: Auto");
  delay(1000);
  lcd.clear();
  lcd.print("Change mode: ");
  lcd.setCursor(0,1);
  lcd.print("Press button 1");
}

void loop() {
  manually();
  autoControl();
}

void autoControl() {
  bool isAuto = true;
  while(isAuto==true) {
    temperature();
    humidit();
    rainDetect();
    wind();
    delay(1000);
    int modeValue = digitalRead(modePin);    //read input value
    if(modeValue == 0) {
      isAuto = false;
      lcd.print("Reseting");
      delay(1000);
      lcd.clear();
      break;
    }
  }
}

void manually() {
  int gTime = 0;
  int manualValue = 1;
  while(gTime<=3000) {
    gTime = gTime + 100;
    delay(100);
    manualValue = digitalRead(manualPin);    //read input value
    if(manualValue==0) {
      break;
    }
  }
  
  if(manualValue==0) {
    int modeValue=1;
    int sensorChange = 1;
    while(modeValue==1) {
      temperature();
      while(sensorChange == 1) {
        sensorChange = digitalRead(selectSensorPin);    //read input value
      }
      sensorChange = 1;
      humidit();
      while(sensorChange == 1) {
        sensorChange = digitalRead(selectSensorPin);    //read input value
      }
      sensorChange = 1;
      rainDetect();
      while(sensorChange == 1) {
        sensorChange = digitalRead(selectSensorPin);    //read input value
      }
      sensorChange = 1;
      wind();
      while(sensorChange == 1) {
        sensorChange = digitalRead(selectSensorPin);    //read input value
      }
      int dTime = 0;
      sensorChange = 1;
      while(dTime<=2000) {
        dTime = dTime + 100;
        delay(100);
        modeValue = digitalRead(modePin);    //read input value
        if(modeValue==0) {
          break;
        }
      }
    }
  }
}

void temperature() {
  temp = 0;
  DHT.read11(dhtPin);
  temp = DHT.temperature;

  //Display temperature
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Temperature: ");
  lcd.setCursor(0,1);
  lcd.print(temp);
  lcd.print("C");
  saveTemp();
  delay(2000);
  lcd.clear();
}

void humidit() {
  humidity = 0;
  DHT.read11(dhtPin);
  humidity = DHT.humidity;

  //Display humidity
  lcd.setCursor(0,0);
  lcd.print("Humidity: ");
  lcd.setCursor(0,1);
  lcd.print(humidity);
  lcd.print("%");
  saveHumidity();
  delay(2000);
  lcd.clear();
}

void rainDetect() {
  int rainReading = analogRead(A2);
  if(rainReading<100) {
    rainStatus = "Raining";
    lcd.print("Raining");
  }else {
    rainStatus = "Not raining";
    lcd.print("Not raining");
  }
  saveRain();
  delay(1000);
  lcd.clear();
}

void wind() {
  //variables and initial values
  int countTime = 0;    //count 5 seconds
  int windReading;    //analog reading of sensor
  float usedTime;   //used time accurately for the calculation
  float omega;
  int windRounds = 0;
  unsigned long startTime = millis();   //starting time of the round count
  unsigned long initialTime = millis();   //help to count 5 seconds
  float windSpeed = 0;

  while(countTime < 10) {
    windReading = analogRead(A0);
    Serial.println(windReading);
    if(windReading > 600) {
      windRounds = windRounds + 1;
    }
    
    if((millis()-initialTime) >= 1000) {
      countTime = countTime + 1;
      initialTime = millis();
    }  
  }

  if(windRounds == 0 || windRounds == 1) {
    windSpeed = 0;

  } else {
    usedTime = (millis() - startTime)*0.001;
    omega = ((2 * 3.141593 * (windRounds - 1))/usedTime);
    windSpeed = 0.04*omega*3.6;    //six after the omega is for convert to minute
  }

  lcd.setCursor(0,0);
  lcd.print("Wind Speed: ");
  lcd.setCursor(0,1);
  lcd.print(windSpeed);
  lcd.print("km/h");
  saveWind();
  
  delay(5000);
  lcd.clear();
}

void saveTemp() {
  //write to the temperature file
  File myFile = SD.open("temp.txt", FILE_WRITE);

  if(myFile) {
    Serial.print("Writing to temperature.txt...");
    myFile.println(temp);
    myFile.close();
    Serial.println("done.");
  } else {
    Serial.println("error opening temp.txt");
  }
}

void saveHumidity() {
  //write to the humidity file
  File myFile = SD.open("humidity.txt", FILE_WRITE);

  if(myFile) {
    Serial.print("Writing to humidity.txt...");
    myFile.println(humidity);
    myFile.close();
    Serial.println("done.");
  } else {
    Serial.println("error opening humidity.txt");
  }
}

void saveRain() {
  //write to the rain file
  File myFile = SD.open("rain.txt", FILE_WRITE);

  if(myFile) {
    Serial.print("Writing to rain.txt...");
    myFile.println(rainStatus);
    myFile.close();
    Serial.println("done.");
  } else {
    Serial.println("error opening rain.txt");
  }
}

void saveWind() {
  //write to the wind file
  File myFile = SD.open("wind.txt", FILE_WRITE);

  if(myFile) {
    Serial.print("Writing to wind.txt...");
    myFile.println(windSpeed);
    myFile.close();
    Serial.println("done.");
  } else {
    Serial.println("error opening wind.txt");
  }
}
