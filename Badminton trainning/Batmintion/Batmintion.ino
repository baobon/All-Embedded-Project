#include "BatmintionIO.h"
#include "Batmintionled.h"
#include "BatmintionLCD.h"
#include "Batmintion_Button.h"
uint8_t statusSet = 0, statusHard = 0, level = 1, clearLCD = 1, trainning = 0, pinCount;
uint8_t isRunning = 0, statusButton = 0 , statusButtonfinish = 0;
// Couter
uint16_t times_off = timesoff;
uint16_t times_buzzeron = timebuzzeron;
uint8_t times_counter = 0;
uint16_t times_hard = 0;
//uint8_t Set_train = 0;
uint8_t b_delay = 0;
uint8_t b_random = 0;

/*
    For WiFi Send
*/
ESP8266WebServer server(80);
const char* ssid = "Badmintion Trainning";
const char* pass = "123456789";
String command;



byte ledPin[] = {D8, D7, D6, D5, D0};
Vietduino_I2CLCD lcd(0x27, 16, 2);
OneButton btnOne(BUTTON_ONE, true);
OneButton btnTwo(BUTTON_TWO, true);

void setup() {
  Serial.begin(115200);
  Serial.println("Start Badmintion Debug");
  pinMode(BUZZER, OUTPUT);
  b_buzzerTrainningLOW();
  pinCount = sizeof(ledPin);
  for (int i = 0; i < pinCount; i++) {
    pinMode(ledPin[i], OUTPUT); //Các chân LED là OUTPUT
    digitalWrite(ledPin[i], LOW); //Mặc định các đèn LED sẽ tắt
  }

  lcd.clear();
  lcd.init();
  lcd.backlight();
  b_begin();

}

void loop() {
  VIETDUINO_UPDATE;
}


/*
   For WiFi Command Read & Handing

*/
void HTTP_handleRoot(void) {

  if ( server.hasArg("MakerCar") ) {
    Serial.println(server.arg("MakerCar"));
  }
  server.send ( 200, "text/html", "" );
  delay(1);
}

String splitString(String v_G_motherString, String v_G_Command, String v_G_Start_symbol, String v_G_Stop_symbol, unsigned char v_G_Offset)
{
  unsigned char tem_Offset = v_G_Offset - 1;
  unsigned char lenOfCommand = v_G_Command.length();

  int posOfCommand = v_G_motherString.indexOf(v_G_Command);
  int PosOfStartSymbol = v_G_motherString.indexOf(v_G_Start_symbol, posOfCommand + lenOfCommand);
  if (posOfCommand < 0)
    return "";

  // Xu ly Dau Command
  //  DB_SPLIT(v_G_Command + " posOfCommand " + (String)posOfCommand);
  if (posOfCommand > 0)
  {
    String tem__ = v_G_motherString.substring(posOfCommand - 1, posOfCommand);
    //    DB_SPLIT("posOfCommand at Start " + tem__);
    if ((tem__ != " ") && (tem__ != v_G_Stop_symbol))
    {
      return "";
    }
  }

  // Xu ly cuoi Command
  unsigned int temPosOfLastCommand = posOfCommand + lenOfCommand;
  if (temPosOfLastCommand)
  {
    String tem__ = v_G_motherString.substring(temPosOfLastCommand, temPosOfLastCommand + 1);
    //    DB_SPLIT("posOfCommand at Last " + tem__);
    if ((tem__ != " ") && (tem__ != v_G_Start_symbol))
    {
      return "";
    }
  }

  if (v_G_Offset == 0)
  {
    String tem__ = v_G_motherString.substring(posOfCommand, PosOfStartSymbol);
    if (tem__ == v_G_Command)
    {
      return tem__;
    }
    return "";
  }

  while (tem_Offset > 0)
  {
    tem_Offset--;
    PosOfStartSymbol = v_G_motherString.indexOf(v_G_Start_symbol, PosOfStartSymbol + 1);
  }

  if (v_G_Stop_symbol != "")
  {
    int PosOfStopSymbol = v_G_motherString.indexOf(v_G_Stop_symbol, PosOfStartSymbol + 1);
    if (PosOfStopSymbol == -1)
    {
      return "";
    }
    else
      ;
    return v_G_motherString.substring(PosOfStartSymbol + 1, PosOfStopSymbol);
  }
  else
  {
    return v_G_motherString.substring(PosOfStartSymbol + 1);
  }
}

