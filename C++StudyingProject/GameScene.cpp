#include "GameScene.h"
#include "DxLib.h"

#include "Stage1Generate.h"
#include "Player.h"
#include "HUD.h"
#include "Block.h"
#include "Coin.h"
#include "CoinEffect.h"
#include "NormalSlime.h"
#include "SpikeSlime.h"

GameScene::GameScene()
    : m_fadeIn(true)
    , m_fadeAlpha(255)
{
    // ステージ１生成
    m_stage1 = std::make_unique<Stage1Generate>();

    // プレイヤー／HUD生成
    m_player = std::make_unique<Player>();
    m_hud = std::make_unique<HUD>();

    // 空中プラットフォーム → Block クラスで描画
    for (auto& r : m_stage1->GetPlatformRects()) {
        m_blocks.push_back(std::make_unique<Block>(r.x, r.y));
    }

    // コイン配置 → Coin クラスで生成
    for (auto& r : m_stage1->GetCoinRects()) {
        m_coins.push_back(std::make_unique<Coin>(r.x, r.y));
    }

    // 敵生成（床の高さ floorY を Stage1Generate から推定）
    int floorY = m_stage1->GetStartPos().y + 64;  // スタート位置直下が床
    m_enemies.push_back(
        std::make_unique<NormalSlime>(200, floorY - 64, static_cast<float>(floorY))
    );
    m_enemies.push_back(
        std::make_unique<SpikeSlime>(400, floorY - 64)
    );
}

