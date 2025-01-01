# For CRIB members

## memo

### Processor の仮想関数について

TProcessor を継承して新しいプロセッサを作成するときに、TProcessor を継承し、
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

使うときは、virtual 修飾子はつけなくてもよく(つけても良い)、override キーワードをつけておくとオーバーライドした関数であることがわかって良いかも。
この時は、ClassDefOverride を使う。
どのタイミングでこの関数が呼ばれるかという違いがあるが、Init と Process さえあれば行いたい処理は十分可能だと思う。

関数が呼ばれるタイミングは以下の通り。

- <add> -> Init
- -> <res> -> BeginOfRun -> PreLoop -> (PreProcess -> Process -> PostProcess) -> (PreProcess -> Process -> PostProcess) -> ...
- -> PostProcess -> PostLoop -> EndOfRun
- -> <.q> -> Terminate

途中で sus を挟んだ場合は、

- PostProcess) -> <sus> -> PostLoop -> <res> -> PreLoop -> (PreProcess -> Process -> PreProcess) -> ...

といったような順番で呼ばれる。

## How to merge from each experiment to main branch
