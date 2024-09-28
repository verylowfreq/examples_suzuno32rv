このサンプルコードは、USBメモリーに保存されたJPEG画像を液晶へスライドショー表示します。

使い方：

液晶シールド WaveShare 2.8inch TFT Shield を用意してください。LCDコントローラはST7789です。

USBメモリーにJPEG画像を保存してください。

 - ファイル名は "IMAGE1.JPG", "IMAGE2.JPG" のように、"IMAGE" に 1 から 9 までの数字を付加してください。
 - JPEG画像の解像度は240x320にしてください。縦長です。
 - **プログレッシブJPEGに非対応です** 。

このプロジェクトをPlatformIOでビルドしてください。

`.pio\build\genericCH32V203C8T6` の中に出力された firmware.elf や firmware.bin を書き込んでください。


このプロジェクトでは以下のライブラリを利用しています。

 - picojpeg : マイコン向けのJPEGデコードライブラリ
 - 公式サンプルコードのUSBホスト : USBマスストレージへのアクセスとファイルシステム操作
 - WaveShare TFTシールドライブラリ : 液晶シールドへの描画
