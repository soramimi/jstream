[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/soramimi/jstream)

# Jstream

C++17 向けの軽量で柔軟なヘッダーオンリー JSON パーサー・ジェネレーターライブラリです。

## 概要

jstream ライブラリは、イベントベースのストリーミング JSON パーサーと、JSON データを生成するためのシンプルなインターフェイスを提供します。C++ アプリケーションで JSON を扱うための、柔軟で使いやすいライブラリを目指しています。

### 主な特徴

- **イベントベースのパース**: JSON データをイベントのストリームとしてパース
- **パスベースのアクセス**: パス式を使って JSON 要素にアクセス
- **柔軟な設定**: コメント、クォートなしキー、末尾カンマ、16進数、特殊定数をサポート
- **ロケール非依存**: 数値パースがロケール設定に影響されない
- **Unicode 対応**: 文字列内の Unicode 文字やサロゲートペアを扱える
- **Variant ベースのデータモデル**: `std::variant` を使った型安全な JSON 表現
- **ヘッダーオンリー**: `include/jstream.h` をインクルードするだけ

## インストール

ヘッダーファイルをプロジェクトに含めるだけです:

```cpp
#include "jstream.h"
```

## 使い方

### JSON のパース

```cpp
#include "jstream.h"
#include <iostream>

int main() {
    const char* json = R"({
        "name": "John Doe",
        "age": 30,
        "cities": ["New York", "London", "Tokyo"]
    })";

    // JSON をパース
    jstream::Reader reader(json);

    // 必要に応じてオプション機能を有効化
    reader.allow_comment(true);
    reader.allow_unquoted_key(true);

    // JSON イベントを順に処理
    while (reader.next()) {
        if (reader.match("{name") && reader.isstring()) {
            std::cout << "Name: " << reader.string() << std::endl;
        } else if (reader.match("{age") && reader.isnumber()) {
            std::cout << "Age: " << reader.number() << std::endl;
        } else if (reader.match("{cities[*") && reader.isstring()) {
            std::cout << "City: " << reader.string() << std::endl;
        }
    }

    return 0;
}
```

### ストリーミング入力

`Reader` は逐次到着する JSON をパースできます。入力コールバックを使って Reader を作成し、`input()` でチャンクを追加し、トークンの途中でパースが停止した場合は `is_not_enough_input()` を確認します。

```cpp
#include "jstream.h"
#include <iostream>

int main() {
    std::string chunk1 = R"({"name": "Jo")";
    std::string chunk2 = R"(hn", "age": 30})";

    jstream::Reader reader;
    reader.input(chunk1);
    reader.input(chunk2);

    while (reader.next()) {
        if (reader.match("{name") && reader.isstring()) {
            std::cout << "Name: " << reader.string() << std::endl;
        } else if (reader.match("{age") && reader.isnumber()) {
            std::cout << "Age: " << reader.number() << std::endl;
        }
    }
}
```

コールバック駆動のストリーミングでは、`parse()` を使ってコールバックを設定します:

```cpp
jstream::Reader reader;
reader.parse([&](){ reader.input(get_next_chunk()); });
while (reader.next()) { /* ... */ }
```

ストリームに複数のトップレベル JSON ドキュメントが含まれている場合、最初のドキュメントをパースし、`next_document()` を呼んで `EndDocument` 状態をクリアしてから、パースを続行します。

### JSON の生成

```cpp
#include "jstream.h"
#include <iostream>

int main() {
    // 標準出力に出力する JSON ライターを作成
    jstream::Writer writer([](const char* p, int n) {
        std::cout.write(p, n);
    });

    // 必要に応じて整形を設定
    writer.enable_indent(true);
    writer.enable_newline(true);

    // JSON を生成
    writer.object({}, [&]() {
        writer.string("name", "John Doe");
        writer.number("age", 30);
        writer.array("cities", [&]() {
            writer.string("New York");
            writer.string("London");
            writer.string("Tokyo");
        });
        writer.boolean("active", true);
        writer.null("optionalField");
    });

    return 0;
}
```

出力を `std::string` で取得することもできます:

```cpp
jstream::Writer writer;           // コールバックなし
writer.string("hello", "world");
std::string json = writer;        // 暗黙的な std::string 変換
```

