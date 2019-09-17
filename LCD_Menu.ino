// include the library code:
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

const uint8_t scroll_bar[4][8] = {
    {B11111, B11111, B00000, B01010, B10101, B01010, B10101, B01010}, //0 
    {B10101, B01010, B00000, B11111, B11111, B00000, B10101, B01010}, //1 
    {B10101, B01010, B10101, B01010, B10101, B00000, B11111, B11111}, //2
    {B10101, B01010, B10101, B01010, B10101, B01010, B10101, B01010}  //3
  };

//Camera Trigger
static int camTrig = A3;

//Motor Configuration
static int motorAL = 3; static int motorAR = 2;
static int motorBL = 12; static int motorBR = 13;
int motval1 = 0; int motval2 = 0;

//Motor Statement
int motorA1 = LOW; int motorA2 = LOW;
int motorB1 = LOW; int motorB2 = LOW;

int motorA_status = "ON";
int motorB_status = "OFF";
int motorC_status = "OFF";

//Limit Switch Configuration
int limitA = A0;  int sensorA = 1000;
int limitB = A1;  int sensorB = 1000;

//init menu level
int motor_direction = "Left";
int motor_rotation = "CW";
long motor1_delay = 0;
long motor2_delay = 0;
long motor3_delay = 0;
bool run_status = true;
int Cam_Stat = HIGH;
int Cam_Trigger = "ON";
const int ledPin =  LED_BUILTIN;
int overridestop = 0;

int init_counter=3;
byte lev0=0; //init level-0
byte lev1=0; //init level-1
int sel_item=0; //Selected item
int first_item=0; //Selected menu first item
int last_item=0; //Selected menu last item
int qty=0; //number of menu items
byte menu_sym=0x7e; //menu symbol
int del=250; //delay
byte mcols=16; //lcd display number of cols
byte mrows=2; //lcd display number of rows
bool clr=false;
bool mstate=false;
byte brightness=255; //default brightness
byte volume=50; //default volume
long previousMillis = 0;  // здесь будет храниться время последнего изменения состояния задержки анимации
long interval = 2000;      // интервал начала скроллинга в мс и оно же время убирания функционального меню
long previousMillis2 = 0;  // здесь будет храниться время последнего изменения состояния задержки анимации
long interval2 = 40;      // интервал между буквами скроллинга в мс
bool infunc=false; //показатель что находимся внутри выполнения функции меню
long interval3 = 0;
long interval4 = 0;
long interval5 = 0;
long previousMillis3 = 0;
long previousMillis4 = 0;
long previousMillis5 = 0;


void lcd_clr() {
//clear screen function
lcd.clear();
lcd.setCursor(0,0);
lcd.print("CLEAR SCREEN");
delay(1000);
lcd_menu();
}

//set Lmenu
typedef struct {
  unsigned char* itemname;
  byte level;
  byte sublevel;
  bool checked;
  bool selected;
  void (*function)();
  } Lmenu;

Lmenu lmenu[]= {
    { "Start Motors",   0,  0,  false,  false, init_motors }, //zero
    { "Motor A Conf",   0,  0,  false,  false, nextlevel },
    { "Motor B Conf",   0,  0,  false,  false, nexttwolevel },
    { "Camera Trigger",   0,  0,  false,  false, nextthreelevel},
    { "LCD Brightness",   0,  0,  false,  false, set_brightness },
    { "Beep Volume",   0,  0,  false,  false, set_volume },
    
    { "Motor A Conf",   0,  1,  false,  false, zero }, //level 2
    { "Status ",   0,  1,  false,  false, setA_status },
    { "Direction ",   0,  1,  false,  false, set_direction },
    { "Interval ",   0,  1,  false,  false, interval1_count },
    { "Back",   0,  1,  false,  false, rootlev }, 
    
    { "Motor B Conf",   0,  2,  false,  false, zero }, //level 3
    { "Status ",   0,  2,  false,  false, setB_status },
    { "Rotation ",   0,  2,  false,  false, set_rotation },
    { "Interval ",   0,  2,  false,  false, interval2_count },
    { "Back",   0,  2,  false,  false, rootlev },

    { "Camera Trigger",   0,  3,  false,  false, zero }, //level 3
    { "Status ",   0,  3,  false,  false, setC_status },
    { "Interval ",   0,  3,  false,  false, interval3_count },
    { "Back",   0,  3,  false,  false, rootlev },
};


