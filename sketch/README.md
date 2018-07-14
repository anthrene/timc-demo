#Develop Environment

Arduino IDE, 1.8.5

Arduino RFID Library for MFRC522, master branch, https://github.com/miguelbalboa/rfid
#Arduino IDEからインストールできるバージョンでは動かないので、GitHubのmaster branchをインストールすること。
#インストール先は以下。(Aruduino IDEで一度最新版インストール後、ディレクトリ配下全て削除し、git cloneが実績あり)
#C:\Users\"User Name"\Documents\Arduino\libraries\MFRC522

Arduino core for the ESP32, master branch, https://github.com/espressif/arduino-esp32
#Arduino IDEからインストールできないので、GitHubのmaster branchをインストールすること。
#インストール先は以下。
#C:\Program Files (x86)\Arduino\hardware\espressif\esp32

AquesTalk pico for ESP32, 1.0.0, https://www.a-quest.com/download.html#a-etc
#インストール先とファイルは以下。
#C:\Program Files (x86)\Arduino\hardware\espressif\esp32\tools\sdk\lib\libaquestalk.a
#C:\Program Files (x86)\Arduino\hardware\espressif\esp32\tools\sdk\include\aquestalk\aquestalk.h
#リンクの手順通りに下記ファイルを作成。
#C:\Program Files (x86)\Arduino\hardware\espressif\esp32\platform.local.txt
