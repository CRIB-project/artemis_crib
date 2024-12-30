# Data Classes

前節で、`TChannelSelector`を作成した際に、出力するブランチを`TCloneArray`の要素に`art::TSimpleData`を格納しました。
このようなデータ構造を新しく作成してみましょう。

`art::TSimpleData`は`fValue`の一要素だけを持つクラスでしたが、今回は`std::vector<T>`の要素を一つ持つクラスを作成してみましょう。

さまざまなデータを持つ複雑なクラスを作成する際も、ここで紹介する手順に沿って作成することができます。

## 設計

以下のようなデータクラスを作成します。

- 名前: `TSimpleVectorData`
- 名前空間: `art::crib` (CRIB で開発したクラスにつける)
- メンバクラス: `std::vector<T>`の型のメンバ変数を持つ
- テンプレート: `<T>`として`Double_t`、`Int_t`、`UInt_t`を実装

> **Note**: ROOT で使用するクラスは事前に、利用するテンプレート型を辞書に登録する必要があります。
> したがって、ここではテンプレート型を定義して、上に述べた型に対して定義する形をとります。

## ヘッダファイル

### Step 1: インクルードガード

Processor の作成と同様に、固有のインクルードガードを設定してください。

```cpp
#ifndef _CRIB_TSIMPLEVECTORDATA_H
#define _CRIB_TSIMPLEVECTORDATA_H

#endif // _CRIB_TSIMPLEVECTORDATA_H
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
#include <vector>

namespace art::crib {
template <typename T>
class TSimpleVectorData : public TDataObject {
  public:
    TSimpleVectorData();
    ~TSimpleVectorData();

  private:
    std::vector<T> fVec;
}
} // namespace art::crib
```

ここでは、コンストラクタとデストラクタ、また、`std::vector<T>`型のメンバ変数として、`fVec`を定義しています。

### Step 4: 必要なメソッドの定義

- コピーコンストラクタ、代入演算子
- Copy メソッド(override)
- Clear メソッド(override)

これらのメソッドは、他のプロセッサから呼ばれる可能性があり、適切に実装しなければ、思った動作をしない可能性があります。

これらを定義しておきます。

```cpp
class TSimpleVectorData : public TDataObject {
  public:
    TSimpleVectorData(const TSimpleVectorData &rhs);
    TSimpleVectorData &operator=(const TSimpleVectorData &rhs);

    void Copy(TObject &dest) const override;
    void Clear(Option_t *opt = "") override;
}
```

### Step 5: ROOT のマクロ

辞書に登録するためのマクロを設定します。

```cpp
class TSimpleVectorData : public TDataObject {
  private:
    ClassDefOverride(TSimpleVectorData, 0);
}
```
