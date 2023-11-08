#include "Arduino.h"
#include <RCSwitch.h>
#include <SoftwareSerial.h>
int txPin = 13;//SIM800 rx pin
int rxPin = 12;//SIM800 tx pin
SoftwareSerial mySerial(rxPin, txPin);
char* PhoneNumber1 = "ATD09******733;" ; // for special characters: 10=*, 11=#, 12=1sec delay
char* PhoneNumber2 = "ATD09******11;";
char* PhoneNumber3 = "ATD09******499;";
String Message;
uint8_t reg_fail = 0;
int tail = 13140;

long unsigned int InactiveSignal1 = 12451490;
long unsigned int InactiveSignal2 = 15484578;
long unsigned int ActiveSignal1 = 12451489;
long unsigned int ActiveSignal2 = 15484577;
long unsigned int MotionSignal = 5592405;//12451492;//5592405;
int Motionsensor = 4; //motion detector
int AlarmPin = 5;
uint8_t nchg = 0;
unsigned long CurrentTime;
unsigned long PhoneCallstartTime;
unsigned long AlarmstartTime;
RCSwitch mySwitch = RCSwitch();
bool isactive = true;
bool motiondetect = false;
uint8_t st = 0;
uint8_t countcall = 0;
bool frstloop = true;
uint8_t motion = 0;
uint8_t motion_Trigger_count = 0;
uint8_t motion_Second = 0;

double Voltage_Avr = 0;
double Voltage_Avr_BD = 0;
double Current_Avr = 0;
double Balance_Voltage_Avr = 0.0;
uint8_t Current_control_Pin = 10;
int count_loop = 0;
int count_loop_a = 0;
uint8_t Start_Charging_Pin = 11;
unsigned long StartCharging;
uint8_t Count_triger = 0;
uint8_t Push_Bottum_Pin = 9;
uint8_t Charging_Mode_Trigger = 0;
uint8_t Emerg_Charging_Mode_Trigger = 0;
uint8_t LED_Pin = 8;
uint8_t Count_triger_Charg=0;
uint8_t state=0;



void setup() {
  // put your setup code here, to run once:
  //Serial.begin(9600);
  pinMode(Start_Charging_Pin, OUTPUT);
  pinMode(Current_control_Pin, OUTPUT);
  pinMode(LED_Pin, OUTPUT);
  pinMode(Push_Bottum_Pin, INPUT);


  pinMode(txPin, OUTPUT);
  pinMode(rxPin, INPUT);
  Serial.begin(115200);
  //Begin serial communication with Arduino and SIM800L
  mySerial.begin(115200);
  pinMode(AlarmPin, OUTPUT);
  mySwitch.enableReceive(0);  // Receiver on interrupt 0 => that is pin #2
}

void loop() {
  if (digitalRead(Push_Bottum_Pin) == HIGH) {
    Charging_Mode_Trigger = 1;
    digitalWrite(LED_Pin, HIGH);
    
    delay(200);
  }
  if (Charging_Mode_Trigger == 1)
  {
    Charging_mode();
    Count_triger_Charg=0;
    Emerg_Charging_Mode_Trigger = 0;
    if (digitalRead(Push_Bottum_Pin) == HIGH) {
      delay(3000);
      if (digitalRead(Push_Bottum_Pin) == HIGH) {
        Charging_Mode_Trigger = 0;
        digitalWrite(Start_Charging_Pin, LOW);
        digitalWrite(LED_Pin, LOW);
        delay(500);

      }
    }
  } else if (Emerg_Charging_Mode_Trigger == 1)
  {
    digitalWrite(LED_Pin, HIGH);
    Count_triger_Charg=0;
    Charging_mode();
    Charging_Mode_Trigger = 0;
    //int motion_Second = digitalRead(Motionsensor);
    if (mySwitch.available()) {

      int value = mySwitch.getReceivedValue();

      if (value == 0) {
      } else {
        long unsigned int a = mySwitch.getReceivedValue() ;
        if (a == MotionSignal )
        {

          motion_Second = 1;
        }
      }
    }
    if (isactive == true)
    {
      if ( motion_Second == 1 )
      {
        Emerg_Charging_Mode_Trigger = 0;
        Charging_Mode_Trigger = 0;
        motion_Second = 0;
        digitalWrite(LED_Pin, LOW);

      }
    }
  }
  else {
    digitalWrite(LED_Pin, LOW);
    Charging_Mode_Trigger = 0;
    Emerg_Charging_Mode_Trigger = 0;
    Burglar_Mode();
    Check_Voltage();
    delay(80);
  }

}

