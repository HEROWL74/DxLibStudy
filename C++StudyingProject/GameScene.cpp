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
    ,m_isGameOver(false)
    , m_gameOverTimer(0)
    , m_gameOverIndex(0)
{
    // ステージ１生成
    m_stage1 = std::make_unique<Stage1Generate>();

    // プレイヤー／HUD生成
    m_player = std::make_unique<Player>();
    m_hud = std::make_unique<HUD>();
    
    //サウンドをロード
    m_coinSound = LoadSoundMem("Sounds/sfx_coin.ogg");
    if (m_coinSound == -1)printfDx("サウンド読み込み失敗: Sounds/sfx.coin.ogg");
    // フォント読み込み（UI/Font/GameFont.ttf を適宜ファイル名に変更）
    m_fontHandle = CreateFontToHandle(
        "UI/Font/GameFont.ttf",  // フォントファイルへの相対パス
        64,                      // フォントサイズ
        5,                       // 太さ
        DX_FONTTYPE_ANTIALIASING // アンチエイリアス
    );
    if (m_fontHandle == -1) {
        printfDx("フォント読み込み失敗: UI/Font/GameFont.ttf\n");
    }
    

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
   

    // ── プレイヤー更新 ──
    m_player->Update();

    float vy = m_player->GetVY();


    {

       

        if (vy > 0.0f) {  // プレイヤーが落下中のとき
            int px = m_player->GetX(), py = m_player->GetY();
            int pw = m_player->GetW(), ph = m_player->GetH();
            float oldB = (py - vy) + ph;  // 前の足位置
            float newB = py + ph;         // 現在の足位置

            for (const auto& r : m_stage1->GetFloorRects()) {
                if (px + pw > r.x && px < r.x + r.w &&
                    oldB <= r.y && newB >= r.y) {
                    m_player->SetY(static_cast<float>(r.y - ph));  // 床上に配置
                    m_player->SetVY(0.0f);                         // 落下停止
                    m_player->SetOnGround(true);                   // 着地フラグ
                    break;
                }
            }
        }

    }
    // ── スプリングアニメ更新 ──
    m_stage1->UpdateSprings();

    // ── スプリング踏み判定 → 跳ね返り & アニメ開始 ──
    {
        int px = m_player->GetX(), py = m_player->GetY();
        int pw = m_player->GetW(), ph = m_player->GetH();
        auto& springs = m_stage1->GetSpringRects();
        int footY = py + ph;
        int prevFootY = (py - vy) + ph;

        for (auto& r : m_stage1->GetSpikeRects()) {
            if (px + pw > r.x && px < r.x + r.w &&
                prevFootY <= r.y && footY >= r.y) {
                m_player->TakeDamage(1);
                break;
            }
        }

    }

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

    // ── スパイク踏み判定 → ダメージ ──
    {
        int px = m_player->GetX(), py = m_player->GetY();
        int pw = m_player->GetW(), ph = m_player->GetH();
        int footY = py + ph;
        int prevFootY = (py - vy) + ph;

        for (auto& r : m_stage1->GetSpikeRects()) {
            if (px + pw > r.x && px < r.x + r.w &&
                prevFootY <= r.y && footY >= r.y) {
                m_player->TakeDamage(1);
                break;
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

  
 
    
    // ── 敵衝突判定：踏みつけ vs 横衝突 ──
    {
        int px = m_player->GetX(), py = m_player->GetY();
        int pw = m_player->GetW(), ph = m_player->GetH();
        float vy = m_player->GetVY();
        float oldBottom = (py - vy) + ph;
        float newBottom = py + ph;

        for (auto it = m_enemies.begin(); it != m_enemies.end(); ) {
            auto& e = *it;
            int ex = e->GetX(), ey = e->GetY();
            int ew = e->GetW(), eh = e->GetH();

            bool overlapX = (px + pw > ex) && (px < ex + ew);
            bool overlapY = (py + ph > ey) && (py < ey + eh);

            if (!overlapX || !overlapY) {
                ++it;
                continue;
            }

            // ① 上から踏みつけ判定：落下中かつ足が敵の頭を通過した場合
            if (vy > 0.0f && oldBottom <= ey && newBottom >= ey) {
                // 踏みつけ成功 → 敵消去 & プレイヤー跳ね返り
                it = m_enemies.erase(it);
                m_player->SetVY(-10.0f);
                m_player->SetOnGround(false);
                continue;
            }

            // ② それ以外は横衝突扱い → ダメージ＋ノックバック
            {
                // １回だけダメージ処理
                m_player->TakeDamage(1);

                // ノックバック方向：プレイヤーの中心 vs 敵の中心
                float playerCenterX = px + pw * 0.5f;
                float enemyCenterX = ex + ew * 0.5f;
                float dir = (playerCenterX < enemyCenterX) ? -1.0f : 1.0f;

                m_player->SetVY(-8.0f);
                m_player->SetVX(dir * 5.0f);
                m_player->SetOnGround(false);
            }

            // 処理済みの敵は消さずに次へ
            ++it;
        }

        // ── 敵更新 ──
        for (auto& e : m_enemies) {
            e->Update();
        }
    }


    // ── ブロック状態更新 ──
    for (auto& b : m_blocks) b->Update(*m_player);

    // ── コイン収集＆エフェクト生成 ──
    for (auto& c : m_coins) {
        if (!c->isCollected()) {
            c->Update(*m_player);
            if (c->isCollected()) {
                PlaySoundMem(m_coinSound, DX_PLAYTYPE_BACK);
                float sx = static_cast<float>(c->GetX());
                float sy = static_cast<float>(c->GetY());
                // HUD のコインアイコン位置を取得
                auto digitPos = m_hud->GetCoinDigitPos(
                    m_player->GetMaxHealth() / 2,
                    m_player->GetCoinCount()
                );
                int tx = digitPos.first;
                int ty = digitPos.second;

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

    // ■ ライフ０でゲームオーバー開始
    if (!m_isGameOver && m_player->GetHealth() <= 0) {
        m_isGameOver = true;
        m_gameOverTimer = 0;
        m_gameOverIndex = 0;
    }

    // ゲームオーバー中は他の Update を止めて文字送りだけ行う
    if (m_isGameOver) {
        ++m_gameOverTimer;
        // 10フレームごとに１文字ずつ表示
        if (m_gameOverTimer % 10 == 0 &&
            m_gameOverIndex < static_cast<int>(m_gameOverText.size()))
        {
            ++m_gameOverIndex;
        }
        return;
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
        m_player->GetHealth(),
        m_player->GetMaxHealth(),
        m_player->GetCoinCount()
    );
    // ■ ゲームオーバー文字表示
    if (m_isGameOver && m_fontHandle >= 0 && m_gameOverIndex > 0) {
        // 今まで表示する文字数分だけ切り出し
        std::string s = m_gameOverText.substr(0, m_gameOverIndex);

        // 画面中央に表示
        int textW = GetDrawStringWidthToHandle(s.c_str(), s.size(), m_fontHandle);
        int x = (1920 - textW) / 2;
        int y = 1080 / 2 - 32;  // 好きな位置に調整してください

        DrawStringToHandle(
            x, y,
            s.c_str(),
            GetColor(255, 0, 0),  // 赤文字
            m_fontHandle
        );

        // フェードインオーバーレイ
        if (m_fadeIn) {
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_fadeAlpha);
            DrawBox(0, 0, 1920, 1080, GetColor(0, 0, 0), TRUE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }

    }
}

bool GameScene::IsHitBottom(const Rect& r, int px, int py, int pw, int ph, float vy)
{
    int prevB = static_cast<int>((py - vy) + ph); // 前フレームの足位置
    int nowB = py + ph;                          // 現在の足位置
    return px + pw > r.x && px < r.x + r.w &&
        prevB <= r.y && nowB >= r.y;
}
