#include "Adafruit_DotStar.h"
#include <SPI.h>
#include "graphics.h"

int numDiv = 0;
int stateDiv = 0;
int stateRot = 0;
const int PHOTOPIN = 33;
const int th_SENSORS = 3000; // 赤外線センサの閾値
float rpm;
volatile unsigned long rotTime, timeOld, timeNow; // コンパイル時に無視されないようにvolatile修飾子
const int VDATAPIN = 23;
const int VCLOCKPIN = 18;
// const HDATAPIN = 13;
// const HCLOCKPIN = 14;
const int motorPin = A19;
int sw = 0; //人感センサー 回転の有無を決める
int value = 0;

Adafruit_DotStar vstrip(VNUMPIXELS, VDATAPIN, VCLOCKPIN, DOTSTAR_RGB); // DOTSTATR_BRGなどでも設定可能
// Adafruit_DotStar hstrip(HNUMPIXELS, HDATAPIN, HCLOCKPIN, DOTSTAR_RGB);

void setup()
{
    Serial.begin(115200);

    // 使用するタイマーのチャネルと周波数を設定
    ledcSetup(0, 30000, 8);
    // ledPinをチャネル0へ接続
    ledcAttachPin(motorPin, 0);

    vstrip.begin();
    // htrip.begin();
    vstrip.show();
    // hstrip.show();
}

void loop()
{
    Serial.println(analogRead(PHOTOPIN)); // 赤外線センサの動作確認用
    // for(int a=0;a<24*5;a++){
    //     vstrip.setPixelColor(a, 'AA0000');
    //     vstrip.show();
    //     vstrip.clear();
    //     vstrip.show();
    //     delay(30);
    // }
    // if(sw==0){
    //     while(value<80){ //だんだん早くなって80をキープ
    //         ledcWrite(0, value);
    //         value += 1;
    //     }
    // }else{
    //     while(value>0){ //だんだんゆっくりになって0をキープ
    //         ledcWrite(0, value);
    //         value -= 1;
    //     }
    // }
    ledcWrite(0, 200);
    

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
        rpm = 60 * 1000 * 1000 * 1 / rotTime; //rotation per minute
        Serial.print("rpm="); Serial.println(rpm);
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
