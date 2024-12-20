# Process Seg Data

`art::TRIDFEventStore`で出力される`segdata`([Read RIDF files の節](../preparation/ridf.md)を参照してください。)から値を読み取って、それを TTree に格納するというシンプルなプロセッサを作りながら、同時に`segdata`について詳細に理解します。

## 状況設定

新しく、`segdata`から特定のチャンネルのデータを抜き出すようなプロセッサを作ってみましょう。

- `TChannelSelector`という名前のプロセッサにする
- CRIB 独自で開発したコードなので、`art::crib`という名前空間を用いる
- `segdata`をインプットとする
