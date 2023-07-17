#include "Adafruit_DotStar.h"
#include <SPI.h>
#include <WiFi.h>
#include <AsyncUDP.h>
#include "graphics.h"
#include "security.h"

AsyncUDP udp;

int numDiv = 0;
int stateDiv = 0;
int stateRot = 0;
const int PHOTOPIN = 33;
const int th_SENSORS = 3000; // 赤外線センサの閾値
const int motor_ENCODAPIN = 27;
float rpm;
int value = 0;
volatile unsigned long rotTime, timeOld, timeNow, Time, pre_time; // コンパイル時に無視されないようにvolatile修飾子
const int VDATAPIN = 23;
const int VCLOCKPIN = 18;
// const HDATAPIN = 13;
// const HCLOCKPIN = 14;
const int motorPin = A19;

Adafruit_DotStar vstrip(VNUMPIXELS, VDATAPIN, VCLOCKPIN, DOTSTAR_RGB); // DOTSTATR_BRGなどでも設定可能
// Adafruit_DotStar hstrip(HNUMPIXELS, HDATAPIN, HCLOCKPIN, DOTSTAR_RGB);

char chararrayDiv[] = "0x00";
char chararrayColor[] = "0xffffff";

// 割り込み処理 ボタンを検知
void IRAM_ATTR button_pushed()
{
    value += 1; 
}


void setup()
{
    // Serial通
    Serial.begin(115200);

    // motor
    // 使用するタイマーのチャネルと周波数を設定
    ledcSetup(0, 30000, 8);
    // ledPinをチャネル0へ接続
    ledcAttachPin(motorPin, 0);

    pinMode(motor_ENCODAPIN, INPUT);
    // 割り込みを登録 トリガはLOWになった時
	attachInterrupt(motor_ENCODAPIN, button_pushed, FALLING);

    // WiFi.mode(WIFI_STA);
    // WiFi.begin(ssid, password);
    // if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    //     Serial.println("WiFi Failed");
    //     while (1) {
    //         delay(1000);
    //     }
    // }

    // //UDP受信
    // if (udp.listen(1234)) { //python側とポートを合わせる。自由な数字で良い
    //     Serial.print("UDP Listening on IP: ");
    //     Serial.println(WiFi.localIP());
    //     udp.onPacket([](AsyncUDPPacket packet) {
    //         chararrayDiv[2] = packet.data()[0];
    //         chararrayDiv[3] = packet.data()[1];
    //         //      Serial.print("strtoul=");
    //         //      Serial.println(int(strtoul(chararrayDiv, NULL, 16))); //パケットロスをしらべる
    //         for (int i = 0; i < VNUMPIXELS ; i++) {
    //             for (int j = 0; j < 6 ; j++) {
    //                 chararrayColor[j + 2] = packet.data()[2 + i * 6 + j];
    //             }
    //             vpic[int(strtoul(chararrayDiv, NULL, 16))][i] = strtoul(chararrayColor, NULL, 16);
    //         }
    //     });
    // }


    vstrip.begin();
    // htrip.begin();
    vstrip.show();
    // hstrip.show();
}

void loop()
{
    // Serial.println(analogRead(PHOTOPIN)); // 赤外線センサの動作確認用
    // Serial.println(value);
    ledcWrite(0, 200);
    if(value == 0)
    {
        pre_time = micros();
    }else if (value == 5)
    {
        Time = micros();
        rpm = 60000000 / (Time - pre_time);
        value = 0;
        Serial.print("rpm="); Serial.println(rpm);
    }
    
    

    // 赤外線センサの排他処理, 一回転(赤外線センサが反応してから次に反応するまで)するのにかかる時間(rotTime)の計算
    if (stateRot == 0 && analogRead(PHOTOPIN) < th_SENSORS)
    {
        timeNow = micros();
        rotTime = timeNow - timeOld;
        timeOld = timeNow;
        stateRot = 1;
    }
    if (stateRot == 1 && analogRead(PHOTOPIN) > th_SENSORS)
    {
        stateRot = 0;
        // rpm = 60 * 1000 * 1000 * 1 / rotTime; //rotation per minute
        // Serial.print("rpm="); Serial.println(rpm);
    }

    // 経過時間から光らせるLEDを決める。
    if (stateDiv == 1 && micros() - timeOld < rotTime / Div * (numDiv + 1))
    {
        stateDiv = 0;
    }
    if (stateDiv == 0 && micros() - timeOld > rotTime / Div * (numDiv + 1))
    {
        stateDiv = 1;

        vstrip.clear();
        // hstrip.clear();

        // どのLEDを何色に光らせるのかをset
        for (int i = 0; i < VNUMPIXELS; i++)
        {
            vstrip.setPixelColor(i, pgm_read_dword(&vpic[numDiv][i]));
        }
        // for(int i=0; i<HNUMPIXELS; i++)
        // {
        //     hstrip.setPixelColor(i, pgm_read_dword(&hpic[numDiv+(Div/2)][i])); //半分ずれる
        // }

        // setした通りにLEDを光らせる
        vstrip.show();
        // hstrip.show();

        numDiv++;
        if (numDiv >= Div)
        {
            numDiv = 0;
        }
    }
}

