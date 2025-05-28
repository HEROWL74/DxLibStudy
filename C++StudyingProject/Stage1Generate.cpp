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
            printfDx("�摜�̓ǂݍ��݂Ɏ��s���܂���: %s\n", p);
            __debugbreak();
        }
        };

    // �n��
    L("ground", "Sprites/Tiles/Default/terrain_dirt_block_center.png");
    // �͂���
    L("ladder_top", "Sprites/Tiles/Default/ladder_top.png");
    L("ladder_mid", "Sprites/Tiles/Default/ladder_middle.png");
    L("ladder_bot", "Sprites/Tiles/Default/ladder_bottom.png");
    // ��
    L("flag_a", "Sprites/Tiles/Default/flag_green_a.png");
    L("flag_b", "Sprites/Tiles/Default/flag_green_b.png");
    // �X�v�����O
    L("spring", "Sprites/Tiles/Default/spring.png");
    L("spring_out", "Sprites/Tiles/Default/spring_out.png");
    // �X�p�C�N
    L("spikes", "Sprites/Tiles/Default/spikes.png");
}

void Stage1Generate::SetupMap() {
    const int W = 20, H = 10;  // �T�C�Y��������
    m_layout.assign(H, std::vector<std::string>(W, ""));

    // �n�ʂ��쐬�i��ԉ��j
    int floorRow = H - 1;
    for (int x = 0; x < W; ++x) {
        m_layout[floorRow][x] = "ground";
        m_floors.push_back({ x * TILE, floorRow * TILE, TILE, TILE });
    }

    // �X�^�[�g�ʒu�i���[�A�n�ʂ̏�j
    m_start = { 1 * TILE, (floorRow - 1) * TILE, TILE, TILE };

    // �X�e�[�W1�F��{������w�ԃV���v���ȍ\��
    // 1. �ŏ��̏����ȃW�����v���K�p�v���b�g�t�H�[��
    m_platforms.push_back({ 4 * TILE, 7 * TILE, 3 * TILE, TILE });

    // 2. �͂����̗��K�i�Z���͂���1�����j
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

    // 3. �͂����̏�̃v���b�g�t�H�[��
    m_platforms.push_back({ 9 * TILE, 6 * TILE, 4 * TILE, TILE });

    // 4. �S�[���v���b�g�t�H�[���i���������ʒu�j
    m_platforms.push_back({ 15 * TILE, 5 * TILE, 3 * TILE, TILE });

    // �S�[�����̔z�u
    {
        int gx = 16 * TILE;
        int gy = 4 * TILE;
        m_layout[4][16] = "flag_a";
        m_goal = { gx, gy, TILE, TILE };
    }

    // �X�v�����O�i1�����A�S�[����O�Ŏg�p�j
    {
        int sx = 13 * TILE;
        int sy = 8 * TILE;  // �n�ʂ̏�
        m_springs.push_back({ {sx, sy, TILE, TILE}, false, 0 });
    }

    // �X�p�C�N�͔z�u���Ȃ��i�X�e�[�W1�ł͊댯�v�f�����O�j

    // �R�C���̔z�u�i���Ȃ߂ŕ�����₷���ʒu�j
    // �ŏ��̃v���b�g�t�H�[����
    for (int i = 0; i < 2; ++i) {
        int cx = (5 + i) * TILE;
        int cy = 6 * TILE;  // �v���b�g�t�H�[���̏�
        m_coins.push_back({ cx, cy, TILE / 2, TILE / 2 });
    }

    // �͂�����̃v���b�g�t�H�[��
    for (int i = 0; i < 2; ++i) {
        int cx = (10 + i) * TILE;
        int cy = 5 * TILE;
        m_coins.push_back({ cx, cy, TILE / 2, TILE / 2 });
    }

    // �S�[���O�̃{�[�i�X�R�C���i1�j
    {
        int cx = 15 * TILE;
        int cy = 4 * TILE;
        m_coins.push_back({ cx, cy, TILE / 2, TILE / 2 });
    }
}

void Stage1Generate::DrawMap() const {
    // ���C�A�E�g�z��Ɋ�Â��`��
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

    // �X�v�����O�̕`��
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
    return m_spikes;  // ��̃x�N�^�[
}

void Stage1Generate::ActivateSpring(int index) {
    if (0 <= index && index < (int)m_springs.size()) {
        m_springs[index].active = true;
        m_springs[index].timer = m_springDuration;
    }
}