void level_recount() {
  int i;
  int j=0;
  //найдем первый элемент этого меню
  for (i=0; i<(sizeof(lmenu)/sizeof(Lmenu)); i++) {
    if ((lev0==lmenu[i].level)&&(lev1==lmenu[i].sublevel)) {
      if(j==0) {
        sel_item=i;
        first_item=i;
      }
      last_item=i;
      j++;
    }
  }
  qty=j;
}

void setup() {
  //Serial.begin(9600);
  
  pinMode(motorAL, OUTPUT);  pinMode(motorAR, OUTPUT);
  pinMode(motorBL, OUTPUT);  pinMode(motorBR, OUTPUT);
  pinMode(limitA, INPUT);  pinMode(limitB, INPUT);
  
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.createChar(0, (uint8_t*)scroll_bar[0]); //0
  lcd.createChar(1, (uint8_t*)scroll_bar[1]); //1
  lcd.createChar(2, (uint8_t*)scroll_bar[2]); //2
  lcd.createChar(3, (uint8_t*)scroll_bar[3]); //3
  // пример вывода: lcd.print("\0");
  analogWrite(10,brightness); //яркость на 10-м пине для DFRobot keypad shield 0-255
  lcd.clear();
  level_recount();
  lcd_menu();
}
 

byte key() {
  //right 5 - 0
  //up 3 - 98
  //down 4 - 254
  //left 2  - 408 (редко 409)
  //select 1  - 640 (бывает и 642)
  //rst - RESET
  int val=analogRead(0);
  if (val<50)  return 5; 
  else if (val<150)  return 3; 
  else if (val<400)  return 4; 
  else if (val<500)  return 2; 
  else if (val<800)  return 1; 
  else   return 0; 
}

byte ncomm (byte command) {
  lcd.setCursor(0,0);
  switch (command) {
  case 1: lcd.print("Brightness");  break;
  case 2: lcd.print("Beep Volume"); break; 
  case 3: lcd.print("Direction");   break;
  case 4: lcd.print("Rotation");    break;
  case 5: lcd.print("Interval");    break;
  case 6: lcd.print("Interval");    break;
  case 7: lcd.print("Interval");    break;
  }  
}

byte scomm (byte command, byte val) {
  lcd.setCursor(0,0);
  switch (command) {
  case 1: brightness=val;        bright(brightness);    break;
  case 2: volume=val;            break; 
  case 3: motor_direction=val;   break;
  case 4: motor_rotation=val;    break;
  case 5: motor1_delay=val;      break;
  case 6: motor2_delay=val;      break;
  case 7: motor3_delay=val;      break;
  }  
}

byte set_var(byte command, int val, int maxlevel, int minlevel, int steps) {
  previousMillis = millis();  // запоминаем текущее время
  byte s;
  lcd.clear();
  ncomm(command);
  infunc=true;
  while(infunc==true) {
    if (millis() - previousMillis > interval) {
      s=key();
      if((s==0)||(s==1))  {
        infunc=false;
        lcd_menu();
        delay (500);
        break;
      } 
    } 
  lcd.setCursor(0,1);
  for (s=0;s<10;s++) {
    if (floor(val/(floor(maxlevel)/10))>s) { 
      lcd.print((char)255); 
    } else { 
      lcd.print("\3"); 
    }
  }
if(val<10) {lcd.print(" ");}
if(val<100) {lcd.print(" ");}
if(val<1000) {lcd.print(" ");}
if(val<10000) {lcd.print(" ");}
if(val<100000) {lcd.print(" ");}
lcd.print(val);
delay(200);
switch(key()) {
case 1:
infunc=false;
lcd_menu();
delay (500);
break;
case 4:
 previousMillis = millis();  // запоминаем текущее время
 if( val>minlevel) { val-=steps; }
 scomm(command,val);
 break;
 case 2:
 previousMillis = millis();  // запоминаем текущее время
 if( val>minlevel) { val-=steps; }
 scomm(command,val);
 break;
 case 3: 
 previousMillis = millis();  // запоминаем текущее время
 if( val<maxlevel) { val+=steps; }
 scomm(command,val);
 break;
 case 5: 
 previousMillis = millis();  // запоминаем текущее время
 if( val<maxlevel) { val+=steps; }
 scomm(command,val);
 break;
 case 6: 
 previousMillis = millis();  // запоминаем текущее время
 if( val<maxlevel) { val+=steps; }
 scomm(command,val);
 break;
 case 8:
 previousMillis = millis();  // запоминаем текущее время
 if( val<maxlevel) { val+=steps; }
 scomm(command,val);
 break;
 case 9:
 previousMillis = millis();  // запоминаем текущее время
 if( val<maxlevel) { val+=steps; }
 scomm(command,val);
 break;
}
}
return val;
}