void handlingdata(String data) {
  /*
     Cac thong so can luu y

     statusSet set Level
     statusHard set Hard
     times_counter = time... ----> Get when set Hard
     trainning = 1 ---> When Start
     times_counter = 0 ---> Btn Out

  */
  statusSet       = (splitString(data, "statusSet", "=", ",", 1)).toInt();
  statusHard      = (splitString(data, "statusHard", "=", ",", 1)).toInt();
  trainning       = (splitString(data, "trainning", "=", ",", 1)).toInt();
  uint8_t b_stop  = (splitString(data, "stopTrainning", "=", ",", 1)).toInt();
  if (b_stop == 1) {
    times_counter = 0;
  }


}

CREATE_FUNCTION(ReadValueFromPhone) {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, pass);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  //  Starting WEB - server
  server.on ( "/", HTTP_handleRoot );
  server.onNotFound ( HTTP_handleRoot );
  server.begin();
  Serial.println("Begin Send Data");
  while (1)
  {
    server.handleClient();
    command = server.arg("Badmintion");
    if (command != "")
    {
      handlingdata(command);
    }
    M_DELAY(0);

  }

  M_DELAY(0);
  END_CREATE_FUNCTION
}



CREATE_FUNCTION(TimeLed) {
  if (times_counter > 0 && trainning == 1) {
    btnTwo.attachDoubleClick(btn_OutRun);
    if (b_delay == 1) {
      btnTwo.attachDoubleClick(btn_OutRun);
      for (int j = 0; j < 49; j++) {
        b_random = random(5);
      }
      Serial.println(b_random);
      digitalWrite(ledPin[b_random], HIGH);
      b_buzzerTrainningHIGH();
      M_DELAY(times_buzzeron);
      b_buzzerTrainningLOW();
      M_DELAY(times_hard - times_buzzeron);
      for (int k = 0; k < pinCount; k += 1) {
        digitalWrite(ledPin[k], LOW); // Tắt đèn
      }
      M_DELAY(times_off);
    }
    M_DELAY(0);
  }

  END_CREATE_FUNCTION
}



CREATE_FUNCTION(TimerCount) {
  if (times_counter > 0 && isRunning == 1) {

    if (b_delay == 0) {
      if (clearLCD == 0) {
        lcd.clear();
        clearLCD = 1;
      } else if (clearLCD == 1) {
        lcd.setCursor(0, 0);
        lcd.print("Level ");
        lcd.print(statusSet);
        lcd.setCursor(10, 0);
        if (statusHard == 1) {
          lcd.print("Easy");
        } else if (statusHard == 2) {
          lcd.print("Normal");
        } else   if (statusHard == 3) {
          lcd.print("Hard");
        }
        lcd.setCursor(0, 1);
        lcd.print("Time");

        if (times_counter >= 60) {
          if (times_counter - 60 > 9) {
            lcd.setCursor(6, 1);
            lcd.print("1:");
            lcd.print(times_counter - 60);
          } else {
            lcd.setCursor(6, 1);
            lcd.print("1:0");
            lcd.print(times_counter - 60);
            lcd.print(" ");
          }
        }
        M_DELAY(5000);
        b_delay = 1;
      }
    } else if (b_delay == 1) {
      times_counter = times_counter - 1;
      if (times_counter >= 60) {
        if (times_counter - 60 > 9) {
          lcd.setCursor(6, 1);
          lcd.print("1:");
          lcd.print(times_counter - 60);
        } else {
          lcd.setCursor(6, 1);
          lcd.print("1:0");
          lcd.print(times_counter - 60);
          lcd.print(" ");
        }
      } else {
        if (times_counter >= 10) {
          lcd.setCursor(6, 1);
          lcd.print("0:");
          lcd.print(times_counter);
        } else {
          lcd.setCursor(6, 1);
          lcd.print("0:0");
          lcd.print(times_counter);
          lcd.print(" ");
        }
      }
    }
    M_DELAY(1000);
  } else if (times_counter == 0 && isRunning == 1) {
    isRunning = 0;
    Serial.println("Again");
    trainning = 0;
    clearLCD = 0;
    b_delay = 0;
    statusButtonfinish = 1;
  }
  M_DELAY(0);
  END_CREATE_FUNCTION
}



