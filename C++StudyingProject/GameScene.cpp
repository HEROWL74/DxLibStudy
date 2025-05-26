#include "GameScene.h"
#include "DxLib.h"
#include "NormalSlime.h"
#include "SpikeSlime.h"

GameScene::GameScene() {
    m_generator = std::make_unique<TitleGenerate>();
    m_player = std::make_unique<Player>();

    // ── 敵生成（そのまま） ──
    m_enemies.push_back(std::make_unique<NormalSlime>(200, 800));
    m_enemies.push_back(std::make_unique<SpikeSlime>(400, 800));

    // ── 空中プラットフォームとしてブロックを配置 ──
    const int tileSize = 64;
    const int mapH = m_generator->GetMapHeight();
    const int floorY = 1080 - mapH * tileSize;       // 地上床のY座標
    const int airLevel = floorY - 3 * tileSize;        // 床から3行分上

    // X 座標はお好みで。ここでは横並びに 5 つ配置する例
    int startX = 500;
    int count = 5;
    for (int i = 0; i < count; ++i) {
        int bx = startX + i * tileSize;   // タイル幅ずつ右へ
        m_blocks.push_back(std::make_unique<Block>(bx, airLevel));
    }
    m_fadeIn = true;
    m_fadeAlpha = 255;
}

void GameScene::Update() {
    // プレイヤー前フレーム位置・サイズ取得
    int prevX = m_player->GetX();
    int prevY = m_player->GetY();
    int pw = m_player->GetW();
    int ph = m_player->GetH();

    // プレイヤー更新（入力・物理）
    m_player->Update();

    {
        float vy = m_player->GetVY();
        // 下方向に移動中だけ判定
        if (vy > 0.0f) {
            int px = m_player->GetX();
            int py = m_player->GetY();
            int pw = m_player->GetW();
            int ph = m_player->GetH();
            float oldBottom = (py - vy) + ph;
            float newBottom = py + ph;

            for (auto& b : m_blocks) {
                int bx = b->GetX();
                int by = b->GetY();
                int bw = b->GetW();
                int bh = b->GetH();
                // 水平範囲重なり
                if (px + pw > bx && px < bx + bw) {
                    // 旧底辺がブロックの上面より上（＜）で、新底辺が下面より下（>=）なら着地
                    if (oldBottom <= by && newBottom >= by) {
                        // 着地処理
                        m_player->SetY(static_cast<float>(by - ph));
                        m_player->SetVY(0.0f);
                        m_player->SetOnGround(true);
                        break;
                    }
                }
            }
        }
    }

    // 4) 床（マップ下端）との当たり判定
    {
        const int tileSize = 64;
        const int mapH = m_generator->GetMapHeight();
        const int floorY = 1080 - mapH * tileSize;
        int       py = m_player->GetY();
        float     vy = m_player->GetVY();

        if (vy > 0 && py + ph > floorY) {
            m_player->SetY(floorY - ph);
            m_player->SetVY(0);
            m_player->SetOnGround(true);
        }
    }

    // 5) 敵更新＋衝突判定
    for (auto& e : m_enemies) {
        e->Update();
        int px = m_player->GetX();
        int py = m_player->GetY();
        int pw = m_player->GetW();
        int ph = m_player->GetH();
        if (e->IsColliding(px, py, pw, ph)) {
            // TODO: 衝突時の処理（ダメージ、ノックバック、SEなど）
        }
    }

    for (auto& b : m_blocks) {
        b->Update(*m_player);
    }

    // 6) フェードイン制御
    if (m_fadeIn) {
        m_fadeAlpha -= 5;
        if (m_fadeAlpha <= 0) {
            m_fadeAlpha = 0;
            m_fadeIn = false;
        }
    }
}


void GameScene::Draw() {
    m_generator->DrawMap();
    m_player->Draw();
    //ブロックは床の上に描画
    for (auto& b : m_blocks) {
        b->Draw();
    }
    // フェードイン描画
    if (m_fadeIn) {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_fadeAlpha);
        DrawBox(0, 0, 1920, 1080, GetColor(0, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
    for (auto& e : m_enemies) {
        e->Draw();
    }

}
