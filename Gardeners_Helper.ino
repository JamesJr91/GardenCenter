#include <DallasTemperature.h>
#include <OneWire.h>
#include <SoftwareSerial.h>
#include <Chrono.h>
#include <math.h>

//Delcare constant pin number variables
const int tempSensorA_Pin = 4;
const int tempSensorB_Pin = 5;
const int relayA_Pin = 10;
const int relayB_Pin = 11;
const int modeSelector_Pin = 12;
const int timerReset_Pin = 13;
const int idealTemp = 78;

int mode;
int modeSelectorState;

int tmpA;
int tmpB;

Chrono timer(Chrono::SECONDS);
unsigned long hours;
unsigned long minutes;
unsigned long seconds;
unsigned long timeInSeconds;

//Configure relays to start in the off position
bool relayA = false;
bool relayB = false;
bool lightOn = false;

//Create a placeholder for the timer/temp reading
char tmpAString[10];
char tmpBString[10];
char hourString[10];
char minuteString[10];
char secondString[10];

//Setup LCD rx,tx
SoftwareSerial sLCD(3,2);

//Temp sensor i/o
OneWire tempSensorABus(tempSensorA_Pin);
OneWire tempSensorBBus(tempSensorB_Pin);

DallasTemperature tempSensorA(&tempSensorABus);
DallasTemperature tempSensorB(&tempSensorBBus);

void setup() { 

  //Setup a serial terminal for debug use
  Serial.begin(9600);
  Serial.println("Serial Port Connected");

  //Setup user inputs
  Serial.println("Configuring user inputs");
  pinMode(modeSelector_Pin, INPUT);
  pinMode(timerReset_Pin, INPUT);
  int modeSelectorState = 0;
  int timerResetState = 0;
  Serial.println("User inputs configured");

  //Setup relays
  Serial.println("Configuring relay outputs");
  pinMode(relayA_Pin, OUTPUT);
  pinMode(relayB_Pin, OUTPUT);
  Serial.println("Relay outputs configured");

  //Setup temp sensors
  tempSensorA.begin();
  tempSensorB.begin();

  //Setup for the LCD
  Serial.println("Setting up LCD");
  sLCD.begin(9600);
  delay(500);
  moveCursor();
  clearDisplay();
  sLCD.write("    Tootie's    ");
  selectLineTwo();
  sLCD.write(" Garden Center  ");
  delay(3000);
  clearDisplay();
  Serial.println("LCD setup complete");
}

void loop() {
  
  //Configure switch and button
  modeSelectorState = digitalRead(modeSelector_Pin); // switch -> 0 = Heat Pad Mode, 1 = Light Mode

  switch (modeSelectorState) {

    // 0 will be Heating Pad Mode
    case 0:
      Serial.println("Heating Pad Mode");
      heaterMode();
      updateHeaterDisplay();
      break;

    // 1 will be Lighting Mode
    case 1:
      Serial.println("Lighting Mode");
      if (mode == 0) {
        if(timer.isRunning()) {
          timer.restart();
        }
        mode = 1;
        lightOn = true;
      }
      if(lightOn) {
        lightingMode(57600); //16 hour format
        updateLightDisplay();
      }
      if(!lightOn) {
        lightingMode(28800); //8 hour format
        updateLightDisplay();
      }
      break;
  }

  delay(500);
}

void clearDisplay() {
  sLCD.write(254);
  sLCD.write(1);
}

void moveCursor() {
  sLCD.write(254);
  sLCD.write(128);
}

void selectLineTwo()
{ 
  //puts the cursor at line 0 char 0.
  sLCD.write(0xFE); //command flag
  sLCD.write(192); //position
}

