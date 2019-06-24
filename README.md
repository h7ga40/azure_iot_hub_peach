# Azure IoT Hub with TOPPERS/ASP3

RTOSの[TOPPERS/ASP3](https://www.toppers.jp/asp3-kernel.html)を使ったAzure IoT Hub へ接続する[Azure IoT Hub Device C SDK](https://github.com/Azure/azure-iot-sdk-c)を使ったサンプルです。
ターゲットデバイスは、[GR-PEACH](http://gadget.renesas.com/ja/product/peach.html)で、Ethernet上のHTTPかMQTTで通信します。 MQTTは、mbedTLSを使用した場合に接続できます。

注意点として、「get_time」という関数が「c-utility/adapters/agenttime_mbed.c」に定義されていますが、ベースのソフトにも定義されているので、Azure IoT SDK側の「get_time」関数を「azure_get_time」などにリネームして競合を避ける必要があります。

サンプルを動作させるには、[この手順](https://docs.microsoft.com/ja-jp/azure/iot-hub/quickstart-send-telemetry-c)や[この手順](https://github.com/ms-iotkithol-jp/IoTKitHoLV4)を参照にAzure側にデバイスを追加し、そのデバイスの接続文字列を取得し、ソースコード「app_iothub_client/src/client.c」の以下の個所を書き換える必要があります。

```c
static const char* connectionString = "[device connection string]";
```

## 開発環境

ビルドはGCC+Makefileで行います。arm向けGCCは[こちら](https://gcc-renesas.com/ja/rz/rz-download-toolchains/)や[こちら](https://launchpad.net/gcc-arm-embedded)から入手できます。Windows向けのVSCodeの設定は**6 2017-q2-update**のバージョンに対応しています。

IDEとして[e² studio](https://www.renesas.com/ja-jp/products/software-tools/tools/ide/e2studio.html) V7.4.0か[VSCodec](https://code.visualstudio.com/)を使用します。

Rubyの実行環境が必要なので、​[こちら](https://www.ruby-lang.org/ja/downloads/)からダウンロードして、コマンドプロントから実行できるよう、環境変数でパスを通してください。

デバッグするには[OpenOCD](https://github.com/gnu-mcu-eclipse/openocd/releases)が必要で、バージョンは**0.10.0-8-20180512-1921**以降で、VSCodeの設定ファイルは**0.10.0-12-20190422-2015**を対象にしています。「*asp3_dcre\target\gr_peach_gcc\renesas_rza1h_swd.cfg*」を、OpenOCDインストールフォルダの下の「*scripts\target*」にコピーします。

e² studioを使う場合は、展開したフォルダをワークスペースとして開き、プロジェクトのインポートをこのフォルダに対して行うことで、取り込むことができます。
開発手順は[ここ](https://dev.toppers.jp/trac_user/contrib/wiki/azure_iot_hub_peach)にあります。

VSCodeの場合は、「ファイル」メニューの「ワークスペースを開く」から、ルートにある「*azure_iot_hub_peach.code-workspace*」を開きます。必要に応じてワークスペースファイルや「*.vscode*」フォルダにあるファイルのパスを変更してください。

## フォルダ構成

|パス|内容|ライセンス|
|:---|:---|:---|
|app_iothub_client|サンプルプログラム|TOPPERS License|
|asp3_dcre|RTOS [TOPPERS/ASP3](http://www.toppers.jp/asp3-kernel.html)|TOPPERS License|
|asp3_dcre/tinet|TCPプロトコルスタック [TINET 1.7](http://www.toppers.jp/tinet.html)|TOPPERS License|
|asp3_dcre/mbed|[​mbed](https://www.mbed.com/) HAL APIの利用と実装|Apache 2.0 License|
|asp3_dcre/syssvc/tlsf|メモリアロケータ [tlsf](https://github.com/mattconte/tlsf)|BSD License|
|azure_iothub|[Azure IoT Hub Device C SDK](https://github.com/Azure/azure-iot-sdk-c) (tag 2019-04-11)|MIT License|
|azure_iothub/deps/parson|jsonパーサー [parson](https://github.com/kgabis/parson)|MIT License|
|azure_iothub/c-utility/adapters|TOPPERS向けの実装を追加|TOPPERS License|
|curl-7.57.0|HTTPクライアント[cURL](https://curl.haxx.se/)|Curl License|
|musl-1.1.18|標準Cライブラリ[musl](http://www.musl-libc.org/)|MIT License|
|ntshell/fatfs|FATファイルシステム [FatFs](http://elm-chan.org/fsw/ff/00index_e.html)|FatFs License|
|ntshell/ntshell|仮想ターミナル [NT-Shell](https://www.cubeatsystems.com/ntshell/)|MIT License etc.|
|ntshell/src|Socket API、標準入出力、ファイルディスクリプタ|TOPPERS License|
|wolfssl-3.15.7|SSL/TLS ライブラリ [wolfSSL](https://www.wolfssl.com/) 現在未使用|GPL/商用|
|mbedtls-2.16.1|SSL/TLS ライブラリ [mdebTLS](https://tls.mbed.org/)|Apache 2.0 License|
|zlib-1.2.11|圧縮・伸張ライブラリ [zlib](https://www.zlib.net/)|zlib License|

ライセンスは参考のため載せておきました。正確なライセンスは各ソフトウェアのサイトを確認してください。

|確認|プロトコル|ソフトスタック|
|-|-|-|
|:heavy_check_mark:|HTTP|iothubtransporthttp.c→httpapi_compact.c→tlsio_wolfssl.c→socketio_berkeley.c|
|:heavy_multiplication_x:|HTTP|iothubtransporthttp.c→httpapi_compact.c→tlsio_mbedtls.c→socketio_berkeley.c|
|:heavy_check_mark:|HTTP|iothubtransporthttp.c→httpapi_curl.c→Curl＋WolfSSL|
|:heavy_multiplication_x:|HTTP|iothubtransporthttp.c→httpapi_curl.c→Curl＋mbedSSL|
|:heavy_multiplication_x:|MQTT|iothubtransportmqtt.c→wsio.c→umqtt→tlsio_wolfssl.c→socketio_berkeley.c|
|:heavy_check_mark:|MQTT|iothubtransportmqtt.c→wsio.c→umqtt→tlsio_mbedtls.c→socketio_berkeley.c|
|:heavy_multiplication_x:|MQTT over WebSocket|iothubtransportmqtt_websockets.c→wsio.c→umqtt→tlsio_wolfssl.c→socketio_berkeley.c|
|:heavy_check_mark:|MQTT over WebSocket|iothubtransportmqtt_websockets.c→wsio.c→umqtt→tlsio_mbedtls.c→socketio_berkeley.c|
|:heavy_minus_sign:|AMQP||
|:heavy_minus_sign:|AMQP over WebSocket||

デフォルトでは、「MQTT/mbedTLS」が有効になっています。

### wolfSSLに変更するには

app_iothub_client\Debug\Makefile(60)

```make
$(SRCDIR)/../mbedtls-2.16.1/Debug/libmbedtls.a \
```

を

```make
$(SRCDIR)/../wolfssl-3.15.7/Debug/libwolfssl.a \
```

に変更。

「*azure_iothub*」プロジェクトのコンパイルオプションに、「USE_WOLFSSL=1」を指定。
「*tlsio_mbedtls.c*」で右クリックし、「リソース構成」→「ビルドから除外」を選択し、表示された画面で、「すべて選択」ボタンを押し「OK」を押す。
「*tlsio_wolfssl.c*」で右クリックし、「リソース構成」→「ビルドから除外」を選択し、表示された画面で、「選択をすべて解除」ボタンを押し「OK」を押す。

azure_iothub/c-utility/adapters/platform_toppers.c(55)

```c
return tlsio_mbedtls_get_interface_description();
```

を

```c
return tlsio_wolfssl_get_interface_description();
```

に変更。

curl-7.57.0/lib/curl_config.h(930)

```c
/* #undef USE_CYASSL */
```

を

```c
#define USE_CYASSL 1
```

に変更。

curl-7.57.0/lib/curl_config.h(954)

```c
#define USE_MBEDTLS 1
```

を

```c
/* #undef USE_MBEDTLS */
```

に変更。

## Visual Studio Code対応

以下の記事を参考にしました。
https://os.mbed.com/users/MACRUM/notebook/how-to-setup-vscode-debugging-for--stm32-platforms/
https://qiita.com/mitsu48/items/5c6fec6064af6c4a2c4e

## TOPPERS License

TOPPERS Licenseは、BSDライクなライセンスです。詳しくは、下記のサイトを見てください。

https://www.toppers.jp/license.html

## 謝辞

日本マイクロソフトの太田様には、Azure IoT Hubの操作や設定、メッセージの確認方法など、詳しく教えて頂きました。ありがとうございました。

 :baby_chick: Twiter [@embedded_george](https://twitter.com/embedded_george), :octocat: Github [ms-iotkithol-jp](https://github.com/ms-iotkithol-jp)
