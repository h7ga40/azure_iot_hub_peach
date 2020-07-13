# mruby-azure-iot

[Azure IoT C SDKs and Libraries](https://github.com/Azure/azure-iot-sdk-c)の[mruby](https://github.com/mruby/mruby)拡張機能（mrbgem）です。

使用するには、mrubyのビルド設定ファイル```build_config.rb```に以下の行を加えます。

``` ruby
  conf.gem "#{root}/../mrbgems/mruby-azure-iot" do |g|
    g.cc.include_paths  << ["../azure-iot-sdk-c/certs", "../azure-iot-sdk-c/c-utility/inc", "../azure-iot-sdk-c/c-utility/pal/#{platform}", "../azure-iot-sdk-c/iothub_client/inc", "../azure-iot-sdk-c/provisioning_client/inc", "../azure-iot-sdk-c/deps/parson", "../azure-iot-sdk-c/deps/azure-macro-utils-c/inc", "../azure-iot-sdk-c/deps/umock-c/inc", "../azure-iot-sdk-c/serializer/inc"]
  end
```

Azure IoT SDKへのインクルードパスの設定が必要になります。
上記の例は```mruby```フォルダと追加の```mrbgems```フォルダ、```azure-iot-sdk-c```フォルダが同じ階層になるように配置した場合です。

```#{platform}```に、Azure IoTの使いたいプラットフォームのフォルダ名で置き換えます。具体的には```windows```や```linux```です。クローンしたローカルの[ここ](https://github.com/Azure/azure-c-shared-utility/tree/master/pal)の部分にあたるフォルダです。

[使用例](example/client.rb)は、[こ](https://github.com/Azure/azure-iot-sdk-c/blob/master/serializer/samples/simplesample_mqtt/simplesample_mqtt.c)のコードをRuby化したものになります。