void heaterMode() {

  modeSelectorState = 0;
  mode = 0;

  //request temperature on bus
  tempSensorA.requestTemperatures();
  tempSensorB.requestTemperatures();
  int celsiusTmpA = tempSensorA.getTempCByIndex(0);
  int celsiusTmpB = tempSensorB.getTempCByIndex(0);
  tmpA = (celsiusTmpA * 1.8) + 32;
  tmpB = (celsiusTmpB * 1.8) + 32;

  //Print sensor data to serial and to LCD
  sLCD.write(tmpA);
  sLCD.write(tmpB);
  moveCursor();

  //Turn heating pad on or off
  if (tmpA < idealTemp){
    //turn on heater
    if (!relayA) {
      digitalWrite(relayA_Pin, HIGH);
    }
    else{
      digitalWrite(relayA_Pin, HIGH);
    }
    relayA = true;
    Serial.println("Relay A is on");
  }
  if (tmpA >= idealTemp) {
    //turn off heater
    if (!relayA) {
      digitalWrite(relayA_Pin, LOW);
    }
    else{
      digitalWrite(relayA_Pin, LOW);
    }
    relayA = false;
    Serial.println("Relay A is off");
  }

  if (tmpB < idealTemp){
    //turn on heater
    if (!relayB) {
      digitalWrite(relayB_Pin, HIGH);
    }
    else{
      digitalWrite(relayB_Pin, HIGH);
    }
    relayB = true;
    Serial.println("Relay B is on");
  }
  if (tmpB >= idealTemp) {
    //turn off heater
    if (!relayB) {
      digitalWrite(relayB_Pin, LOW);
    }
    else{
      digitalWrite(relayB_Pin, LOW);
    }
    relayB = false;
    Serial.println("Relay B is off");
  }
}

void updateHeaterDisplay() {
  //Setup serial port output
  Serial.print("Temp A: ");
  Serial.print(tmpA);
  Serial.print(" Temp B: ");
  Serial.println(tmpB);
  Serial.println("");

  //Setup LCD
  clearDisplay();
  moveCursor();
  sprintf(tmpAString,"%4d",tmpA); // right-justify to 4 spaces
  sprintf(tmpBString,"%4d",tmpB); // right-justify to 4 spaces
  sLCD.write("Temp A: ");
  sLCD.write(tmpAString);
  sLCD.write("    ");
  selectLineTwo();
  sLCD.write("Temp B: ");
  sLCD.write(tmpBString);
  delay(1000);
}

void lightingMode(int duration) {
  modeSelectorState = 1;
  mode = 1;


  //Check if timer reset button was pressed
  if (digitalRead(timerReset_Pin)) {
    timer.restart();
  }
  
  Serial.print("timer has passed?: ");
  Serial.println(timer.hasPassed(duration));
  if (timer.hasPassed(duration)) {
    if (lightOn) {
      digitalWrite(relayA_Pin, LOW);
      digitalWrite(relayB_Pin, LOW);
      timer.restart(); 
      lightOn = false;
    }
    if (!lightOn) {
      digitalWrite(relayA_Pin, HIGH);
      digitalWrite(relayB_Pin, HIGH);
      timer.restart();
      lightOn = true;
    }
  }

    if (!timer.hasPassed(duration)) {
    if (lightOn) {
      digitalWrite(relayA_Pin, HIGH);
      digitalWrite(relayB_Pin, HIGH);
      lightOn = true;
    }
    if (!lightOn) {
      digitalWrite(relayA_Pin, LOW);
      digitalWrite(relayB_Pin, LOW);
      lightOn = false;
    }
  }
}

void updateLightDisplay() {
  clearDisplay();
  moveCursor();

  if (timer.elapsed() < 60) {
    seconds = timer.elapsed();
    }
  if (timer.elapsed() >= 60) {
    seconds = fmod(timer.elapsed(), 60);
  }
  if (seconds >= 59) {
    minutes++;
  }
  if (minutes >= 59) {
    hours++;
    minutes = 0;
  }

  //Setup serial port output
  Serial.print("Hours: ");
  Serial.print(hours);
  Serial.print(" Minutes: ");
  Serial.print(minutes);
  Serial.print(" Seconds: ");
  Serial.print(seconds);
  Serial.print(" Elapsed Time: ");
  Serial.println(timer.elapsed());

  //Setup lcd output
  sprintf(hourString,"%4d",hours); // right-justify to 4 spaces
  sprintf(minuteString,"%4d",minutes); // right-justify to 4 spaces
  sprintf(secondString,"%4d",seconds); // right-justify to 4 spaces
  sLCD.write("Time Since Start");
  selectLineTwo();
  sLCD.write(hourString);
  sLCD.write(":");
  sLCD.write(minuteString);
  sLCD.write(":");
  sLCD.write(secondString);
  delay(500);
}
