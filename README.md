# Azure IoT Hub with TOPPERS/ASP3

RTOSの[TOPPERS/ASP3](https://www.toppers.jp/asp3-kernel.html)を使ったAzure IoT Hub へ接続する[Azure IoT Hub Device C SDK](https://github.com/Azure/azure-iot-sdk-c)を使ったサンプルです。
ターゲットデバイスは、[GR-PEACH](http://gadget.renesas.com/ja/product/peach.html)で、Ethernet上のHTTPかMQTTで通信します。 MQTTは、mbedTLSを使用した場合に接続できます。

[GR-PEACH](http://gadget.renesas.com/ja/product/peach.html)でも動作確認できています。ベースのソフトに[TOPPERS/ASP3版ECNL](http://dev.toppers.jp/trac_user/contrib/wiki/asp3_tinet_ecnl_rx)を使用します。注意点として、「get_time」という関数が「c-utility/adapters/agenttime_mbed.c」に定義されていますが、ベースのソフトにも定義されているので、Azure IoT SDK側の「get_time」関数を「azure_get_time」などにリネームして競合を避ける必要があります。

サンプルを動作させるには、[この手順](https://docs.microsoft.com/ja-jp/azure/iot-hub/quickstart-send-telemetry-c)や[この手順](https://github.com/ms-iotkithol-jp/IoTKitHoLV4)を参照にAzure側にデバイスを追加し、そのデバイスの接続文字列を取得し、ソースコード「app_iothub_client/src/client.c」の以下の個所を書き換える必要があります。

```c
static const char* connectionString = "[device connection string]";
```

## 開発環境

ビルドはGCC+Makefileで行います。RX向けGCCは[こちら](https://gcc-renesas.com/ja/rx-download-toolchains/)から入手できます。
IDEとして[e² studio V7.4.0](https://www.renesas.com/ja-jp/products/software-tools/tools/ide/e2studio.html)を使用しました。
Rubyの実行環境が必要なので、​[こちら](https://www.ruby-lang.org/ja/downloads/)からダウンロードして、コマンドプロントから実行できるよう、環境変数でパスを通してください。
e² studioのプロジェクトファイルも含まれているので、展開したフォルダをワークスペースとして開き、プロジェクトのインポートをこのフォルダに対して行うことで、取り込むことができます。
開発手順は[ここ](https://dev.toppers.jp/trac_user/contrib/wiki/azure_iot_hub)にあります。

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
```
$(SRCDIR)/../mbedtls-2.16.1/Debug/libmbedtls.a \
```
を
```
$(SRCDIR)/../wolfssl-3.15.7/Debug/libwolfssl.a \
```
に変更。

「azure_iothub」プロジェクトのコンパイルオプションに、「USE_WOLFSSL=1」を指定

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

## TOPPERS License

TOPPERS Licenseは、BSDライクなライセンスです。詳しくは、下記のサイトを見てください。

https://www.toppers.jp/license.html

## 謝辞

日本マイクロソフトの太田様には、Azure IoT Hubの操作や設定、メッセージの確認方法など、詳しく教えて頂きました。ありがとうございました。

 ![Twiter]("data:image/svg+xml;base64,PHN2ZyBpZD0iTG9nb19GSVhFRCIgZGF0YS1uYW1lPSJMb2dvIOKAlCBGSVhFRCIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIiB2aWV3Qm94PSIwIDAgNDAwIDQwMCI+PGRlZnM+PHN0eWxlPi5jbHMtMXtmaWxsOm5vbmU7fS5jbHMtMntmaWxsOiMxZGExZjI7fTwvc3R5bGU+PC9kZWZzPjx0aXRsZT5Ud2l0dGVyX0xvZ29fQmx1ZTwvdGl0bGU+PHJlY3QgY2xhc3M9ImNscy0xIiB3aWR0aD0iNDAwIiBoZWlnaHQ9IjQwMCIvPjxwYXRoIGNsYXNzPSJjbHMtMiIgZD0iTTE1My42MiwzMDEuNTljOTQuMzQsMCwxNDUuOTQtNzguMTYsMTQ1Ljk0LTE0NS45NCwwLTIuMjIsMC00LjQzLS4xNS02LjYzQTEwNC4zNiwxMDQuMzYsMCwwLDAsMzI1LDEyMi40N2ExMDIuMzgsMTAyLjM4LDAsMCwxLTI5LjQ2LDguMDcsNTEuNDcsNTEuNDcsMCwwLDAsMjIuNTUtMjguMzcsMTAyLjc5LDEwMi43OSwwLDAsMS0zMi41NywxMi40NSw1MS4zNCw1MS4zNCwwLDAsMC04Ny40MSw0Ni43OEExNDUuNjIsMTQ1LjYyLDAsMCwxLDkyLjQsMTA3LjgxYTUxLjMzLDUxLjMzLDAsMCwwLDE1Ljg4LDY4LjQ3QTUwLjkxLDUwLjkxLDAsMCwxLDg1LDE2OS44NmMwLC4yMSwwLC40MywwLC42NWE1MS4zMSw1MS4zMSwwLDAsMCw0MS4xNSw1MC4yOCw1MS4yMSw1MS4yMSwwLDAsMS0yMy4xNi44OCw1MS4zNSw1MS4zNSwwLDAsMCw0Ny45MiwzNS42MiwxMDIuOTIsMTAyLjkyLDAsMCwxLTYzLjcsMjJBMTA0LjQxLDEwNC40MSwwLDAsMSw3NSwyNzguNTVhMTQ1LjIxLDE0NS4yMSwwLDAsMCw3OC42MiwyMyIvPjwvc3ZnPg==")@embedded_george
 ![GitHub]("data:image/svg+xml;base64,PHN2ZyBhcmlhLWhpZGRlbj0idHJ1ZSIgZm9jdXNhYmxlPSJmYWxzZSIgZGF0YS1wcmVmaXg9ImZhYiIgZGF0YS1pY29uPSJnaXRodWItc3F1YXJlIiByb2xlPSJpbWciIHhtbG5zPSJodHRwOi8vd3d3LnczLm9yZy8yMDAwL3N2ZyIgdmlld0JveD0iMCAwIDQ0OCA1MTIiIGNsYXNzPSJzdmctaW5saW5lLS1mYSBmYS1naXRodWItc3F1YXJlIGZhLXctMTQgZmEtMngiPjxwYXRoIGZpbGw9ImN1cnJlbnRDb2xvciIgZD0iTTQwMCAzMkg0OEMyMS41IDMyIDAgNTMuNSAwIDgwdjM1MmMwIDI2LjUgMjEuNSA0OCA0OCA0OGgzNTJjMjYuNSAwIDQ4LTIxLjUgNDgtNDhWODBjMC0yNi41LTIxLjUtNDgtNDgtNDh6TTI3Ny4zIDQxNS43Yy04LjQgMS41LTExLjUtMy43LTExLjUtOCAwLTUuNC4yLTMzIC4yLTU1LjMgMC0xNS42LTUuMi0yNS41LTExLjMtMzAuNyAzNy00LjEgNzYtOS4yIDc2LTczLjEgMC0xOC4yLTYuNS0yNy4zLTE3LjEtMzkgMS43LTQuMyA3LjQtMjItMS43LTQ1LTEzLjktNC4zLTQ1LjcgMTcuOS00NS43IDE3LjktMTMuMi0zLjctMjcuNS01LjYtNDEuNi01LjYtMTQuMSAwLTI4LjQgMS45LTQxLjYgNS42IDAgMC0zMS44LTIyLjItNDUuNy0xNy45LTkuMSAyMi45LTMuNSA0MC42LTEuNyA0NS0xMC42IDExLjctMTUuNiAyMC44LTE1LjYgMzkgMCA2My42IDM3LjMgNjkgNzQuMyA3My4xLTQuOCA0LjMtOS4xIDExLjctMTAuNiAyMi4zLTkuNSA0LjMtMzMuOCAxMS43LTQ4LjMtMTMuOS05LjEtMTUuOC0yNS41LTE3LjEtMjUuNS0xNy4xLTE2LjItLjItMS4xIDEwLjItMS4xIDEwLjIgMTAuOCA1IDE4LjQgMjQuMiAxOC40IDI0LjIgOS43IDI5LjcgNTYuMSAxOS43IDU2LjEgMTkuNyAwIDEzLjkuMiAzNi41LjIgNDAuNiAwIDQuMy0zIDkuNS0xMS41IDgtNjYtMjIuMS0xMTIuMi04NC45LTExMi4yLTE1OC4zIDAtOTEuOCA3MC4yLTE2MS41IDE2Mi0xNjEuNVMzODggMTY1LjYgMzg4IDI1Ny40Yy4xIDczLjQtNDQuNyAxMzYuMy0xMTAuNyAxNTguM3ptLTk4LjEtNjEuMWMtMS45LjQtMy43LS40LTMuOS0xLjctLjItMS41IDEuMS0yLjggMy0zLjIgMS45LS4yIDMuNy42IDMuOSAxLjkuMyAxLjMtMSAyLjYtMyAzem0tOS41LS45YzAgMS4zLTEuNSAyLjQtMy41IDIuNC0yLjIuMi0zLjctLjktMy43LTIuNCAwLTEuMyAxLjUtMi40IDMuNS0yLjQgMS45LS4yIDMuNy45IDMuNyAyLjR6bS0xMy43LTEuMWMtLjQgMS4zLTIuNCAxLjktNC4xIDEuMy0xLjktLjQtMy4yLTEuOS0yLjgtMy4yLjQtMS4zIDIuNC0xLjkgNC4xLTEuNSAyIC42IDMuMyAyLjEgMi44IDMuNHptLTEyLjMtNS40Yy0uOSAxLjEtMi44LjktNC4zLS42LTEuNS0xLjMtMS45LTMuMi0uOS00LjEuOS0xLjEgMi44LS45IDQuMy42IDEuMyAxLjMgMS44IDMuMy45IDQuMXptLTkuMS05LjFjLS45LjYtMi42IDAtMy43LTEuNXMtMS4xLTMuMiAwLTMuOWMxLjEtLjkgMi44LS4yIDMuNyAxLjMgMS4xIDEuNSAxLjEgMy4zIDAgNC4xem0tNi41LTkuN2MtLjkuOS0yLjQuNC0zLjUtLjYtMS4xLTEuMy0xLjMtMi44LS40LTMuNS45LS45IDIuNC0uNCAzLjUuNiAxLjEgMS4zIDEuMyAyLjguNCAzLjV6bS02LjctNy40Yy0uNC45LTEuNyAxLjEtMi44LjQtMS4zLS42LTEuOS0xLjctMS41LTIuNi40LS42IDEuNS0uOSAyLjgtLjQgMS4zLjcgMS45IDEuOCAxLjUgMi42eiIgY2xhc3M9IiI+PC9wYXRoPjwvc3ZnPg==")

