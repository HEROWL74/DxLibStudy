#include "GameScene.h"
#include "DxLib.h"
#include "NormalSlime.h"
#include "SpikeSlime.h"

GameScene::GameScene() {
    m_generator = std::make_unique<TitleGenerate>();
    m_player = std::make_unique<Player>();
    m_hud = std::make_unique<HUD>();


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


    for (int i = 0; i < 5; ++i) {
        int cx = startX + i * tileSize;
        int cy = airLevel - tileSize / 2;  // プラットフォームの中心ちょい上
        m_coins.push_back(std::make_unique<Coin>(cx, cy));
    }
}

void GameScene::Update() {


    // プレイヤー更新（入力・物理）
    m_player->Update();

    
        float vy = m_player->GetVY();
        int   px = m_player->GetX();
        int   py = m_player->GetY();
        int   pw = m_player->GetW();
        int   ph = m_player->GetH();

        // ── 2) 頭突き判定（vy<0 のときだけ） ──
        if (vy < 0.0f) {
            float oldY = py - vy;            // １フレーム前の Y
            for (auto& b : m_blocks) {
                int bx = b->GetX();
                int by = b->GetY();
                int bw = b->GetW();
                int bh = b->GetH();
                // 水平重なりチェック
                if (px + pw > bx && px < bx + bw) {
                    float blockBottom = by + bh;
                    // 旧頭頂 ≥ ブロック底面  &&  新頭頂 ≤ ブロック底面
                    if (oldY >= blockBottom && py <= blockBottom) {
                        // ぶつかった！
                        m_player->SetY(blockBottom);    // プレイヤーの頭をブロック底面に合わせる
                        m_player->SetVY(0.0f);          // 上昇停止
                        break;
                    }
                }
            }
        }
        // ── 3) 空中ブロック上での着地判定（vy>0 のとき） ──
        if (vy > 0.0f) {
            float oldBottom = (py - vy) + ph;
            float newBottom = py + ph;
            for (auto& b : m_blocks) {
                int bx = b->GetX();
                int by = b->GetY();
                int bw = b->GetW();
                int bh = b->GetH();
                // 水平重なり
                if (px + pw > bx && px < bx + bw) {
                    // 旧底面 ≤ ブロック上面  &&  新底面 ≥ ブロック上面
                    if (oldBottom <= by && newBottom >= by) {
                        // 着地！
                        m_player->SetY(static_cast<float>(by - ph));
                        m_player->SetVY(0.0f);
                        m_player->SetOnGround(true);
                        break;
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

    for (auto& c : m_coins) {
        c->Update(*m_player);
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
    // コイン描画
    for (auto& c : m_coins) c->Draw();
    // フェードイン描画
    if (m_fadeIn) {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_fadeAlpha);
        DrawBox(0, 0, 1920, 1080, GetColor(0, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
    for (auto& e : m_enemies) {
        e->Draw();
    }
    m_hud->Draw(m_player->GetHealth(),
        m_player->GetMaxHealth(),
        m_player->GetCoinCount());
}
