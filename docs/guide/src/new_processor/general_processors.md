# General Processors

`artemis`でデータを処理するためには、`art::TProcessor`を継承したクラスを定義し、その中に具体的な処理を書いていく必要があります。
また、ROOT で取り扱えるようにするために、dictionary に登録するなど、artemis 特有の処理だけではなく、ROOT の処理も加える必要があり、純粋な C++だけではない処理を加える必要があります。
この節では全てのプロセッサを作る際に、共通の設定や実装すべきメソッドなどを説明します。

## ヘッダーファイル

ここでは、`art::crib::THogeProcessor`という名前で、単純なプロセッサを作る場合を考えます。
`T`という接頭詞は、ROOT の慣習です。

1. **インクルードガード**

   ヘッダファイルには特有のトークンを使ったインクルードガードを設定してください。
   `#pragma once`を使う方法もありますが、トークンを使った方法が推奨されています。
   (現在は`#pragma once`でも問題ないかも？)

   ```cpp
   #ifndef TOKEN
   #define TOKEN

   #endif
   ```

2. **`TProcessor.h`のインクルード**

   継承元の TProcessor をインクルードします。継承を使う場合は前方宣言ではなく、インクルードを使用する必要があります。

   ```cpp
   #include <TProcessor.h>
   ```

3. **クラスの前方宣言**

   他のファイルで定義されているクラスを、現在作ろうとしているクラスの中で使用する場合、前方宣言を使います。
   インクルードではなく、前方宣言を使うと効率的な依存関係を定義することができます。
   この場合、使用するクラスはポインタを使用します。

   よく使われる例として、`TClonesArray`と`art::TSegmentedData`、`art::TCategorizedData`を前方宣言してみます。

   ```cpp
   class TClonesArray;

   namespace art {
   class TSegmentedData;
   class TCategorizedData;
   } // namespace art
   ```

4. **クラスの定義**

   CRIB で開発するクラスには`art::crib`をつけてください。
   また、artemis で動作させるには`art::TProcessor`を継承する必要があります。
   名前空間を直接クラスの宣言として`class art::crib::THogeProcessor`として宣言することも可能ですが、C++の一般的な慣例として、`namespace`ブロックを使用することをお勧めします。

   ```cpp
   namespace art::crib {
   class THogeProcessor : public TProcessor {
   };
   } // namespace art::crib
   ```

   > `art::crib`のような書き方は C++17 で標準化されました。

5. **クラスメソッドの実装**

   通常の C++のクラスとしてコンストラクタとデストラクタ、artemis のプロセッサとして、`art::TProcessor`からオーバーライドできるメソッドの代表的なものとして`Init`と`Process`などを実装します。
   ヘッダファイルでは定義のみ行い、具体的な実装はソースファイルで行います。
   また、これらはパブリックメソッドとしてください。

   ```cpp
   class THogeProcessor : public TProcessor {
     public:
       THogeProcessor();
       ~THogeProcessor();

       void Init(TEventCollection *col) override;
       void Process() override;
       void EndOfRun() override;
   };
   ```

   デストラクタは、継承元である`art::TProcessor`で仮想関数となっています。
   また、オーバーライドするメソッドには解析のどのタイミングで呼ばれるかによって、他にも複数のメソッドが用意されています。
   `art::TProcessor`の具体的な実装を参照してください。
   オーバーライドする際は、可読性をよくするため、そのメソッドがオーバーライドしていることを明示的に示すために、`override`修飾子をつけるようにしてください。

6. **メンバ変数の定義**

   その他のメソッドやメンバ変数は、他のファイルから参照することを想定していないので、private もしくは protected 指定子をつけます。
   このクラスから別のクラスを派生させない前提であれば、private 指定子のみを使用しても問題ありません。

   例えば、このような定義方法となります。

   ```cpp
   class THogeProcessor : public TProcessor {
     private:
       TClonesArray **fInData;
       TSegmentedData **fSegmentedData;
       TCategorizedData **fCategorizedData;

       Int_t fCounter; // any variable

       THogeProcessor(const THogeProcessor &) = delete;
       THogeProcessor &operator=(const THogeProcessor &) = delete;
   };
   ```

   まず、前方宣言したクラスに関しては、ポインタで指定していることに注意してください。
   また、最後の二つの行は、コピーコンストラクタとコピー代入演算子に関する記述です。
   C++では、クラスを宣言したときに暗黙的に定義されてしまいますが、artemis のプロセッサーでは必要ないので、`delete`を用いてそれを禁止するようにしています。

