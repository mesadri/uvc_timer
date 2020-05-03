//#define CONFIG_EEPROM

#include <EEPROM.h>
#include <TimerOne.h>
#include <Wire.h>
#include <MultiFuncShield.h>

#define LIFE_OF_UV_TUBE 360000  //6000 hour
#define TIMER_DEFUALT 10
#define RELAY_PIN 9

void readEEPROM();
void clearEEPROM();
void writeEEPROM();

int address = 0;      //EEPROM address counter
int timerValue;       //countdown timer Value in miniute


//Values that get stored in EEPROM
struct config_t
{
    unsigned long int life_timer_miniute;
    char lastTimerVal;
    unsigned long int total_shut;
} configuration;

/*

For more information and help, please visit https://www.cohesivecomputing.co.uk/hackatronics/arduino-multi-function-shield/part-3/

All our hackatronics projects are free for personal use, and there are many more
in the pipeline. If you find our projects helpful or useful, please consider making
a small donation to our hackatronics fund using the donate buttons on our web pages.
Thank you.

*/

enum CountDownModeValues
{
  COUNTING_STOPPED,
  COUNTING
};

byte countDownMode = COUNTING_STOPPED;

byte tenths = 0;
char seconds = 0;
char minutes = 0;

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN,LOW);
  // put your setup code here, to run once:
  Timer1.initialize();
  MFS.initialize(&Timer1);    // initialize multifunction shield library
  
  Serial.begin(9600);


#ifdef CONFIG_EEPROM
  configuration.life_timer_miniute = LIFE_OF_UV_TUBE;
  configuration.lastTimerVal = 10;
  configuration.total_shut = 0;
  writeEEPROM();
  Serial.println("\n\nEEPROM configured");
  Serial.print("Life: ");
  Serial.print(configuration.life_timer_miniute);
  MFS.write("CONF");
#else
  Serial.println("\n\nUV countdown timer ...");
  readEEPROM();
  Serial.print("Life: ");
  Serial.print(configuration.life_timer_miniute);
  Serial.print("\nTotal shoot: ");
  Serial.print(configuration.total_shut);
  MFS.write((int)(configuration.life_timer_miniute/60));
  delay(1000);
  timerValue = configuration.lastTimerVal;
  MFS.write(timerValue);
#endif
  
}


void loop() {
  // put your main code here, to run repeatedly:
#ifdef CONFIG_EEPROM
  return;
#endif
  if(configuration.life_timer_miniute<60)
  {
    MFS.write("Err");
    while(1);
    return;
  }
  byte btn = MFS.getButton();
  
  switch (countDownMode)
  {
    case COUNTING_STOPPED:
        if (btn == BUTTON_1_SHORT_RELEASE)
        {
          minutes = timerValue;
          seconds = 0;
          // start the timer
          countDownMode = COUNTING;
          MFS.beep();
          MFS.writeLeds(LED_ALL, ON);
          MFS.blinkLeds(LED_ALL, ON);
          configuration.lastTimerVal = timerValue;
          configuration.life_timer_miniute -= timerValue;
          configuration.total_shut++;
          writeEEPROM();
          digitalWrite(RELAY_PIN,HIGH);
        }
        else if (btn == BUTTON_1_LONG_PRESSED)
        {
          // reset the timer
        }
        else if (btn == BUTTON_2_PRESSED || btn == BUTTON_2_LONG_PRESSED)
        {
          if (timerValue < 60)
          timerValue++;
          MFS.write(timerValue);
        }
        else if (btn == BUTTON_3_PRESSED || btn == BUTTON_3_LONG_PRESSED)
        {
          if(timerValue>0)
            timerValue--;
          MFS.write(timerValue);
        }
        break;
        
    case COUNTING:
        if (btn == BUTTON_1_SHORT_RELEASE || btn == BUTTON_1_LONG_RELEASE)
        {
          // stop the timer
          //countDownMode = COUNTING_STOPPED;
        }
        else
        {
          // continue counting down
          tenths++;
          
          if (tenths == 10)
          {
            tenths = 0;
            seconds--;
            
            if (seconds < 0 && minutes > 0)
            {
              seconds = 59;
              minutes--;
            }
            
            if (minutes == 0 && seconds == 0)
            {
              // timer has reached 0, so sound the alarm
              MFS.beep(50, 50, 10);  // beep 10 times, 500 milliseconds on / 500 off
              countDownMode = COUNTING_STOPPED;
              MFS.writeLeds(LED_ALL, OFF);
              MFS.write("End");
              digitalWrite(RELAY_PIN,LOW);
              //MFS.blinkDisplay(DIGIT_ALL, ON);
              break;
            }
            
            MFS.write((minutes*100 + seconds)/100.0,2);
          }
          delay(100);
        }
        break;
  }
}

void readEEPROM()
{
   byte b2[sizeof(config_t)]; // create byte array to store the struct
   for(int i=0;i<sizeof(config_t);i++)
    b2[i] = EEPROM.read(i);
   memcpy(&configuration, b2, sizeof(config_t)); // copy byte array to temporary struct
   if(configuration.total_shut ==4294967295) //first time
      configuration.total_shut = 0;
   if(configuration.lastTimerVal ==-1)  //first time value
      configuration.lastTimerVal = TIMER_DEFUALT;
}
 
void clearEEPROM()
{
  //EEPROM.length()
}
 
void writeEEPROM()
{
   byte b2[sizeof(config_t)]; // create byte array to store the struct
   memcpy(b2, &configuration, sizeof(config_t)); // copy the struct to the byte array
   for(int i=0;i<sizeof(config_t);i++)
    EEPROM.write(i, b2[i]);
}
