# General Processors

artemis で使用されるソースコードは、いくつかの種類に分類することができます。
ここでは[okawak](https://github.com/okawak/)が勝手に名前をつけています。

- Main: artemis の実行バイナリを作成するメインファイル
- Loop: イベントループを管理するファイル
- Hist: ヒストグラムを管理するファイル
- Command: artemis のコンソール上で使えるコマンドを定義するファイル
- Decoder: デコーダーとそのデコーダーを管理するファイル
- EventStore: ループで使用するイベントを管理するファイル
- Processor: 実際のデータの処理を行うファイル
- Data: データ構造を定義するファイル
- Parameter: パラメータを管理するオブジェクトを定義するファイル
- Others: その他のファイル、例えば artemis canvas など解析上で役に立つファイル群

この中で、新たな機能を追加するときには、`Processor`と`Data`を新たに定義することになると思います。
まず、一般的なプロセッサの構造について説明したいと思います。
