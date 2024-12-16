# Read RIDF Files

This section explains how to read RIDF files using `artemis`.
Currently, the binary files in RIDF format are processed using two classes: `art::TRIDFEventStore` and `art::TMappingProcessor`.

## Using `art::TRIDFEventStore` to Read Data

To load data from an RIDF file as input, use `art::TRIDFEventStore`.
Below is an example of a steering file:

```yaml
Anchor:
  - &input ridf/@NAME@@NUM@.ridf
  - &output output/@NAME@/@NUM@/hoge@NAME@@NUM@.root
  - &histout output/@NAME@/@NUM@/hoge@NAME@@NUM@.hist.root

Processor:
  - name: timer
    type: art::TTimerProcessor

  - name: ridf
    type: art::TRIDFEventStore
    parameter:
      OutputTransparency: 1
      Verbose: 1
      MaxEventNum: 100000
      SHMID: 0
      InputFiles:
        - *input

  - name: outputtree
    type: art::TOutputTreeProcessor
    parameter:
      FileName:
        - *output
```

The `timer` processor is used to display the analysis time and is conventionally included in all processors.
Focus on the `ridf` block below it.

**Key Parameters**

| Parameter              | Defalut Value | Description                                                                                                                                                                   |
| ---------------------- | ------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **OutputTransparency** | 0             | 0 ensures that outputs from this processor persist (saved in the ROOT file). 1 allows usage within the artemis process but does not persist. (inherit from `art::TProcessor`) |
| **Verbose**            | 1             | 0 enables quiet mode; 1 outputs detailed logs. (inherit from `art::TProcessor`)                                                                                               |
| **MaxEventNum**        | 0             | 0 means no limit. If a value is specified, it limits the number of loops to the specified number.                                                                             |
| **SHMID**              | 0             | The Shared Memory ID used when DAQ starts in online mode (babirl `nssta` mode) to temporarily store data.                                                                     |
| **InputFiles**         | Empty array   | Specifies paths to input RIDF files as an array. Not used in online mode. When multiple files are listed, they are processed consecutively to create one ROOT file.           |

パラメータを何も指定しない場合に、デフォルト値が用いられます。
また、`art::TProcessor`から継承されているパラメータは全てのプロセッサに共通するパラメータであることに注意してください。

### `art::TRIDFEventStore`の処理

通常このプロセッサで処理された後のオブジェクトそのままでは扱いにくいので、`OutputTransparency`を 1 に設定し、ROOT ファイルに書き込まないことが多いです。
しかし、具体的な処理を見るために、0 にするとどういったものが出力されるかをみてみます。

```shell
artlogin <username>
a
```

```shell
artemis [] add steering/hoge.yaml NAME=xxxx NUM=xxxx
artemis [] res
artemis [] sus
artemis [] fcd 0
artemis [] br
```

基本的な`artemis`のコマンドについては[Artemis Commands の説](../setting/commands.md)を参照してください。

出力はこうなります。

```plaintext
segdata              art::TSegmentedData
eventheader          art::TEventHeader
```

`eventheader`はデフォルトで出力されるようになっており、`OutputTransparency`を 0 にするによって出力されるようになったのは`segdata`です。
実はこの中に必要なデータの中身が入っています。
詳細は、のちの章で紹介します。