void Burglar_Mode()
{
  digitalWrite(Start_Charging_Pin, LOW);
  if (mySwitch.available()) {

    int value = mySwitch.getReceivedValue();

    if (value == 0) {
    } else {
      long unsigned int a = mySwitch.getReceivedValue() ;

      if (a == InactiveSignal1 || a == InactiveSignal2)
      {
        motiondetect = false;
        isactive = false;
        st = 0;
        countcall = 0;
        motion = 0;
        motion_Second = 0;
        motion_Trigger_count = 0;
        Serial.println("Inactive ");
        digitalWrite(AlarmPin, HIGH);
        delay(300);
        digitalWrite(AlarmPin, LOW);
         mySwitch.resetAvailable();

      }
      if (a == ActiveSignal1 || a == ActiveSignal2 )
      {
        digitalWrite(AlarmPin, LOW);
        isactive = true;
        st = 0;
        countcall = 0;
        mySwitch.resetAvailable();
        motiondetect = false;
        delay(4000);
        motiondetect = false;
        motion = 0;
        motion_Second = 0;
        motion_Trigger_count = 0;
        Serial.println("Active ");
        digitalWrite(AlarmPin, HIGH);
        delay(300);
        digitalWrite(AlarmPin, LOW);
        delay(300);
        digitalWrite(AlarmPin, HIGH);
        delay(300);
        digitalWrite(AlarmPin, LOW);

      }
      if (a == MotionSignal )
      {
         Serial.println("trggr received ");
        motion_Trigger_count++;
        mySwitch.resetAvailable();
        //motion = 1;
      }
    }
  }
  // int motion_Second = digitalRead(Motionsensor);
  //  if (digitalRead(Motionsensor) == 1)
  //  {
  //    motion_Trigger_count++;
  //  }
  if (motion_Trigger_count > 2)
  {
    motion_Second = 1;
    motion = 1;
  }
  if (motion_Trigger_count > 3)
  {
    motion_Trigger_count = 4;
  }
  if (frstloop == true)
  {
    delay(10000);
    //motion=LOW;
    frstloop = false;
    digitalWrite(AlarmPin, LOW);
    motion = 0;
    motion_Second = 0;
    motion_Trigger_count = 0;
  }
  if (isactive == true)
  {
    if (motion == 1 || motion_Second == 1 )
    {
      Serial.println("Sensor Triger");

      digitalWrite(AlarmPin, HIGH);
      motiondetect = true;
      if (st == 0) {
        PhoneCallstartTime = millis();
        AlarmstartTime = millis();
        st = 1;
      }

    }
  }
  if (motiondetect == false) {
    digitalWrite(AlarmPin, LOW);
    motion_Second = 0;
    motion = 0;

  }

  CurrentTime = millis();
  if (motiondetect == true && isactive == true && (CurrentTime - AlarmstartTime) > 240000)
  {
    Serial.println("Max Time");
    digitalWrite(AlarmPin, LOW);
    delay(30000);
    AlarmstartTime = millis();
    motion = 0;
    motiondetect = false;
    motion_Trigger_count = 0;
    motion_Second=0;
  }

  if (motiondetect == true && isactive == true && (CurrentTime - PhoneCallstartTime) > 18000)
  {

    if (nchg == 0)
    {
      mySerial.println(PhoneNumber1);
      updateSerial();
      //updateSerial();
      delay(12000);
      mySerial.println("ATH"); //hang up
      updateSerial();
      delay(1000);
      nchg = 1;
      countcall++;
    } else if (nchg == 1)
    {

      delay(3000);
      mySerial.println(PhoneNumber2);
      updateSerial();
      delay(12000);
      mySerial.println("ATH"); //hang up
      updateSerial();
      delay(1000);
      nchg = 2;
      countcall++;
    } else
    {

      delay(3000);
      mySerial.println(PhoneNumber3);
      updateSerial();
      delay(12000);
      mySerial.println("ATH"); //hang up
      updateSerial();
      delay(1000);
      nchg = 0;
      countcall++;
    }

  }

  if (countcall > 8)
  {
    PhoneCallstartTime = millis();
    countcall = 0;
  }
  Check_Reg();
  Change_IMEI();
}