CREATE_FUNCTION(Setup) {

  if (statusButton == 1 && isRunning == 0) {
    b_buzzerTrainningHIGH();
    M_DELAY(TIME_BUTTONDELAY);
    b_buzzerTrainningLOW();
    statusButton = 0;
  }

  if (statusButtonfinish == 1) {
    b_buzzerTrainningHIGH();
    M_DELAY(200);
    b_buzzerTrainningLOW();
    M_DELAY(200);
    b_buzzerTrainningHIGH();
    M_DELAY(200);
    b_buzzerTrainningLOW();
    M_DELAY(200);
    b_buzzerTrainningHIGH();
    M_DELAY(200);
    b_buzzerTrainningLOW();
    statusButtonfinish = 0;
  }

  if (statusSet == 0 && isRunning == 0) {
    b_led1(); M_DELAY(100);
    b_led2(); M_DELAY(100);
    b_led3(); M_DELAY(100);
    b_led4(); M_DELAY(100);
    b_led5(); M_DELAY(100);
  } else if (statusSet == 1 && isRunning == 0) {
    b_led1();  M_DELAY(100);
    b_ledoff(); M_DELAY(100);
  } else if (statusSet == 2 && isRunning == 0) {
    b_led2();  M_DELAY(100);
    b_ledoff(); M_DELAY(100);
  } else if (statusSet == 3 && isRunning == 0) {
    b_led3();  M_DELAY(100);
    b_ledoff(); M_DELAY(100);
  }
  M_DELAY(0);
  END_CREATE_FUNCTION
}

CREATE_FUNCTION(Value) {
  if (statusSet == 4) {
    statusSet = 1;
  }
  if (statusHard == 4) {
    statusHard = 1;
  }
  M_DELAY(200);
  END_CREATE_FUNCTION
}

CREATE_FUNCTION(Debug) {
  Serial.println("statusSet: " + String(statusSet) + " statusHard: " + String(statusHard)
                 + " trainning: " + String(trainning) + " times_counter: " + String(times_counter));
  M_DELAY(200);
  END_CREATE_FUNCTION
}



CREATE_FUNCTION(TickButton) {
  while (1) {
    btnOne.tick();
    btnTwo.tick();
    M_DELAY(0);
  }
  END_CREATE_FUNCTION
}

