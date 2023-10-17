// udp通信を使わないコード
// 画像が回転してしまう 2023.10.04時点
// 10.20-29 展示用
// 時間ごとに2枚の画像を切り替え
#include "Adafruit_DotStar.h"
#include <SPI.h>
#include "graphics_tenji.h"
#include "security.h"

// #define DEBUG

int stateRot = 0;
int stateDiv = 0;
int numDiv = 0;
int numFrame = 1;
const int PHOTOPIN = 33;
const int th_PHOTOSENSORS = 3000; // 赤外線センサの閾値
float rpm;
volatile unsigned long rotTime, timeOld, timeNow; // コンパイル時に無視されないようにvolatile修飾子
const int VDATAPIN = 23;
const int VCLOCKPIN = 18;
const int HDATAPIN = 13;
const int HCLOCKPIN = 14;
const int HNUMPIXELS = VNUMPIXELS;
const int motorPin = A19; // gpio26

Adafruit_DotStar vstrip(VNUMPIXELS, VDATAPIN, VCLOCKPIN, DOTSTAR_BGR); // DOTSTATR_BRGなどでも設定可能 RGB, RBG, BRG,
Adafruit_DotStar hstrip(HNUMPIXELS, HDATAPIN, HCLOCKPIN, DOTSTAR_BGR);

void setup()
{
    // Serial通信
    Serial.begin(115200);

    attachInterrupt(digitalPinToInterrupt(33), RotCount, FALLING );

    // モーター
    // 使用するタイマーのチャネルと周波数を設定
    ledcSetup(0, 30000, 8); // 30kHz
    // motorPinをチャネル0へ接続
    ledcAttachPin(motorPin, 0);

    // LED tape
    vstrip.begin();
    hstrip.begin();
    vstrip.clear();
    hstrip.clear();
    vstrip.show();
    hstrip.show();
}

void loop()
{
    int start = micros(); // 処理速度計算

    rotate(); // 回転
    pov();    // LED

#ifdef DEBUG
    // Serial.print("PHOTOPIN = ");
    // Serial.println(analogRead(PHOTOPIN)); // 赤外線センサの動作確認用
    // Serial.print("value = ");
    // Serial.println(value); // モータエンコーダの動作確認用
    // Serial.print("rotTime = ");
    // Serial.println(rotTime);
    // Serial.print("rpm=");
    // Serial.println(rpm);
    // Serial.print("numDiv = ");
    // Serial.println(numDiv);
    // Serial.print("vpic = ");
    // Serial.println(vpic[Div][VNUMPIXELS]);
#endif // DEBUG
    int goal = micros();
    Serial.print("goal - start = ");
    Serial.println(goal - start);
}

void rotate()
{
    ledcWrite(0, 100);
}

void pov()
{
    Serial.println(analogRead(PHOTOPIN));
    // フォロリフレクタの反応時の処理(排他処理をしている)
    // if (stateRot == 0 && analogRead(PHOTOPIN) < th_PHOTOSENSORS) // 閾値以下=反応
    // {
    //     timeNow = micros();
    //     rotTime = timeNow - timeOld;
    //     timeOld = timeNow;
    //     // rpm = 60000000 / rotTime;
    //     // numDiv = 0; // フォトリフレクタの位置を描画開始場所にする
    //     stateRot = 1;
    // }
    // if (stateRot == 1 && analogRead(PHOTOPIN) > th_PHOTOSENSORS)
    // {
    //     stateRot = 0;
    // }

    // 常に光らせておく設定
    // rotTimeの変化のあるなしで静止時には光らないようにできそう
    // 何番目のLEDを何色に光らせるのかをset
    if (stateDiv == 1 && micros() - timeOld < rotTime / Div * (numDiv + 1))
    {
        stateDiv = 0;
    }
    if (stateDiv == 0 && micros() - timeOld > rotTime / Div * (numDiv + 1))
    {
        stateDiv = 1;
        vstrip.clear();
        hstrip.clear(); // 一つ前の点灯パターンを消さないとそのまま残る。これがないと画像が回転しているように見える

        for (int i = 0; i < VNUMPIXELS; i++)
        {
            vstrip.setPixelColor(i, vpic[numFrame][numDiv][i]);
            hstrip.setPixelColor(i, vpic[numFrame][(numDiv + (Div / 2)) % Div][i]); // 半分ずれる
        }

        // setした通りにLEDを光らせる
        vstrip.show();
        hstrip.show();

        // 経過時間から何番目のパターンを光らせるか決める。
        numDiv++;
        if (numDiv >= Div - 1)
        {
            numDiv = 0;
        }
    }
}

void RotCount()
{
    timeNow = micros();
    rotTime = timeNow - timeOld;
    timeOld = timeNow;
}