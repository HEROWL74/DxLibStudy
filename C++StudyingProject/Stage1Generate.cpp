#include "Stage1Generate.h"
#include "DxLib.h"

Stage1Generate::Stage1Generate()
    : m_springDuration(15)
{
    LoadTiles();
    SetupMap();
}

Stage1Generate::~Stage1Generate() {
    for (auto& kv : m_th) DeleteGraph(kv.second);
}

void Stage1Generate::LoadTiles() {
    auto L = [&](const std::string& k, const char* p) {
        m_th[k] = LoadGraph(p);
        if (m_th[k] == -1) {
            printfDx("画像の読み込みに失敗しました: %s\n", p);
            __debugbreak();
        }
        };

    // 地面
    L("ground", "Sprites/Tiles/Default/terrain_dirt_block_center.png");
    // はしご
    L("ladder_top", "Sprites/Tiles/Default/ladder_top.png");
    L("ladder_mid", "Sprites/Tiles/Default/ladder_middle.png");
    L("ladder_bot", "Sprites/Tiles/Default/ladder_bottom.png");
    // 旗
    L("flag_a", "Sprites/Tiles/Default/flag_green_a.png");
    L("flag_b", "Sprites/Tiles/Default/flag_green_b.png");
    // スプリング
    L("spring", "Sprites/Tiles/Default/spring.png");
    L("spring_out", "Sprites/Tiles/Default/spring_out.png");
    // スパイク
    L("spikes", "Sprites/Tiles/Default/spikes.png");
}

void Stage1Generate::SetupMap() {
    const int W = 20, H = 10;  // サイズを小さく
    m_layout.assign(H, std::vector<std::string>(W, ""));

    // 地面を作成（一番下）
    int floorRow = H - 1;
    for (int x = 0; x < W; ++x) {
        m_layout[floorRow][x] = "ground";
        m_floors.push_back({ x * TILE, floorRow * TILE, TILE, TILE });
    }

    // スタート位置（左端、地面の上）
    m_start = { 1 * TILE, (floorRow - 1) * TILE, TILE, TILE };

    // ステージ1：基本操作を学ぶシンプルな構成
    // 1. 最初の小さなジャンプ練習用プラットフォーム
    m_platforms.push_back({ 4 * TILE, 7 * TILE, 3 * TILE, TILE });

    // 2. はしごの練習（短いはしご1つだけ）
    {
        int x = 8;
        int topRow = 6;
        int len = 3;

        for (int i = 0; i < len; ++i) {
            int y = topRow + i;
            if (i == 0) m_layout[y][x] = "ladder_top";
            else if (i == len - 1) m_layout[y][x] = "ladder_bot";
            else m_layout[y][x] = "ladder_mid";
            m_ladders.push_back({ x * TILE, y * TILE, TILE, TILE });
        }
    }

    // 3. はしごの上のプラットフォーム
    m_platforms.push_back({ 9 * TILE, 6 * TILE, 4 * TILE, TILE });

    // 4. ゴールプラットフォーム（少し高い位置）
    m_platforms.push_back({ 15 * TILE, 5 * TILE, 3 * TILE, TILE });

    // ゴール旗の配置
    {
        int gx = 16 * TILE;
        int gy = 4 * TILE;
        m_layout[4][16] = "flag_a";
        m_goal = { gx, gy, TILE, TILE };
    }

    // スプリング（1つだけ、ゴール手前で使用）
    {
        int sx = 13 * TILE;
        int sy = 8 * TILE;  // 地面の上
        m_springs.push_back({ {sx, sy, TILE, TILE}, false, 0 });
    }

    // スパイクは配置しない（ステージ1では危険要素を除外）

    // コインの配置（少なめで分かりやすい位置）
    // 最初のプラットフォーム上
    for (int i = 0; i < 2; ++i) {
        int cx = (5 + i) * TILE;
        int cy = 6 * TILE;  // プラットフォームの上
        m_coins.push_back({ cx, cy, TILE / 2, TILE / 2 });
    }

    // はしご上のプラットフォーム
    for (int i = 0; i < 2; ++i) {
        int cx = (10 + i) * TILE;
        int cy = 5 * TILE;
        m_coins.push_back({ cx, cy, TILE / 2, TILE / 2 });
    }

    // ゴール前のボーナスコイン（1つ）
    {
        int cx = 15 * TILE;
        int cy = 4 * TILE;
        m_coins.push_back({ cx, cy, TILE / 2, TILE / 2 });
    }
}

void Stage1Generate::DrawMap() const {
    // レイアウト配列に基づく描画
    for (int y = 0; y < (int)m_layout.size(); ++y) {
        for (int x = 0; x < (int)m_layout[y].size(); ++x) {
            const auto& key = m_layout[y][x];
            if (key.empty()) continue;
            auto it = m_th.find(key);
            if (it != m_th.end()) {
                DrawGraph(x * TILE, y * TILE, it->second, TRUE);
            }
        }
    }

    // スプリングの描画
    auto springIt = m_th.find("spring");
    auto springOutIt = m_th.find("spring_out");
    if (springIt != m_th.end() && springOutIt != m_th.end()) {
        for (const auto& spring : m_springs) {
            int handle = spring.active ? springOutIt->second : springIt->second;
            DrawGraph(spring.r.x, spring.r.y, handle, TRUE);
        }
    }
}

void Stage1Generate::UpdateSprings() {
    for (auto& s : m_springs) {
        if (s.active && --s.timer <= 0) {
            s.active = false;
        }
    }
}

const std::vector<Rect>& Stage1Generate::GetSpringRects() const {
    static std::vector<Rect> tmp;
    tmp.clear();
    for (auto& s : m_springs) tmp.push_back(s.r);
    return tmp;
}

const std::vector<Rect>& Stage1Generate::GetSpikeRects() const {
    return m_spikes;  // 空のベクター
}

void Stage1Generate::ActivateSpring(int index) {
    if (0 <= index && index < (int)m_springs.size()) {
        m_springs[index].active = true;
        m_springs[index].timer = m_springDuration;
    }
}