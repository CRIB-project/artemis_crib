# Home

This is a detailed instruction about source code for developers.

## Doxygen LISENSE

This pages are created by [doxygen](https://www.doxygen.nl/).

Copyright © 1997-2009 by Dimitri van Heesch.

Permission to use, copy, modify, and distribute this software and its documentation under the terms of the GNU General Public License is hereby granted. No representations are made about the suitability of this software for any purpose. It is provided "as is" without express or implied warranty. See the GNU General Public License for more details.

Documents produced by doxygen are derivative works derived from the input used in their production; they are not affected by this license.

## 名前空間

CRIB独自で開発したクラス、コードに対しては、cribという名前空間でラップして、artemis本体にあるコードと分けて管理できるようにしようと考えています。
例えば、`art::TTimingChargeMappingProcessor`はartemis自体にあるプロセッサから使用しており、`art::crib::TMUXMappingProcesssor`はartemis_cribで独自に
作成したプロセッサを使用している、といった形です。

もし、artemis自身にある同名、同じ役割のプロセッサを編集して使いたいと思った場合は、cribという名前空間を使用して追加してください。
また、今後もCRIB実験で使える汎用的なものを開発した場合もcribという名前空間を用いてプルリクエスト等を送ってください。

## version管理

基本的には後方互換性はありません。
厳密にどういった変更をしたかなどのノートは残さないので、解析をするときは、その環境をそのまま使う方が良いと思います。
artemis自体、artemis-workdir (artemis_crib)のバージョン、使っているenergy lossのツールのバージョン、
pythonのバージョン(pyROOTや便利スクリプトに使用、artemisの動作自体には使いません。)などを記録し、それをベースに解析を始めることをお勧めします。
同じ動作をすることが確認できてから、versionアップデートすると良いと思います。

pythonとartemis_cribのバージョン管理をしやすくするためのversion管理を2024年8月のバージョンから導入することにしました。
versionの名前は、年.月.0 (major.minor.patch)としています。(CMakeLists.txtとpyproject.toml)
ある程度動作確認が済んだら、gitのtagで分けていこうと思います。

以下動作未確認です。
gitのmainブランチを使用している場合は、`install/lib`の中にあるライブラリのシンボリックリンクを変えれば古いバージョンのものが使えるはず？

pythonのバージョンは`poetry`を用いて管理することにしました。
公式サイトからインストールするか、`pip install poetry`でインストールできます。
仮想環境(.venv)をそのディレクトリ内に作りたいときは、グローバルな設定で、

```
poetry config virtualenvs.in-project true
```

とすれば良いです。(ローカルな設定にもできます。)
その後、art_workdirで、

```
poetry install
```

で仮想環境の準備ができます。が、仮想環境がなくてもartemisは動作します。
うまくいかない場合は、lockファイルを消してからもう一度試してみてください。(うまくいくかはわからずです。)
.venvをそのディレクトリ内に作成する場合は、artloginで自動的にその環境には入るように設定していますが、
そうでない場合、作成した仮想環境に入りたい場合は、

```
poetry shell
```

で有効化できます。

## 豆知識

日本語ですみません。ライブラリを整備している中で、気になったことをメモしておきます。
時間がある時に英語で整備し直すかもしれません。

### 1. //!の意味

プロセッサなどを作成していると、

```cpp
TClonesArray **fInData; //!
```

のようなコードに出くわすことがあるが、この最後の"//!"についてはよくわかっていなかった。
このおまじないのようなものを消すとエラーが出てくるし、調べてもDoxygenのコメント形式だというような情報しか得られずよくわからないままだった。

最近ChatGPTに聞いてみると、初めて知ったことだが、ROOTのCINTやClingインタープリタででポインタのポインタをI/Oシステムで使うことが難しく、
正しく解析できないというエラーで、//!をつけるとこのI/O処理から除外するように伝えることができるらしい。

難しくて完全には理解できていないが、ポインタのポインタをROOTで扱うのは難しいから、//!をつけるとROOTの範疇から外すという意味合いになるということなのかな???
ROOTの作法ではなく、純粋なC++な知識だけが必要なものがいいなあと感じたり。。。

### 2. ClassDefについて

ClassをDictionaryに登録するときに、ClassDefを使うが、クラスメソッドをoverrideするときにClassDefを使うと長い警告文が出てくる。

調べてみると、ClassDefにも種類があるようで、オーバーライドを使用する時には、ClassDefOverrideというものが定義されているらしい。
[情報](https://root-forum.cern.ch/t/classdef-variants/44736)の通り全てを列挙してみると、

- ClassDef: Regular
- ClassDefOverride: Use this one if your class definition use the c++ `override` keyword for virtual functions (If you use the regular ClassDef, the compiler might warn that the function declared in the ClassDef are missing the override keyword).
- ClassDefNV: Use if you class does not (and shoud not) have a virtual function table (i.e. no virtual function in the class and any of its base classes

ROOTには認識されるが、Dictionaryに登録しない場合は、

- ClassDefInline
- ClassDefInlineOverride
- ClassDefInlineNV

という書き方もできるらしいが、一応まだexperimentalらしい。

通常は、ClassDefかClassDefOverrideだけ使えば良さそうな感じかな?

### 3. Processorの仮想関数について

TProcessorを継承して新しいプロセッサを作成するときに、TProcessorを継承し、
関数をオーバーライドするときに汎用的に使えそうな(自分が使っている)クラスメソッドの一覧のメモ。

```cpp
   // InitProcメソッドで呼ばれる
   virtual void Init (TEventCollection *) {;}

   // user defined member functions to process data
   virtual void BeginOfRun() {;}
   virtual void EndOfRun() {;}
   virtual void PreProcess() {;}
   virtual void Process() {;}
   virtual void PostProcess() {;}
   virtual void PreLoop() {;}
   virtual void PostLoop() {;}
   virtual void Terminate() {;}
```

使うときは、virtual修飾子はつけなくてもよく(つけても良い)、overrideキーワードをつけておくとオーバーライドした関数であることがわかって良いかも。
この時は、ClassDefOverrideを使う。
どのタイミングでこの関数が呼ばれるかという違いがあるが、InitとProcessさえあれば行いたい処理は十分可能だと思う。

関数が呼ばれるタイミングは以下の通り。

- <add> -> Init
- -> <res> -> BeginOfRun -> PreLoop -> (PreProcess -> Process -> PostProcess) -> (PreProcess -> Process -> PostProcess) -> ...
- -> PostProcess -> PostLoop -> EndOfRun
- -> <.q> -> Terminate

途中でsusを挟んだ場合は、

- PostProcess) -> <sus> -> PostLoop -> <res> -> PreLoop -> (PreProcess -> Process -> PreProcess) -> ...

といったような順番で呼ばれる。

### 4. Data classの仮想関数について

新しいData classを作成する(ROOTのブランチに新しいクラスを入れたい)ときには、
TDataObjectを継承し、そのときに汎用的に使えそうな仮想関数は以下の通りです。
(他にも仮想関数はあります。)

```cpp
   virtual void Clear(Option_t *option = "");
   virtual void Copy(TObject& object) const;
   virtual void Print(Option_t* option = "") const;
   virtual void SetID(Int_t id) { fID = id; }
   virtual Int_t GetID() const { return fID; }
```

この中で、特にClearとCopyは、必ず実装しておくべきで、Clearは各processorのなかで、
最初にオブジェクトを初期化するときに使用し、Copyは、
art::crib::TBranchCopyProcessorでTTreeEventStoreからartemisを回すときに、
アウトプットされるROOTファイルにも同じブランチをコピーするときに使えます。

こちらも同様に、overrideキーワードをつけ、ClassDefOverrideで実装してください。
