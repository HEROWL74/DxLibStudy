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
            __debugbreak();  // デバッグ中なら明示的に止める
        }
        };
    // 床
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
    const int W = 30, H = 12;
    m_layout.assign(H, std::vector<std::string>(W, "")); // 空で初期化

    int floorRow = H - 1;
    for (int x = 0; x < W; ++x) {
        m_layout[floorRow][x] = "ground";
    }

    m_start = { (W - 1) * TILE, (floorRow - 1) * TILE, TILE, TILE };

    struct PS { int x, y, len; };
    std::vector<PS> plats = {
        {  4, 8,  5 },
        { 12, 5,  7 },
        { 20, 3,  4 }
    };
    for (auto& p : plats) {
        m_platforms.push_back({ p.x * TILE, p.y * TILE, p.len * TILE, TILE });
    }

    struct LS { int x, len; };
    std::vector<LS> ladders = {
        {  6, 4 },
        { 16, 6 },
        { 28, 3 }
    };
    for (auto& l : ladders) {
        int x = l.x;
        int topRow = floorRow - l.len + 1;
        for (int i = 0; i < l.len; ++i) {
            int y = topRow + i;
            if (y < 0 || y >= H || x < 0 || x >= W) continue;
            if (i == 0) m_layout[y][x] = "ladder_top";
            else if (i == l.len - 1) m_layout[y][x] = "ladder_bot";
            else m_layout[y][x] = "ladder_mid";
            m_ladders.push_back({ x * TILE, y * TILE, TILE, TILE });
        }
    }

    // ゴール旗配置（正しいインデックスでm_layoutに行単位アクセス）
    {
        LS& topL = ladders[1];
        int gx = (topL.x + 1) * TILE;
        int gyRow = floorRow - topL.len + 0;
        int gy = gyRow * TILE;

        if (gyRow >= 0 && gyRow < H && (topL.x + 1) < W) {
            m_layout[gyRow][topL.x + 1] = "flag_a";
            m_goal = { gx, gy, TILE, TILE };
        }
        else {
            printfDx("旗の配置座標が不正です\n");
            __debugbreak();
        }
    }

    std::vector<int> springOnPlat = { 0, 2 };
    for (int idx : springOnPlat) {
        auto& pr = plats[idx];
        int sx = pr.x + (pr.len / 2 - 1) * TILE;
        int sy = pr.y * TILE - TILE;
        m_springs.push_back({ {sx, sy, TILE, TILE}, false, 0 });
    }

    std::vector<int> spikeOnPlat = { 1, 2 };
    for (int idx : spikeOnPlat) {
        auto& pr = plats[idx];
        int sx = pr.x * TILE;
        int sy = pr.y * TILE - TILE;
        m_spikes.push_back({ sx, sy, TILE, TILE });
    }

    int coinsPerPlatform = 5;
    for (auto& pr : m_platforms) {
        for (int i = 0; i < coinsPerPlatform; ++i) {
            float fx = pr.x + (pr.w / (coinsPerPlatform + 1)) * (i + 1);
            float fy = pr.y - TILE / 2;
            m_coins.push_back({ (int)fx, (int)fy, TILE / 2, TILE / 2 });
        }
    }
}

void Stage1Generate::DrawMap() const {
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
    return m_spikes;
}

void Stage1Generate::ActivateSpring(int index) {
    if (0 <= index && index < (int)m_springs.size()) {
        m_springs[index].active = true;
        m_springs[index].timer = m_springDuration;
    }
}