void set_volume() {
  set_var(2, volume, 100, 0, 5);
}

void set_brightness() {
  set_var(1, brightness, 255, 0, 5);
}

void interval1_count() {
  set_var(5, motor1_delay, 120, 0, 2);
}

void interval2_count() {
  set_var(6, motor2_delay, 120, 0, 2);
}

void interval3_count() {
  set_var(7, motor3_delay, 120, 0, 2);
}

void set_direction() {
  if(motor_direction=="Left" || motor_direction=="LeftLow") {motor_direction="Right";} else {motor_direction="Left";}
  analogWrite(10,255);
  delay(500);
  analogWrite(10,0);
  delay(500);
  analogWrite(10,brightness);
}

void set_rotation() {
  if(motor_rotation=="CW") {motor_rotation="CCW";} else {motor_rotation="CW";}
  analogWrite(10,255);
  delay(500);
  analogWrite(10,0);
  delay(500);
  analogWrite(10,brightness);
}

void onoff() {
  if(brightness==0){ brightness=255; } else { brightness=0; }
  analogWrite(10,brightness);
  delay(500);
}

void lcdConfirm() {
  analogWrite(10,255); delay(50); analogWrite(10,0); delay(50);
  analogWrite(10,255); delay(50); analogWrite(10,0); delay(50);
  analogWrite(10,brightness);
}

void Camera_Trigger() {
  if(Cam_Trigger=="ON"){ Cam_Trigger="OFF"; } else { Cam_Trigger="ON"; }
  lcdConfirm();
}

void setA_status() {
  if(motorA_status=="ON"){ motorA_status="OFF"; } else { motorA_status="ON"; }
  lcdConfirm();
}

void setB_status() {
  if(motorB_status=="ON"){ motorB_status="OFF"; } else { motorB_status="ON"; }
  lcdConfirm();
}

void setC_status() {
  if(motorC_status=="ON"){ motorC_status="OFF"; } else { motorC_status="ON"; }
  lcdConfirm();
}

void bright(byte val) {
  analogWrite(10,val);
}

void brighter() {
  if(brightness<255) {brightness+=1;}
  bright(brightness);
  delay(10);
}

void darker() {
  if(brightness>=1) {
    brightness-=1;
  }
  bright(brightness);
  delay(10); 
}

void zero() {
  //empty fuction
  return;
}

void init_motors() {
  run_status = true;
  int i;
  for (i=init_counter; i>0; i--) {
    lcd.clear();
    lcd.setCursor(7, 0);
    lcd.print(i); 
    delay(1000);
  }
  lcd.setCursor(0, 0);  lcd.print("A: "); 
  lcd.setCursor(3, 0);  if(motorA_status=="ON"){ lcd.print("ON"); } else { lcd.print("OFF"); }
  lcd.setCursor(7, 0);  if (motor_direction=="Right" || motor_direction=="RightLow") { lcd.print("BW"); } else { lcd.print("FW"); }
  lcd.setCursor(10, 0); lcd.print("D: ");
  lcd.setCursor(13, 0); lcd.print(motor1_delay);

  lcd.setCursor(0, 1);  lcd.print("B: "); 
  lcd.setCursor(3, 1);  if(motorB_status=="ON"){ lcd.print("ON"); } else { lcd.print("OFF"); }
  lcd.setCursor(7, 1);  if (motor_rotation=="CW") { lcd.print("CW"); } else { lcd.print("CC"); }
  lcd.setCursor(10, 1); lcd.print("T: ");
  lcd.setCursor(13 , 1); if(Cam_Trigger=="ON"){ lcd.print("ON"); } else { lcd.print("OFF"); }
  
  while (run_status == true) {
    start_motors();
  }
  lcd.clear();
  lcd.setCursor(0, 3);
  lcd.print("Motor Stop"); 
  delay(3000);

  lcd.clear();
  lcd_menu();
  return;
}

