#include "GameScene.h"
#include "DxLib.h"
#include "NormalSlime.h"
#include "SpikeSlime.h"
#include <memory>
#include <algorithm>  // std::remove_if

GameScene::GameScene()
    : m_fadeIn(true)
    , m_fadeAlpha(255)
{
    // 描画マップ・プレイヤー・HUD を生成
    m_generator = std::make_unique<TitleGenerate>();
    m_player = std::make_unique<Player>();
    m_hud = std::make_unique<HUD>();

    // 敵の生成
    m_enemies.push_back(std::make_unique<NormalSlime>(200, 770));
    m_enemies.push_back(std::make_unique<SpikeSlime>(400, 770));

    // 空中プラットフォームのブロックを並べる
    const int tileSize2 = 64;
    const int mapH2 = m_generator->GetMapHeight();
    const int floorY2 = 1080 - mapH2 * tileSize2;
    const int airLevel = floorY2 - 3 * tileSize2;
    const int startX2 = 500;
    const int count = 5;
    for (int i = 0; i < count; ++i) {
        int bx = startX2 + i * tileSize2;
        m_blocks.push_back(std::make_unique<Block>(bx, airLevel));
    }

    // コイン配置例：空中プラットフォーム上
    const int tileSize = 64;
    int mapH = m_generator->GetMapHeight();
    int floorY = 1080 - mapH * tileSize;
    int airY = floorY - 3 * tileSize;
    for (int i = 0; i < 5; ++i) {
        m_coins.push_back(
            std::make_unique<Coin>(500 + i * tileSize, airY - tileSize / 2)
        );
    }
}

void GameScene::Update() {
    // 1) プレイヤー更新
    m_player->Update();

    // プレイヤー状態取得
    float vy = m_player->GetVY();
    int   px = m_player->GetX();
    int   py = m_player->GetY();
    int   pw = m_player->GetW();
    int   ph = m_player->GetH();

    // 2) 頭突き判定（上方向移動中のみ）
    if (vy < 0.0f) {
        float oldY = py - vy;
        for (auto& b : m_blocks) {
            int bx = b->GetX(), by = b->GetY();
            int bw = b->GetW(), bh = b->GetH();
            if (px + pw > bx && px < bx + bw) {
                float blockBottom = by + bh;
                if (oldY >= blockBottom && py <= blockBottom) {
                    m_player->SetY(blockBottom);
                    m_player->SetVY(0.0f);
                    break;
                }
            }
        }
    }

    // 3) 空中プラットフォーム上での着地判定（下方向移動中のみ）
    if (vy > 0.0f) {
        float oldBottom = (py - vy) + ph;
        float newBottom = py + ph;
        for (auto& b : m_blocks) {
            int bx = b->GetX(), by = b->GetY();
            int bw = b->GetW(), bh = b->GetH();
            if (px + pw > bx && px < bx + bw) {
                if (oldBottom <= by && newBottom >= by) {
                    m_player->SetY(static_cast<float>(by - ph));
                    m_player->SetVY(0.0f);
                    m_player->SetOnGround(true);
                    break;
                }
            }
        }
    }

    // 4) 床（マップ下端）への着地
    {
        const int tileSize = 64;
        const int mapH = m_generator->GetMapHeight();
        const int floorY = 1080 - mapH * tileSize;
        if (vy > 0.0f && py + ph > floorY) {
            m_player->SetY(static_cast<float>(floorY - ph));
            m_player->SetVY(0.0f);
            m_player->SetOnGround(true);
        }
    }

    // 5) 敵更新＋衝突判定
    for (auto& e : m_enemies) {
        e->Update();
        if (e->IsColliding(px, py, pw, ph)) {
            // TODO: ダメージ／ノックバックなど
        }
    }

    // 6) ブロック状態更新
    for (auto& b : m_blocks) {
        b->Update(*m_player);
    }

    // 7) コイン収集判定 → Update とエフェクト生成
    for (auto& c : m_coins) {
        if (!c->isCollected()) {
            c->Update(*m_player);
            if (c->isCollected()) {
                int hearts = m_player->GetMaxHealth() / 2;
                auto pos = m_hud->GetCoinIconPos(hearts);
                int tx = pos.first;
                int ty = pos.second;
                m_coinEffects.push_back(
                    std::make_unique<CoinEffect>(
                        c->GetX(), c->GetY(), tx, ty
                    )
                );
            }
        }
    }

    // 8) CoinEffect 更新・完了処理
    for (auto& e : m_coinEffects) {
        e->Update();
    }
    for (auto it = m_coinEffects.begin(); it != m_coinEffects.end(); ) {
        if ((*it)->IsFinished()) {
            m_player->AddCoin(1);
            it = m_coinEffects.erase(it);
        }
        else {
            ++it;
        }
    }

    // 9) フェードイン制御
    if (m_fadeIn) {
        m_fadeAlpha -= 5;
        if (m_fadeAlpha <= 0) {
            m_fadeAlpha = 0;
            m_fadeIn = false;
        }
    }
}

void GameScene::Draw() {
    // 背景マップ
    m_generator->DrawMap();

    // プレイヤー
    m_player->Draw();

    // 空中プラットフォーム（ブロック）
    for (auto& b : m_blocks) {
        b->Draw();
    }

    // コイン（未収集のみ）
    for (auto& c : m_coins) {
        c->Draw();
    }

    // フェードインオーバーレイ
    if (m_fadeIn) {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_fadeAlpha);
        DrawBox(0, 0, 1920, 1080, GetColor(0, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    // コインエフェクト
    for (auto& e : m_coinEffects) {
        e->Draw();
    }

    // 敵
    for (auto& e : m_enemies) {
        e->Draw();
    }

    // HUD（ハート＋コイン数）
    m_hud->Draw(
        m_player->GetMaxHealth() / 2,
        m_player->GetMaxHealth(),
        m_player->GetCoinCount()
    );
}