CREATE_FUNCTION(Button) {

  if (isRunning == 0) {
    if (statusSet == 0 && statusHard == 0) {
      b_begin();
      btnOne.attachClick(btnClickLevelSet);
      M_DELAY(10);
    } else if (statusSet == 1 && statusHard == 0) {
      setLevel();
      btnOne.attachClick(btnClickLevelSet);
      btnTwo.attachDoubleClick(btn_DobleClickBackSet);
      btnTwo.attachClick(btnClickLevelHard);
      M_DELAY(10);
    } else if (statusSet == 2 && statusHard == 0) {
      setLevel();
      btnOne.attachClick(btnClickLevelSet);
      btnTwo.attachDoubleClick(btn_DobleClickBackSet);
      btnTwo.attachClick(btnClickLevelHard);
      M_DELAY(10);
    } else if (statusSet == 3 && statusHard == 0) {
      setLevel();
      btnOne.attachClick(btnClickLevelSet);
      btnTwo.attachDoubleClick(btn_DobleClickBackSet);
      btnTwo.attachClick(btnClickLevelHard);
      M_DELAY(10);
    }

    // STATUS SET = 1

    if (statusSet == 1 && statusHard == 1) {
      if (trainning == 0) {
        setHard();
        btnOne.attachClick(btnClickLevelHard);
        btnTwo.attachClick(btnClick_Trainning);
        btnTwo.attachDoubleClick(btn_DobleClickBackHard);
        M_DELAY(10);
      } else if (trainning == 1) {
        times_counter = timeEasy;
        times_hard = LedLevel1;
        isRunning = 1;
        M_DELAY(10);
      }
    } else  if (statusSet == 1 && statusHard == 2) {
      if (trainning == 0) {
        setHard();
        btnOne.attachClick(btnClickLevelHard);
        btnTwo.attachClick(btnClick_Trainning);
        btnTwo.attachDoubleClick(btn_DobleClickBackHard);
        M_DELAY(10);
      } else if (trainning == 1) {
        times_counter = timeNormal;
        times_hard = LedLevel1;
        isRunning = 1;
        M_DELAY(10);
      }
    } else if (statusSet == 1 && statusHard == 3) {
      if (trainning == 0) {
        setHard();
        btnOne.attachClick(btnClickLevelHard);
        btnTwo.attachClick(btnClick_Trainning);
        btnTwo.attachDoubleClick(btn_DobleClickBackHard);
        M_DELAY(10);
      } else if (trainning == 1) {
        times_counter = timeHard;
        times_hard = LedLevel1;
        isRunning = 1;
        M_DELAY(10);
      }
    }


    // STATUS SET = 2

    if (statusSet == 2 && statusHard == 1) {
      if (trainning == 0) {
        setHard();
        btnOne.attachClick(btnClickLevelHard);
        btnTwo.attachClick(btnClick_Trainning);
        btnTwo.attachDoubleClick(btn_DobleClickBackHard);
        M_DELAY(10);
      } else if (trainning == 1) {
        times_counter = timeEasy;
        times_hard = LedLevel2;
        isRunning = 1;
        M_DELAY(10);
      }
    } else  if (statusSet == 2 && statusHard == 2) {
      if (trainning == 0) {
        setHard();
        btnOne.attachClick(btnClickLevelHard);
        btnTwo.attachClick(btnClick_Trainning);
        btnTwo.attachDoubleClick(btn_DobleClickBackHard);
        M_DELAY(10);
      } else if (trainning == 1) {
        times_counter = timeNormal;
        times_hard = LedLevel2;
        isRunning = 1;
        M_DELAY(10);
      }
    } else if (statusSet == 2 && statusHard == 3) {
      if (trainning == 0) {
        setHard();
        btnOne.attachClick(btnClickLevelHard);
        btnTwo.attachClick(btnClick_Trainning);
        btnTwo.attachDoubleClick(btn_DobleClickBackHard);
      }
      M_DELAY(10);
    } else if (trainning == 1) {
      times_counter = timeHard;
      times_hard = LedLevel2;
      isRunning = 1;
      M_DELAY(10);
    }

    // STATUS SET = 3

    if (statusSet == 3 && statusHard == 1) {
      if (trainning == 0) {
        setHard();
        btnOne.attachClick(btnClickLevelHard);
        btnTwo.attachClick(btnClick_Trainning);
        btnTwo.attachDoubleClick(btn_DobleClickBackHard);
        M_DELAY(10);
      } else if (trainning == 1) {
        times_counter = timeEasy;
        times_hard = LedLevel3;
        isRunning = 1;
        M_DELAY(10);
      }
    } else  if (statusSet == 3 && statusHard == 2) {
      if (trainning == 0) {
        setHard();
        btnOne.attachClick(btnClickLevelHard);
        btnTwo.attachClick(btnClick_Trainning);
        btnTwo.attachDoubleClick(btn_DobleClickBackHard);
        M_DELAY(10);
      } else if (trainning == 1) {
        times_counter = timeNormal;
        times_hard = LedLevel3;
        isRunning = 1;
        M_DELAY(10);
      }
    } else if (statusSet == 3 && statusHard == 3) {
      if (trainning == 0) {
        setHard();
        btnOne.attachClick(btnClickLevelHard);
        btnTwo.attachClick(btnClick_Trainning);
        btnTwo.attachDoubleClick(btn_DobleClickBackHard);
        M_DELAY(10);
      }
    } else if (trainning == 1) {
      times_counter = timeHard;
      times_hard = LedLevel3;
      isRunning = 1;
      M_DELAY(10);
    }
  }
  M_DELAY(0);
  END_CREATE_FUNCTION
}