void start_motors() { 
  interval3 = motor1_delay*1000;
  interval4 = motor2_delay*1000;
  interval5 = motor3_delay*1000;

  if (millis() - previousMillis > interval3) {
    previousMillis = millis();
    if (motorA_status == "ON") {
      if (motor_direction == "Left") {
        motorA1 = HIGH;   motorA2 = LOW;
        motor_direction = "LeftLow";
      } else if (motor_direction == "LeftLow") {
        motorA1 = LOW;   motorA2 = LOW;
        motor_direction = "Left";
      } else if (motor_direction == "Right") {
        motorA1 = LOW;   motorA2 = HIGH;
        motor_direction = "RightLow";
      } else {
        motorA1 = LOW;   motorA2 = LOW;
        motor_direction = "Right";
      }
    } else {
      motorA1 = LOW;   motorA2 = LOW;
      motor_direction = "LeftLow";
    }
    runmotor1();
  }
  
  readsensor();
  
  if (millis() - previousMillis4 > interval4) {
    previousMillis4 = millis();
    if (motorB_status == "ON") {
      if (motor_rotation == "Left") {
        motorB1 = HIGH;   motorB2 = LOW;
        motor_rotation = "LeftLow";
      } else if (motor_rotation == "LeftLow") {
        motorB1 = LOW;   motorB2 = LOW;
        motor_rotation = "Left";
      } else if (motor_rotation == "Right") {
        motorB1 = LOW;   motorB2 = HIGH;
        motor_rotation = "RightLow";
      } else {
        motorB1 = LOW;   motorB2 = LOW;
        motor_rotation = "Right";
      }
    } else {
      motorB1 = LOW;   motorB2 = LOW;
      motor_rotation = "LeftLow";
    }
    runmotor2();
  }

  if (millis() - previousMillis5 > interval5) {
    previousMillis5 = millis();
    delaytrigger();
  }
}

void runmotor1() {
    digitalWrite(motorAL, motorA1); digitalWrite(motorAR, motorA2);
}

void runmotor2() {
    digitalWrite(motorBL, motorB1); digitalWrite(motorBR, motorB2);
}

void delaytrigger() {
  if (motor3_delay > 1) {
    if (millis() - previousMillis > interval2 ) {
      if(Cam_Trigger=="ON"){ Cam_Stat=HIGH; } else { Cam_Stat=LOW; }
        digitalWrite(camTrig, Cam_Stat);
    } else {
      digitalWrite(camTrig, LOW); 
    }
  }
}

void readsensor() {
  sensorA = analogRead(limitA);
  sensorB = analogRead(limitB);
  overridestop = analogRead(0);

  if ((sensorA < 500)||(sensorB < 500) || overridestop < 642) {
    //Serial.print("Sensor 1: ");    Serial.print(sensorA);       Serial.print(" ");
    //Serial.print("Sensor 2: ");    Serial.print(sensorB);       Serial.println(" ");
    digitalWrite(motorAL, LOW); digitalWrite(motorAR, LOW);
    digitalWrite(motorBL, LOW); digitalWrite(motorBR, LOW);
    run_status = false;

    lcd_menu();
    return;
  }
}

void endlev() {
  lcd.clear();
  infunc=true;
  level_recount();
  sel_item=first_item;
  delay(200);
  infunc=false;
  lcd_menu();
  return;
}

void nextlevel() {
  lcd.clear();
  infunc=true;
  lev1++;
  level_recount();
  delay(200);
  infunc=false;
  lcd_menu();
  return;
}

void nexttwolevel() {
  lcd.clear();
  infunc=true;
  lev1 = lev1+2;
  level_recount();
  delay(200);
  infunc=false;
  lcd_menu();
  return;
}

void nextthreelevel() {
  lcd.clear();
  infunc=true;
  lev1 = lev1+3;
  level_recount();
  delay(200);
  infunc=false;
  lcd_menu();
  return;
}

void rootlev() {
  lcd.clear();
  infunc=true;
  lev0=0;
  lev1=0;
  sel_item=0;
  level_recount();
  delay(200); 
  infunc=false;
  lcd_menu();
  return;
}

void loop() {
  //show menu

  //start_motors();
  
  if (millis() - previousMillis > interval) {
    previousMillis = millis();  // запоминаем текущее время
    mstate=true;
    if(infunc==false) {
      str_animate();
    }
  }

  if(clr==false) {
    switch(key()) {
    
    case 0: //нет нажатий
      clr==false;
      break;

    case 1: //Select
      if(infunc==false) {
        run_menu();
      } else {
        infunc=false;
        lcd_menu();
        delay(1000);
      }
      break;

    case 2: //left
      //if(infunc==false) {
      //  if(volume>5) { 
      //    volume-=5; 
      //  }
      //set_var(0, volume ,100,0, 5);
      //}
      break;

    case 3: //Up
      if(infunc==false) {
        sel_item--;
        if(sel_item<first_item) { 
          sel_item=first_item; 
        }
        clr=true;
      }
      break;

    case 4: //Down
      if(infunc==false) { 
        if(sel_item<last_item) {  
          sel_item++; 
        }
        clr=true;
      }
      break;

    case 5: //right
      //if(infunc==false) {
      //  if(volume<100) { 
      //    volume+=5; 
      //  }
        //set_var(0, volume ,100, 0, 5);
      //}
      break;
    }

  } else {
    if(infunc!=true) {
      if(key()==0) {
        lcd_menu();
      }
    }
  }
}

