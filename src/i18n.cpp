// i18n.cpp — English/Japanese/Portuguese string table for the menu.
#include "i18n.h"

#include <cstring>
#include <string>

namespace i18n {
namespace {

struct Pair {
    const char* en;
    const char* ja;
    const char* pt;
};

// Linear scan on purpose: the menu asks for a few dozen short strings
// per frame, so the table stays flat and readable.
constexpr Pair kTable[] = {
    {"Game not found", "ゲームが見つかりません", "Jogo não encontrado"},
    {"No characters: join a match", "キャラクターなし：マッチに参加してください", "Nenhum personagem: entre numa partida"},
    {"In match", "マッチ中", "Em partida"},
    {"Passive reading. Nothing is written to the game.",
     "読み取り専用。ゲームには書き込みません。",
     "Leitura passiva. Nada é escrito no jogo."},
    {"Open in browser", "ブラウザで開く", "Abrir no navegador"},
    {"Targets", "ターゲット", "Alvos"},
    {"Visual", "表示", "Visual"},
    {"Settings", "設定", "Configurações"},
    {"Enable ESP", "ESPを有効化", "Ativar ESP"},
    {"Target: %s", "ターゲット: %s", "Alvo: %s"},
    {"Counselors", "カウンセラー", "Conselheiros"},
    {"BOX", "BOX", "CAIXA"},
    {"Draw", "描画", "Desenhar"},
    {"Black outline", "黒い縁", "Contorno preto"},
    {"Background", "背景", "Fundo"},
    {"Style", "スタイル", "Estilo"},
    {"Solid", "通常", "Sólida"},
    {"Corners", "コーナー", "Cantos"},
    {"Rounded", "角丸", "Arredondada"},
    {"SKELETON", "スケルトン", "ESQUELETO"},
    {"Draw skeleton", "スケルトンを描画", "Desenhar esqueleto"},
    {"Thickness", "太さ", "Espessura"},
    {"TEXT", "TEXT", "TEXTO"},
    {"Player name", "名前", "Nome do jogador"},
    {"Health", "体力", "Vida"},
    {"Stamina", "スタミナ", "Stamina"},
    {"Distance", "距離", "Distância"},
    {"Size", "サイズ", "Tamanho"},
    {"Small", "小", "Pequeno"},
    {"Medium", "中", "Médio"},
    {"Large", "大", "Grande"},
    {"MISC", "その他", "DIVERSOS"},
    {"Snapline", "スナップライン", "Linha de mira"},
    {"Head dot", "頭マーカー", "Marcador na cabeça"},
    {"Center crosshair", "中央クロスヘア", "Mira central"},
    {"Visible only", "可視のみ", "Apenas visíveis"},
    {"%d targets  |  %d visible", "%d ターゲット  |  %d 可視", "%d alvos  |  %d visíveis"},
    {"Name", "名前", "Nome"},
    {"Visible", "可視", "Visível"},
    {"yes", "はい", "sim"},
    {"no", "いいえ", "não"},
    {"Max distance", "最大距離", "Distância máx."},
    {"Min height", "最低高さ", "Altura mín."},
    {"Box width", "ボックス幅", "Largura da caixa"},
    {"Text scale", "文字スケール", "Escala do texto"},
    {"Crosshair size", "クロスヘアサイズ", "Tamanho da mira"},
    {"COLORS", "カラー", "CORES"},
    {"Open color picker", "カラーピッカーを開く", "Abrir seletor de cor"},
    {"Box", "ボックス", "Caixa"},
    {"Outline", "縁", "Contorno"},
    {"Fill", "塗り", "Preenchimento"},
    {"Skeleton", "スケルトン", "Esqueleto"},
    {"Line", "ライン", "Linha"},
    {"Head", "頭", "Cabeça"},
    {"Crosshair", "クロスヘア", "Mira"},
    {"OVERLAY", "オーバーレイ", "SOBREPOSIÇÃO"},
    {"Keep above the game", "ゲーム手前に表示", "Manter acima do jogo"},
    {"HOTKEYS", "ホットキー", "ATALHOS"},
    {"INSERT  toggles the menu", "INSERT  メニュー開閉", "INSERT  abre/fecha menu"},
    {"END     toggles the ESP", "END     ESP切替", "END     liga/desliga ESP"},
    {"DELETE  quits", "DELETE  終了", "DELETE  sai"},
    {"ABOUT", "情報", "SOBRE"},
    {"Passive read: the game is never modified.",
     "読み取り専用：ゲームは一切変更されません。",
     "Leitura passiva: o jogo nunca é modificado."},
    {"Targets are detected automatically by your character.",
     "ターゲットは自動で検出されます。",
     "Alvos são detectados automaticamente pelo seu personagem."},
    {"LANGUAGE", "言語", "IDIOMA"},
    {"Language", "言語", "Idioma"},
};

// Scratch buffer for keys that carry an ImGui id after "##".
std::string& scratch() {
    static std::string s;
    return s;
}

}  // namespace

const char* tr(const char* key) {
    if (!key) return key;
    if (g_lang != kJa && g_lang != kPtBr) return key;

    // Visible label only: everything after "##" is an ImGui id.
    const char* hash = std::strstr(key, "##");
    const size_t n = hash ? static_cast<size_t>(hash - key) : std::strlen(key);

    for (const Pair& p : kTable) {
        if (p.en[n] != '\0') continue;  // not the same visible length
        if (std::strncmp(p.en, key, n) == 0) {
            const char* out = (g_lang == kJa) ? p.ja : p.pt;
            if (!hash) return out;
            scratch().assign(out).append(hash);
            return scratch().c_str();
        }
    }
    return key;
}

}  // namespace i18n