7. **ROOT 独自の記法**

   artemis は純粋な C++ではなく、ROOT の作法に則る必要があります。
   つまり、このクラスを ROOT が処理できるように少し修正を加える必要があります。

   ```cpp
   class THogeProcessor : public TProcessor {
     private:
       TClonesArray **fInData; //!
       TSegmentedData **fSegmentedData; //!
       TCategorizedData **fCategorizedData; //!

       Int_t fCounter; // any variable

       THogeProcessor(const THogeProcessor &) = delete;
       THogeProcessor &operator=(const THogeProcessor &) = delete;

       ClassDefOverride(THogeProcessor, 0);
   };
   ```

   ROOT にはストリーマ機能(`TStreamer`)と呼ばれるものがあります。
   これは`TTree`や`TFile`に保存する際に自動的に使われる機能です。
   このときに、ポインタのポインタ(`**fVariable`)が構造を持っていた場合、ROOT ではうまく扱うことができません。
   従って、これはストリーム対象外である、ということを明示するために`//!`というキーワードをつけます。

   また、定義したクラスを ROOT の枠組みで利用可能にするためには`ClassDef`マクロを使用する必要があります。
   引数の数字はバージョンを表しており、クラスの変更があった場合に数字をインクリメントすることで、互換性を保つことができるようになります。
   `ClassDef`の末尾に`;`をつけなくて動作しますが、統一性や formatter などの関係上`;`をつけることをお勧めします。
   クラス内で、`override`キーワードを用いる場合は、`ClassDef`ではなく、`ClassDefOverride`マクロを使用してください。

**まとめ**

最終的なヘッダファイルの全体像はこちらになります。
細かな部分に違いはありますが、すべてのプロセッサのおおまかな構成はすべてこのようになっています。

```cpp
#ifndef _CRIB_THOGEPROCESSOR_H_
#define _CRIB_THOGEPROCESSOR_H_

#include "TProcessor.h"

class TClonesArray;

namespace art {
class TSegmentedData;
class TCategorizedData;
} // namespace art

namespace art::crib {
class THogeProcessor : public TProcessor {
  public:
    THogeProcessor();
    ~THogeProcessor();

    void Init(TEventCollection *col) override;
    void Process() override;
    void EndOfRun() override;

  private:
    TClonesArray **fInData; //!
    TSegmentedData **fSegmentedData; //!
    TCategorizedData **fCategorizedData; //!

    Int_t fCounter; // any variable

    THogeProcessor(const THogeProcessor &) = delete;
    THogeProcessor &operator=(const THogeProcessor &) = delete;

    ClassDefOverride(THogeProcessor, 0);
};
} // namespace art::crib

#endif // end of #ifndef _CRIB_THOGEPROCESSOR_H_
```

`fInData`、`fSegmentedData`、`fCategorizedData`は定義方法を述べただけで、この後の処理では用いません。

## ソースファイル

```cpp
#include "THogeProcessor.h"

namespace art::crib {

// コンストラクタ実装
THogeProcessor::THogeProcessor()
    : fCounter(0)
{
    // コンストラクタ本体
}

// デストラクタ実装
THogeProcessor::~THogeProcessor()
{
    // 必要なクリーンアップ処理
}

// Initメソッド実装
void THogeProcessor::Init(TEventCollection *col) {
    // Init処理
}

// EndOfRunメソッド実装
void THogeProcessor::EndOfRun() {
    // 終了処理
}

// Processメソッド実装
void THogeProcessor::Process() {
    // 処理本体
}

} // namespace art::crib
```

## Dictionary への登録

## ビルド設定

## その他コメント

既存のプロセッサは、[okawak](https://github.com/okawak/)がまだ C++に対して未熟なときに作成したコードも含まれており、適切ではない表現が多々あると思います。
ぜひ積極的に修正を行なってください。

新しいプロセッサを作る場合、自由に作っていただいて構いませんが、既存のものを編集する、また CRIB でこれからも使えるような有用なものを作成していただいた場合、使い方をこのマニュアルに記載していただくか、コード内に Doxygen コメントとして残していただけると幸いです。

作成者が常に CRIB メンバーとして残るわけではないので、(artemis を将来的に使い続けるかは分かりませんが、)将来の新しいメンバーのために、分かりやすいツールとなるようにご協力お願いいたします。