void lcd_menu() {
  mstate=false;
  infunc=false;
  int i;
  int j;
  int row=0;
  clr=false;
  for (i=first_item; i<=last_item; i++) {
    if ((i>=sel_item)&&(row<mrows)&&(lev0==lmenu[i].level)&&(lev1==lmenu[i].sublevel)) {
      lcd.setCursor(0, row);
      if(i==sel_item) {
        lcd.print ((char)menu_sym); 
      } else {
        lcd.print(" "); 
      }
      //sprintf(nav," %d/%d",(i+1),qty);
      int procent=ceil((sel_item-first_item)*100/qty);
      char* item=lmenu[i].itemname;

      //обрежем название
      if(strlen(item)>(mcols-2)) {
        mstate=true;
      }
      
    for(j=0; j<strlen(item); j++) { 
      if(j<(mcols-2)) {
        lcd.print(item[j]);
      }
    }
    
    //курсорами добьем до конца
    for(j=(strlen(item)+1); j<(mcols-1); j++) { 
      lcd.print(" ");
    }

    //сформируем navbar
    if ((i==(qty-1))||(row==(mrows-1)))  { 
      //низ
      if(procent>=55) {
        if(procent>=70) {
          if(procent>=90) {
            lcd.write((uint8_t)2);    
          } else {
            lcd.write((uint8_t)1);  
          }   
        } else {
          lcd.write((uint8_t)0); 
        }
      } else {
        lcd.write((uint8_t)3);   
      }
    } else {
      if ((i==first_item)||(i==sel_item)){
          //верх
          if(procent<=55) {
            if(procent<=35) {
              if(procent<=20) {
                lcd.write((uint8_t)0);   
              } else{
                lcd.write((uint8_t)1);   
              } 
            } else {
              lcd.write((uint8_t)2); 
            }
          } else {
            lcd.write((uint8_t)3);   
          }
        } else {
          if (sel_item==qty){
            //последний элемент меню на экране
            lcd.write((uint8_t)2);  
          } else {
            //все остальное ||
            lcd.write((uint8_t)3);
          }
        }
      }
      row++;
    }
  }

  //очистка последней строки
  if(row<=(mrows-1)) {
    for(j=row; j<mcols; j++) {
      lcd.setCursor(0, j);  
      for(i=0; i<=mcols; i++) {
        lcd.print(" ");
      }
    }
  }
}

void run_menu() {
  int j;
  int i;
  lmenu[sel_item].function();
}


void str_animate() {
  int j;
  int i;
  if (infunc==false) {
    char* item=lmenu[sel_item].itemname;
    if((mstate==true)&&(strlen(item)>(mcols-2))) {
      for(i=0; i<(strlen(item)+1); i++) { 
        lcd.setCursor(1, 0); 
        for(j=i; j<(strlen(item)+i); j++) { 
          if((j<(mcols-2+i))&&(j<strlen(item))) {
            lcd.print(item[j]);
              if (millis() - previousMillis2 > interval2) {
                previousMillis2 = millis();  // запоминаем текущее время
                delay(interval2);
                int val=analogRead(0);
                if(val<1000) {
                  mstate==false; return;
                }
              }
            } else {
              if(j<(mcols-2+i)) {
                lcd.print(" "); 
            } 
          }
        }
      }
      //напечатаем строку снова
      mstate=false;
      lcd.setCursor(1, 0);
      for(j=0; j<strlen(item); j++) { 
        if(j<(mcols-2)) {
          lcd.print(item[j]);
        }
      }
    } else {
      lcd.setCursor(1, 0);
      for(j=0; j<strlen(item); j++) { 
        if(j<(mcols-2)) {
          lcd.print(item[j]);
        }
      }
      mstate=false;
      return;
    }
  } else {
    if (millis() - previousMillis > interval) {
      previousMillis = millis();  // запоминаем текущее время
      delay(interval);
      infunc=false;
      lcd_menu();
    } 
  }
}
