# SummerCamp ESP

![license](https://img.shields.io/badge/license-MIT-blue)
![language](https://img.shields.io/badge/language-C%2B%2B-00599C)
![platform](https://img.shields.io/badge/platform-Windows-0078D6)
![release](https://img.shields.io/github/v/release/kaikochumi/SummerCamp-ESP)
![build](https://github.com/kaikochumi/SummerCamp-ESP/actions/workflows/build.yml/badge.svg)

**Friday the 13th: The Game** の元 *Summer Camp* ビルド（Camp Forest Green、
Unreal Engine 4.18.3）用の外部 ESP オーバーレイです。半透明の最前面ウィンドウに
プレイヤーのボックス、スケルトン、体力バーを描画します。ゲームプロセスは読み取り
専用で開き、書き込みも DLL 注入もカーネルドライバーも使いません。

メニューは英語と日本語に対応しています（`設定` タブで切り替え、または
設定ファイルの `lang` キー）。

![ゲーム内の様子](docs/screenshots/in-game.png)
*Jason側から：4人のカウンセラーをコーナーボックス・スケルトン・スナップライン・
距離で追跡。右にメニューを開いたまま。*

> [!IMPORTANT]
> **動きましたか？⭐をお願いします。** 個人の趣味プロジェクトで、スターが唯一の
> 「使っている人がいる」サインです。ゲーム更新後のオフセット対応のモチベーション
> になります。

## スクリーンショット

| 英語 | 日本語 |
|------|--------|
| ![英語メニュー](docs/screenshots/menu-en.png) | ![日本語メニュー](docs/screenshots/menu-ja.png) |

## 機能

- プレイヤー ESP：ボックス（通常 / コーナー / 角丸）、スケルトン、名前、
  距離、体力・スタミナバー
- スナップライン、ヘッドマーカー、中央クロスヘア（それぞれオン/オフ可能）
- ターゲットは自動判別：カウンセラー側なら Jason を、Jason 側なら
  カウンセラーを追跡します
- ターゲットタブに一覧を表示（名前・距離・体力・視認状況）
- 設定はすべて設定ファイルに保存され、再起動後も引き継がれます
- menu 以外はクリックスルー：ゲーム側の入力は妨げません

## 必要環境

Windows 10 以降、CMake 3.20 以上。コンパイラは次のどちらか：

- MinGW-w64（GCC 16 で動作確認）
- MSVC（Build Tools 2022 でもそのままビルド可）

Dear ImGui は `third_party/imgui` に同梱しているので、ビルド時に
ネットワークアクセスは不要です。バックエンドは win32 と dx11 のみ同梱。

## ビルド

MinGW の場合：

    cmake -B build -G "MinGW Makefiles"
    cmake --build build -j

MSVC の場合は `-G` 行を外して生成されたソリューションをビルドします。
実行ファイルは `build/summercamp-esp.exe` に出力されます。

## 使い方

[最新リリース](https://github.com/kaikochumi/SummerCamp-ESP/releases/latest)から
`SummerCamp-ESP-v0.1.0-Win64.zip` をダウンロードして展開し、先にゲームを起動して
からツールを起動します：

    summercamp-esp.exe --module summercamp --cfg esp.cfg

引数はどちらも省略可能です。`--module` はゲームプロセスを探すモジュール名、
`--cfg` は設定ファイルのパスです。設定ファイルは初回起動時にデフォルト値で
作成され、メニューを操作するたびに保存されます。

| キー    | 動作         |
|---------|--------------|
| INSERT  | メニュー開閉 |
| END     | ESP 切り替え |
| DELETE  | 終了         |

標準的な見た目に調整したプリセット（細いオレンジのコーナーボックス、
スケルトン、メートル表示の距離）を `config/preset.cfg` に同梱しています：

    summercamp-esp.exe --cfg config/preset.cfg

## 設定

設定ファイルは 1 行に 1 つの `key=value` 形式で、項目名はメニューと一致
しています。色は IM_COL32 の並び（`0xAABBGGRR`）です。`lang=0` で英語、
`lang=1` で日本語になります。日本語フォントはシステムから読み込みます
（Meiryo → Yu Gothic → MS Gothic の順）。

## オフセット

メモリレイアウトは `src/offsets.h` にあり、
[docs/OFFSETS.md](docs/OFFSETS.md) に説明をまとめてあります。数値は
エンジンとゲームの構造から割り出し、ゲームを動かしながら確認したもの
です（UE 4.18.3、ビルド 38951）。ゲームが更新されるとアドレスは
変わります。

**ゲームのバージョン：** コミュニティの *Summer Camp* ビルド
`4.18.3-38951`（Camp Forest Green ランチャー）で、**2026年10月**に
動作確認済みです。あなたのゲームが別のビルドになった場合は、先に
確認し直しが必要です。

## ロードマップ

- オブジェクト ESP：カーバッテリー、ガソリン、ヒューズ、医薬品、ナイフ
- ポール別のボックスカラー（カウンセラー vs Jason）
- ゲーム更新時のメニュー内バージョン通知
- メニューの追加翻訳

## 免責事項

このプロセス外からゲームメモリを読み取るプロジェクトで、Unreal Engine の
メモリレイアウトの学習を目的としています。使用は自己責任で行ってください。
アカウントへのペナルティなど、利用によって生じた結果について作者は
責任を負いません。

## サポート

うまく動いたらスターを押してください（2クリックです）。スターがこのリポジトリを
継続させ、ゲームが更新されるたびに [docs/OFFSETS.md](docs/OFFSETS.md) を
早く更新します。

⭐ [**kaikochumi/SummerCamp-ESP にスターを付ける**](https://github.com/kaikochumi/SummerCamp-ESP/stargazers)

