// i18n.cpp — English/Japanese string table for the menu.
#include "i18n.h"

#include <cstring>
#include <string>

namespace i18n {
namespace {

struct Pair {
    const char* en;
    const char* ja;
};

// Linear scan on purpose: the menu asks for a few dozen short strings
// per frame, so the table stays flat and readable.
constexpr Pair kTable[] = {
    {"Game not found", "ゲームが見つかりません"},
    {"No characters: join a match", "キャラクターなし：マッチに参加してください"},
    {"In match", "マッチ中"},
    {"Passive reading. Nothing is written to the game.",
     "読み取り専用。ゲームには書き込みません。"},
    {"Open in browser", "ブラウザで開く"},
    {"Targets", "ターゲット"},
    {"Visual", "表示"},
    {"Settings", "設定"},
    {"Enable ESP", "ESPを有効化"},
    {"Target: %s", "ターゲット: %s"},
    {"Counselors", "カウンセラー"},
    {"BOX", "BOX"},
    {"Draw", "描画"},
    {"Black outline", "黒い縁"},
    {"Background", "背景"},
    {"Style", "スタイル"},
    {"Solid", "通常"},
    {"Corners", "コーナー"},
    {"Rounded", "角丸"},
    {"SKELETON", "スケルトン"},
    {"Draw skeleton", "スケルトンを描画"},
    {"Thickness", "太さ"},
    {"TEXT", "TEXT"},
    {"Player name", "名前"},
    {"Health", "体力"},
    {"Stamina", "スタミナ"},
    {"Distance", "距離"},
    {"Size", "サイズ"},
    {"Small", "小"},
    {"Medium", "中"},
    {"Large", "大"},
    {"MISC", "その他"},
    {"Snapline", "スナップライン"},
    {"Head dot", "頭マーカー"},
    {"Center crosshair", "中央クロスヘア"},
    {"Visible only", "可視のみ"},
    {"%d targets  |  %d visible", "%d ターゲット  |  %d 可視"},
    {"Name", "名前"},
    {"Visible", "可視"},
    {"yes", "はい"},
    {"no", "いいえ"},
    {"Max distance", "最大距離"},
    {"Min height", "最低高さ"},
    {"Box width", "ボックス幅"},
    {"Text scale", "文字スケール"},
    {"Crosshair size", "クロスヘアサイズ"},
    {"COLORS", "カラー"},
    {"Open color picker", "カラーピッカーを開く"},
    {"Box", "ボックス"},
    {"Outline", "縁"},
    {"Fill", "塗り"},
    {"Skeleton", "スケルトン"},
    {"Line", "ライン"},
    {"Head", "頭"},
    {"Crosshair", "クロスヘア"},
    {"OVERLAY", "オーバーレイ"},
    {"Keep above the game", "ゲーム手前に表示"},
    {"HOTKEYS", "ホットキー"},
    {"INSERT  toggles the menu", "INSERT  メニュー開閉"},
    {"END     toggles the ESP", "END     ESP切替"},
    {"DELETE  quits", "DELETE  終了"},
    {"ABOUT", "情報"},
    {"Passive read: the game is never modified.",
     "読み取り専用：ゲームは一切変更されません。"},
    {"Targets are detected automatically by your character.",
     "ターゲットは自動で検出されます。"},
    {"LANGUAGE", "言語"},
    {"Language", "言語"},
};

// Scratch buffer for keys that carry an ImGui id after "##".
std::string& scratch() {
    static std::string s;
    return s;
}

}  // namespace

const char* tr(const char* key) {
    if (!key || g_lang != kJa) return key;

    // Visible label only: everything after "##" is an ImGui id.
    const char* hash = std::strstr(key, "##");
    const size_t n = hash ? static_cast<size_t>(hash - key) : std::strlen(key);

    for (const Pair& p : kTable) {
        if (p.en[n] != '\0') continue;  // not the same visible length
        if (std::strncmp(p.en, key, n) == 0) {
            if (!hash) return p.ja;
            scratch().assign(p.ja).append(hash);
            return scratch().c_str();
        }
    }
    return key;
}

}  // namespace i18n