void GameScene::Update() {
    // ── はしご登り判定 ──
    {
        int px = m_player->GetX(), py = m_player->GetY();
        int pw = m_player->GetW(), ph = m_player->GetH();
        bool onLadder = false;
        for (auto& r : m_stage1->GetLadders()) {
            if (px + pw > r.x && px < r.x + r.w &&
                py + ph > r.y && py < r.y + r.h) {
                onLadder = true;
                break;
            }
        }
        bool climbInput = CheckHitKey(KEY_INPUT_UP) || CheckHitKey(KEY_INPUT_DOWN);
        m_player->SetClimbing(onLadder && climbInput);
    }

    // ── プレイヤー更新 ──
    m_player->Update();

    float vy = m_player->GetVY();

    // ── スプリングアニメ更新 ──
    m_stage1->UpdateSprings();

    // ── スプリング踏み判定 → 跳ね返り & アニメ開始 ──
    {
        int px = m_player->GetX(), py = m_player->GetY();
        int pw = m_player->GetW(), ph = m_player->GetH();
        auto& springs = m_stage1->GetSpringRects();
        for (int i = 0; i < (int)springs.size(); ++i) {
            auto& r = springs[i];
            if (px + pw > r.x && px < r.x + r.w &&
                py + ph > r.y && py < r.y + r.h) {
                m_player->SetVY(-14.0f);
                m_stage1->ActivateSpring(i);
            }
        }
    }

    // ── スパイク踏み判定 → ダメージ ──
    {
        int px = m_player->GetX(), py = m_player->GetY();
        int pw = m_player->GetW(), ph = m_player->GetH();
        for (auto& r : m_stage1->GetSpikeRects()) {
            if (px + pw > r.x && px < r.x + r.w &&
                py + ph > r.y && py < r.y + r.h) {
                m_player->TakeDamage(1);
            }
        }
    }

    // ── 頭突き判定 ──
    if (vy < 0.0f) {
        int px = m_player->GetX(), py = m_player->GetY();
        int pw = m_player->GetW(), ph = m_player->GetH();
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

    // ── 空中プラットフォーム上着地判定 ──
    if (vy > 0.0f) {
        int px = m_player->GetX(), py = m_player->GetY();
        int pw = m_player->GetW(), ph = m_player->GetH();
        float oldB = (py - vy) + ph, newB = py + ph;
        for (auto& b : m_blocks) {
            int bx = b->GetX(), by = b->GetY();
            int bw = b->GetW();
            if (px + pw > bx && px < bx + bw) {
                if (oldB <= by && newB >= by) {
                    m_player->SetY(static_cast<float>(by - ph));
                    m_player->SetVY(0.0f);
                    m_player->SetOnGround(true);
                    break;
                }
            }
        }
    }

    // ── 床着地判定 ──
    {
        const int mapH = m_stage1->GetStartPos().y / 64 + 1;
        const int floorY = 1080 - mapH * 64;
        int py = m_player->GetY(), ph = m_player->GetH();
        if (vy > 0.0f && py + ph > floorY) {
            m_player->SetY(static_cast<float>(floorY - ph));
            m_player->SetVY(0.0f);
            m_player->SetOnGround(true);
        }
    }

    // ── 敵踏みつけ→Stomp処理 ──
    if (vy > 0.0f) {
        int px = m_player->GetX(), py = m_player->GetY();
        int pw = m_player->GetW(), ph = m_player->GetH();
        float oldB = (py - vy) + ph, newB = py + ph;
        for (auto it = m_enemies.begin(); it != m_enemies.end(); ) {
            auto& e = *it;
            int ex = e->GetX(), ey = e->GetY(), ew = e->GetW();
            if (px + pw > ex && px < ex + ew &&
                oldB <= ey && newB >= ey) {
                it = m_enemies.erase(it);
                m_player->SetVY(-10.0f);
                m_player->SetOnGround(false);
                continue;
            }
            ++it;
        }
    }

    // ── 敵衝突ダメージ＆ノックバック ──
    {
        int px = m_player->GetX(), py = m_player->GetY();
        int pw = m_player->GetW(), ph = m_player->GetH();
        for (auto& e : m_enemies) {
            if (e->IsColliding(px, py, pw, ph)) {
                m_player->TakeDamage(1);
                int ex = e->GetX(), ew = e->GetW();
                float dir = (px + pw / 2 < ex + ew / 2) ? -1.0f : 1.0f;
                m_player->SetVY(-8.0f);
                m_player->SetVX(dir * 5.0f);
                m_player->SetOnGround(false);
                break;
            }
        }
        // 敵更新
        for (auto& e : m_enemies) e->Update();
    }

    // ── ブロック状態更新 ──
    for (auto& b : m_blocks) b->Update(*m_player);

    // ── コイン収集＆エフェクト生成 ──
    for (auto& c : m_coins) {
        if (!c->isCollected()) {
            c->Update(*m_player);
            if (c->isCollected()) {
                float sx = static_cast<float>(c->GetX());
                float sy = static_cast<float>(c->GetY());
                // HUD のコインアイコン位置を取得
                auto pos = m_hud->GetCoinIconPos(m_player->GetMaxHealth() / 2);
                int tx = pos.first;
                int ty = pos.second;
                                // CoinEffect のコンストラクタは (int startX, int startY, int targetX, int targetY)
                m_coinEffects.push_back(
                     std::make_unique<CoinEffect>(
                            static_cast<int>(sx),
                            static_cast<int>(sy),
                            tx,
                            ty
                         )
                );
            }
        }
    }

    // ── CoinEffect 更新＆完了 ──
    for (auto& e : m_coinEffects) e->Update();
    m_coinEffects.erase(
        std::remove_if(
            m_coinEffects.begin(), m_coinEffects.end(),
            [&](auto& e) {
                if (e->IsFinished()) {
                    m_player->AddCoin(1);
                    return true;
                }
                return false;
            }
        ),
        m_coinEffects.end()
    );

    // ── フェードイン制御 ──
    if (m_fadeIn) {
        m_fadeAlpha -= 5;
        if (m_fadeAlpha <= 0) {
            m_fadeAlpha = 0;
            m_fadeIn = false;
        }
    }
}

void GameScene::Draw() {
    // ステージ描画（床、はしご、旗、スパイク、スプリング）
    m_stage1->DrawMap();

    // 空中プラットフォーム
    for (auto& b : m_blocks) b->Draw();

    // コイン（未収集のみ）
    for (auto& c : m_coins) c->Draw();

    // コインエフェクト
    for (auto& e : m_coinEffects) e->Draw();

    // プレイヤー
    m_player->Draw();

    // 敵
    for (auto& e : m_enemies) e->Draw();

    // HUD（ハート数, コイン数）
    m_hud->Draw(
        m_player->GetMaxHealth() / 2,
        m_player->GetMaxHealth(),
        m_player->GetCoinCount()
    );

    // フェードインオーバーレイ
    if (m_fadeIn) {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_fadeAlpha);
        DrawBox(0, 0, 1920, 1080, GetColor(0, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
}
