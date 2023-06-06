# -*- coding: utf-8 -*-
# 画像一枚を1つのファイルに変換するコード
import cv2
import os
import math
import sys
from PIL import Image


# args = sys.argv

# 配列設定
PIXELS = 24  #LED1本あたりのセル数
NUMTAPES = 2    #繋げるLEDの本数
Div = 100       #1周の分割数
l = [[0] * PIXELS*NUMTAPES for i in range(Div)] #RGBを格納するためのリスト宣言・初期化

Bright = 20     # 輝度
Led0Bright = 2  # 中心LEDの輝度 [%]

# ファイル作成
file = open('graphics.h', 'w')
file.write('const int VNUMPIXELS = ' + str(PIXELS*NUMTAPES) + ';\n') #VNUMPIXELSかHNUMPIXELSに変更。
file.write('const int Div = ' + str(Div) + ';\n' + '\n')


file.write('const uint32_t vpic [Div][VNUMPIXELS] = {' + '\n') #vpicかhpicに変更。#VNUMPIXELSかHNUMPIXELSに変更。

# 画像ファイルを読み込む(png,jpg,bmpなどが使用可能)
pic1 = "monstar.png" #1枚目の画像
pic2 = "monstar.png" #2枚目の画像


# 画像変換関数
def polarConv(pic, n):
    # 画像データ読み込み
    imgOrgin = cv2.imread(pic)

    # 画像サイズ取得
    h, w, _ = imgOrgin.shape

    # 画像縮小
    # 画像のh:画像のw = (PIXELS * 2 -1):?. の比を取って縮小してる
    imgRedu = cv2.resize(imgOrgin, (math.floor(
        (PIXELS * 2 - 1)/h * w), PIXELS * 2 - 1))

    # 縮小画像中心座標
    h2, w2, _ = imgRedu.shape
    wC = math.floor(w2 / 2)
    hC = math.floor(h2 / 2)

    # 極座標変換画像準備
    # 第二引数にサイズ、第三引数にRGBの各色を指定する。
    # imgPolar = Image.new('RGB', (PIXELS, Div))

    # 極座標変換
    for j in range(0, Div):
        # file.write('\t{')
        for i in range(0, hC+1):
            # 座標色取得
            # 参考：http://peaceandhilightandpython.hatenablog.com/entry/2016/01/03/151320
            rP = int(imgRedu[hC + math.ceil(i * math.cos(2*math.pi/Div*j)),
                             wC - math.ceil(i * math.sin(2*math.pi/Div*j)), 2]  # Rを取得
                     * ((100 - Led0Bright) / PIXELS * i + Led0Bright) / 100 * Bright / 100)  # 明るさ調整
            gP = int(imgRedu[hC + math.ceil(i * math.cos(2*math.pi/Div*j)),
                             wC - math.ceil(i * math.sin(2*math.pi/Div*j)), 1]  # Gを取得
                     * ((100 - Led0Bright) / PIXELS * i + Led0Bright) / 100 * Bright / 100)  # 明るさ調整
            bP = int(imgRedu[hC + math.ceil(i * math.cos(2*math.pi/Div*j)),
                             wC - math.ceil(i * math.sin(2*math.pi/Div*j)), 0]  # Bを取得
                     * ((100 - Led0Bright) / PIXELS * i + Led0Bright) / 100 * Bright / 100)  # 明るさ調整
            
            # https://yutarine.blogspot.com/2018/12/python-format-hex-zerofill.html
            # https://docs.python.org/ja/3/tutorial/inputoutput.html
            if(n%2 == 1):
                l[j][i] = '0x{:02X}{:02X}{:02X}'.format(rP, gP, bP)
            else:
                l[j][abs((PIXELS*n-1)-i)] = '0x{:02X}{:02X}{:02X}'.format(rP, gP, bP)

            # imgPolar.putpixel((i, j), (rP, gP, bP))

# 変換, lに格納
polarConv(pic1, 1) #0~23番目のセル l[j][i]
polarConv(pic2, 2) #47~24番目のセル l[j][47-i]
# polarConv(pic3, 3) #48~71番目のセル l[j][i+48]
# polarConv(pic4, 4) #95~72番目のセル l[j][96-i]

#lの内容を書き出し
for j in range(0, Div):
    file.write('\t{')
    for i in range(0, PIXELS*NUMTAPES):
        file.write(l[j][i])
        if i == PIXELS*NUMTAPES-1:
            file.write('},\n')
        else:
            file.write(', ')
file.write('};\n')


file.close()
