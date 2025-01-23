# Beam Generator

このページではまず、シミュレーション上で、イベントを発生させるためのビーム生成を行う方法について説明します。

ビームを生成させる方法としては、

1. 乱数で色々な位置、角度からのビームを生成する
2. 実際に得たデータのビーム情報から生成する

以上の二つの方法があります。
それぞれの使い方を解説します。

## 乱数でビームを生成する

## データ情報を元にビーム生成する

ビームの位置、角度のトラッキング情報が ROOT ファイルに保存されており、それを元にしてシミュレーションをする場合について説明します。

### EventStore

ファイルが格納された ROOT ファイルをインプットとする`EventStore`を使います。
この時、シミュレーションで用いるときに便利なように、`TTree`を一周すると一番目のイベントに戻るようになっている`art::crib::TTreePeriodicEventStore`を用います。

これは、artemis が提供する`art::TTreeEventStore`を周回するように変更しただけです。
steering ファイルには以下のようにループ回数を指定します。

```yaml
Processor:
  - name: MyTTreePeriodicEventStore
    type: art::crib::TTreePeriodicEventStore
    parameter:
      FileName: temp.root # [TString] The name of input file
      MaxEventNum: 0 # [Long_t] The maximum event number to be analyzed.
      OutputTransparency: 0 # [Bool_t] Output is persistent if false (default)
      TreeName: tree # [TString] The name of input tree
      Verbose: 1 # [Int_t] verbose level (default 1 : non quiet)
```

### Beam Generator