生の JSON テキストを挿入することもできます:

```cpp
writer.raw("metadata", "{\"source\":\"api\"}");
```

### Variant ベースの API の利用

```cpp
#include "jstream.h"
#include <iostream>

int main() {
    // JSON オブジェクトを作成
    jstream::Variant root;
    auto obj = jstream::obj(root);

    // プロパティを追加
    obj["name"] = "John Doe";
    obj["age"] = 30.0;

    // 配列を追加
    auto& cities = jstream::arr(obj["cities"]);
    cities.push_back("New York");
    cities.push_back("London");
    cities.push_back("Tokyo");

    // 値にアクセス
    if (jstream::is_string(obj.value("name"))) {
        std::cout << "Name: " << jstream::get<std::string>(obj.value("name")) << std::endl;
    }

    return 0;
}
```

### エラー処理

パーサーは、メッセージ、バイトオフセット、行、列を含む詳細なエラー情報を記録します:

```cpp
jstream::Reader reader(json);
while (reader.next()) { }

if (reader.has_error()) {
    for (auto const& e : reader.errors()) {
        std::cout << e.what() << " at line " << e.line
                  << ", column " << e.column << std::endl;
    }
}
```

## 設定オプション

`Reader` クラスは以下の設定オプションをサポートしています:

- `allow_comment(bool)`: JSON 内で C 言語・C++ スタイルのコメントを許可
- `allow_ambiguous_comma(bool)`: 配列やオブジェクト内の末尾カンマを許可
- `allow_unquoted_key(bool)`: クォートなしのオブジェクトキーを許可
- `allow_hexadecimal(bool)`: 16進数形式 (`0xNNN` および `-0xNNN`) を許可
- `allow_special_constant(bool)`: `Infinity` や `NaN` などの特殊定数を許可
- `allow_key_in_array(bool)`: 配列内で `"key":"value"` 構文を許可

## パスマッチング

パス式を使うと、JSON 内の位置を簡潔にマッチングできます:

- `{key}` - オブジェクトのキーにマッチ
- `[*]` - 任意の配列要素にマッチ（定数値）
- `{*}` - 任意のオブジェクトキーにマッチ（定数値）
- `*[` - 任意の配列開始にマッチ
- `*{` - 任意のオブジェクト開始にマッチ
- `**` - 任意のネストされたパスにマッチ（末尾に置く必要がある）
- `@...` - 現在の `nest()` ベースラインからの相対パス

例:

```cpp
reader.match("{user{name");           // user.name
reader.match("{items[*{price");       // items[].price
reader.match("{items[*{price");       // items 内の任意のオブジェクトの price
reader.match_start_object("{items[*{"); // items 配列内の任意のオブジェクト

reader.nest([&](){                    // ネストしたスコープに入る
    if (reader.match("@{value")) {    // nest のベースラインから相対的に "value" をマッチ
        // ...
    }
});
```

## 状態確認

`Reader` で使える主な述語とアクセサ:

- `state()` - 現在の状態 (`StateType`)
- `is_constant()`, `is_structure()`, `is_value()` - 現在の状態を分類
- `is_start_object()`, `is_end_object()`, `is_start_array()`, `is_end_array()`
- `isnull()`, `isboolean()`, `isnumber()`, `isstring()`
- `is_end_document()`
- `key()`, `string()`, `number()`, `boolean()`
- `path()`, `depth()`, `tell()`
- `is_not_enough_input()` - ストリーミングモードでバッファがトークンの途中で終わったためパースが一時停止した場合に true
- `extract()` - 最後にパースされた要素の生テキスト

## ビルドとテスト

qmake プロジェクトファイルが同梱されています:

```bash
qmake jstream.pro
make
```

単体テストは Google Test を使用します:

```bash
cd test-cpp
make
./myapp
```

## 移植版

- **C#**: `jstream-cs/` ディレクトリ。`README_CSharp.md` を参照。
- **Go**: `jstream-go/` ディレクトリ。`README_Go.md` を参照。
- **English**: `README.md`

## ライセンス

このソフトウェアは MIT ライセンスのもとで配布されています。
