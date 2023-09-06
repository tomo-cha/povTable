#include "Adafruit_DotStar.h"
#include <SPI.h>
#include "WiFi.h"
#include "AsyncUDP.h"
#include "security.h"

// #define DEBUG

AsyncUDP udp;

const int VNUMPIXELS = 22*5;
const int Div = 60;
const int Frame = 5;

uint32_t vpic[Div][VNUMPIXELS] = {0, };

int stateRot = 0;
int numDiv = 0;
const int PHOTOPIN = 33;
const int th_PHOTOSENSORS = 1000; // 赤外線センサの閾値
const int motor_ENCODAPIN = 27;
float rpm;
int value = 0;
volatile unsigned long rotTime, timeOld, Time, pre_time; // コンパイル時に無視されないようにvolatile修飾子
const int VDATAPIN = 23;
const int VCLOCKPIN = 18;
const int HDATAPIN = 13;
const int HCLOCKPIN = 14;
const int HNUMPIXELS = VNUMPIXELS;
const int motorPin = A19; // gpio26

char chararrayDiv[] = "0x00";
char chararrayColor[] = "0xffffff";

Adafruit_DotStar vstrip(VNUMPIXELS, VDATAPIN, VCLOCKPIN, DOTSTAR_GBR); // DOTSTATR_BRGなどでも設定可能 RGB, RBG, BRG,
Adafruit_DotStar hstrip(HNUMPIXELS, HDATAPIN, HCLOCKPIN, DOTSTAR_GBR);

// 割り込み処理 ボタンを検知
void IRAM_ATTR button_pushed()
{
    value += 1;
}

void setup()
{
    // Serial通信
    Serial.begin(115200);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
        Serial.println("WiFi Failed");
        while (1)
        {
            delay(1000);
        }
    }

    // UDP受信
    if (udp.listen(1234)) // python側とポートを合わせる。自由な数字で良い
    {
        Serial.print("UDP Listening on IP: ");
        Serial.println(WiFi.localIP());
        udp.onPacket([](AsyncUDPPacket packet)
                     {
                         chararrayDiv[2] = packet.data()[0];
                         chararrayDiv[3] = packet.data()[1];
                         Serial.print("strtoul=");
                         Serial.println(int(strtoul(chararrayDiv, NULL, 16))); // パケットロスをしらべる
                         // for (int k = 0; k < Frame; k++) { //gif用
                         for (int i = 0; i < VNUMPIXELS; i++)
                         {
                             for (int j = 0; j < 6; j++)
                             {
                                 chararrayColor[j + 2] = packet.data()[2 + i * 6 + j];
                             }
                             vpic[int(strtoul(chararrayDiv, NULL, 16))][i] = strtoul(chararrayColor, NULL, 16);
                         }
                         //      }
                     });
    }

    // モーター
    // 使用するタイマーのチャネルと周波数を設定
    ledcSetup(0, 30000, 8);
    // motorPinをチャネル0へ接続
    ledcAttachPin(motorPin, 0);

    pinMode(motor_ENCODAPIN, INPUT);

    // 割り込みを登録 トリガはLOWになった時
    attachInterrupt(motor_ENCODAPIN, button_pushed, FALLING);

    vstrip.begin();
    hstrip.begin();
    vstrip.clear();
    hstrip.clear();
    vstrip.show();
    hstrip.show();

    delay(1000);
}

void loop()
{
    int start = micros();
    // モーターエンコーダからrpm,rotTimeの計測
    if (value == 0)
    {
        pre_time = micros();
    }
    else if (value >= 5)
    {
        Time = micros();
        rpm = 60000000 / (Time - pre_time);
        rotTime = Time - pre_time;
        value = 0;
    }

    // フォロリフレクタの反応時の処理(排他処理をしている)
    if (stateRot == 0 && analogRead(PHOTOPIN) < th_PHOTOSENSORS) // 閾値以下=反応
    {
        timeOld = micros();
        numDiv = 0;
        stateRot = 1;
        // numRot++;
        // if (numRot >= Frame)
        // {
        //     numRot = 0;
        // }
    }
    if (stateRot == 1 && analogRead(PHOTOPIN) > th_PHOTOSENSORS)
    {
        stateRot = 0;
    }

    // 常に光らせておく
    // 何番目のLEDを何色に光らせるのかをset
    for (int i = 0; i < VNUMPIXELS; i++)
    {
        vstrip.setPixelColor(i, pgm_read_dword(&vpic[numDiv][i]));
        hstrip.setPixelColor(i, pgm_read_dword(&vpic[(numDiv + (Div / 2)) % Div][i]));
    }

    // setした通りにLEDを光らせる
    vstrip.show();
    hstrip.show();
    // 経過時間から何番目のパターンを光らせるか決める。
    if (micros() - timeOld > rotTime / Div * (numDiv + 1))
    {
        numDiv++;
        if (numDiv >= Div)
        {
            numDiv = 0;
        }
    }

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
    Serial.print("vpic = ");
    Serial.println(vpic[Div][VNUMPIXELS]);
#endif // DEBUG
    int goal = micros();
    // Serial.print("goal - start = ");
    // Serial.println(goal - start);
}
