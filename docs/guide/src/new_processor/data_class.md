# Data Classes

今までは、`fValue`の一要素だけを持つ`art::TSimpleData`を`TClonesArray`の要素として使っていましたが、より複雑な構造を持つオブジェクトを扱いたい場合、新しいデータクラスを作成します。

このページでは、`art::crib::TMUXData`のクラスを実際に見ながら、データクラスの構造について理解していきます。

- [TMUXData.h](https://github.com/CRIB-project/artemis_crib/blob/main/src-crib/mux/TMUXData.h)
- [TMUXData.cc](https://github.com/CRIB-project/artemis_crib/blob/main/src-crib/mux/TMUXData.cc)

最終的に、前節のように`catdata`から、MUX のデータとして、この`TMUXData`型にデータをつめる Mapping Processor を使って、実際に`TMUXData`を使用する例を見ていきます。

- [TMUXDataMappingProcessor.h](https://github.com/CRIB-project/artemis_crib/blob/main/src-crib/mux/TMUXDataMappingProcessor.h)
- [TMUXDataMappingProcessor.cc](https://github.com/CRIB-project/artemis_crib/blob/main/src-crib/mux/TMUXDataMappingProcessor.cc)

## `TMUXData`の設計

### Step 1: インクルードガード

Processor の作成の際と同様に、ヘッダーファイルに固有のインクルードガードを設定してください。

```cpp
#ifndef _CRIB_TMUXDATA_H
#define _CRIB_TMUXDATA_H

#endif // _CRIB_TMUXDATA_H
```

### Step 2: 名前空間のブロック

`art::crib`の名前空間でブロックを作成し、その中にクラスの実装を書きます。

```cpp
namespace art::crib {
} // namespace art::crib
```

### Step 3: クラスの定義

artemis で使用するデータクラスは、全て`art::TDataObject`を継承する必要があります。
このヘッダファイルをインクルードし、クラスの枠組みを記述します。

```cpp
#include <TDataObject.h>

namespace art::crib {
class TMUXData : public TDataObject {
  public:
    TMUXData();
    ~TMUXData();
    TMUXData(const TMUXData &rhs);
    TMUXData &operator=(const TMUXData &rhs);

    void Copy(TObject &dest) const override;
    void Clear(Option_t *opt = "") override;
  private:
    ClassDefOverride(TMUXData, 1);
};
} // namespace art::crib
```

ここでは、コンストラクタとデストラクタ、コピーコンストラクタ、代入演算子、`Copy`、`Clear`メソッドを宣言しています。
後に実装を書きますが、これらは全てのデータクラスで実装する必要があることに注意してください。

また、ROOT で扱えるようにするために`ClassDef`のマクロを使用しています。

### Step 4: データ構造の記述

MUX は、`[E1, E2, P1, P2, T]`の 5 つのデータの組を出力します。
これらのデータを一つのオブジェクトで管理できるようなデータ構造を作成します。

```cpp
class TMUXData : public TDataObject {
  public:
    // Getter and Setter
    Double_t GetE1() const { return fE1; }
    void SetE1(Double_t value) { fE1 = value; }
    Double_t GetE2() const { return fE2; }
    void SetE2(Double_t value) { fE2 = value; }
    Double_t GetP1() const { return fP1; }
    void SetP1(Double_t value) { fP1 = value; }
    Double_t GetP2() const { return fP2; }
    void SetP2(Double_t value) { fP2 = value; }
    Double_t GetTrig() const { return fTiming; }
    void SetTrig(Double_t value) { fTiming = value; }

    static const Int_t kNRAW = 5;

  private:
    Double_t fE1;
    Double_t fE2;
    Double_t fP1;
    Double_t fP2;
    Double_t fTiming;
};
```

プライベートメンバ変数として、値にアクセスするときは、Getter と Setter を使うようにします。

### Step 5: メソッドの実装

ソースファイルに必要なメソッドの実装を行います。

## `TMUXDataMappingProcessor`の設計