void Charging_mode()
{
  count_loop++;
  digitalWrite(Start_Charging_Pin, HIGH);

  double VoltageAD = analogRead(A1) * 5 / 1023.0 * 2 ;
  double VoltageBD = analogRead(A0) * 5 / 1023.0 * 2 ;
  double BalanceVoltage = analogRead(A2) * 5 / 1023.0 * 2 ;

  double Current = (VoltageBD - VoltageAD) / 2.2;
  //Serial.println(VoltageBD);


  Current_Avr = Current_Avr + Current;
  Voltage_Avr = VoltageAD + Voltage_Avr;
  Voltage_Avr_BD = VoltageBD + Voltage_Avr_BD;
  Balance_Voltage_Avr = Balance_Voltage_Avr + BalanceVoltage;
  if (Current > .5)
  {
    digitalWrite(Current_control_Pin, HIGH);

  } else {
    digitalWrite(Current_control_Pin, LOW);
  }

  if (count_loop > 200)
  {
    count_loop = 0;
    Current_Avr = Current_Avr / 201;
    Voltage_Avr = Voltage_Avr / 201;
    Voltage_Avr_BD = Voltage_Avr_BD / 201;
    Balance_Voltage_Avr = Balance_Voltage_Avr / 201;

    Serial.println(Voltage_Avr);
    Serial.println(Current_Avr);
    Serial.println(Balance_Voltage_Avr);
    Serial.println(Voltage_Avr_BD);
    if (Voltage_Avr > 7.95 && Current_Avr < .06)
    {
      Count_triger++;
    }


    if (Voltage_Avr > 8.10 )
    {
      Count_triger++;
    }
    if (Balance_Voltage_Avr > 4.2 || Voltage_Avr - Balance_Voltage_Avr > 4.2)
    {
      Count_triger++;
    }

    if ( Current_Avr > 1.1)
    {
      Count_triger++;
    }

    if (Count_triger > 20)
    {
      digitalWrite(Start_Charging_Pin, LOW);
      Count_triger = 0;
      Charging_Mode_Trigger = 0;
      Emerg_Charging_Mode_Trigger = 0;
      digitalWrite(LED_Pin, LOW);

    }

  }

  delay(1);

}
void Check_Voltage()
{
  count_loop++;
  double VoltageAD = analogRead(A1) * 5 / 1023.0 * 2 ;
  double BalanceVoltage = analogRead(A2) * 5 / 1023.0 * 2 ;

  Voltage_Avr = VoltageAD + Voltage_Avr;
  Balance_Voltage_Avr = Balance_Voltage_Avr + BalanceVoltage;
  //Serial.println(count_loop);
  if (count_loop > 20)
  {
    count_loop = 0;
    Voltage_Avr = Voltage_Avr / 21-.4;
    Balance_Voltage_Avr = Balance_Voltage_Avr / 21;
    if (Voltage_Avr < 7.25 || Balance_Voltage_Avr < 3.65)
    {
      Count_triger++;
    }
    if (Voltage_Avr < 7.4 || Balance_Voltage_Avr < 3.7)
    {
      Count_triger_Charg++;
    }
    Serial.println(Voltage_Avr);
    Serial.println(Balance_Voltage_Avr);

  }

  if (Count_triger > 10)
  {
    Count_triger = 0;
    Emerg_Charging_Mode_Trigger = 1;
  }
  if (Count_triger_Charg > 10)
  {
    digitalWrite(LED_Pin, state);
    state=1-state;
  }
}

void updateSerial()
{
  delay(500);
  while (Serial.available())
  {
    mySerial.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while (mySerial.available())
  {
    Serial.write(mySerial.read());//Forward what Software Serial received to Serial Port
  }
}
void Check_Reg()
{
  mySerial.println("AT+CREG?");
  delay(500);
  if (mySerial.available() > 0) {
    Message = mySerial.readString();
    delay(10);
  }
  uint8_t d = Message.lastIndexOf("+");
  String SubMess =  Message.substring(d + 6, d + 11);


  if (SubMess.substring(SubMess.lastIndexOf(",") + 1, SubMess.lastIndexOf(",") + 2) == "1")
  {
    Serial.println("no problem");
  } else if (SubMess.substring(SubMess.lastIndexOf(",") + 1, SubMess.lastIndexOf(",") + 2) == "3")
  {
    reg_fail++;
    Serial.println("fuck! registration is denied");
  }
}
void Change_IMEI()
{
  if (reg_fail > 20)
  {
    String IMEI = "8678560311";
    tail++;
    mySerial.print("AT+EGMR=1,7,"); //Check whether it has registered in the network
    mySerial.println(IMEI + (String)tail);
    updateSerial();
    delay(3000);
    reg_fail = 0;
  }
  if (tail > 64000)
  {
    tail = 10000;
  }
}

