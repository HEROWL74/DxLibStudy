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
#include "Camera.h"

GameScene::~GameScene() {
    // unique_ptrは自動的に削除されるが、明示的に定義することで
    // 完全な型定義が利用可能な場所でデストラクタが呼ばれることを保証
}

GameScene::GameScene()
    : m_fadeIn(true)
    , m_fadeAlpha(255)
    , m_isGameOver(false)
    , m_gameOverTimer(0)
    , m_gameOverIndex(0)
{
    // ステージ１生成
    m_stage1 = std::make_unique<Stage1Generate>();

    // プレイヤー／HUD／カメラ生成
    m_player = std::make_unique<Player>();
    m_hud = std::make_unique<HUD>();
    m_camera = std::make_unique<Camera>();

    // サウンドをロード
    m_coinSound = LoadSoundMem("Sounds/sfx_coin.ogg");
    if (m_coinSound == -1) printfDx("サウンド読み込み失敗: Sounds/sfx_coin.ogg");

    // フォント読み込み
    m_fontHandle = CreateFontToHandle(
        "UI/Font/GameFont.ttf",
        64,
        5,
        DX_FONTTYPE_ANTIALIASING
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

    // 敵生成（複数箇所に配置）
    m_enemies.push_back(std::make_unique<NormalSlime>(600, 916, 980.0f));
    m_enemies.push_back(std::make_unique<SpikeSlime>(1200, 916));
    m_enemies.push_back(std::make_unique<NormalSlime>(2000, 916, 980.0f));
    m_enemies.push_back(std::make_unique<SpikeSlime>(2800, 916));
    m_enemies.push_back(std::make_unique<NormalSlime>(3500, 916, 980.0f));
    m_enemies.push_back(std::make_unique<SpikeSlime>(4200, 916));
    m_enemies.push_back(std::make_unique<NormalSlime>(5000, 916, 980.0f));
    m_enemies.push_back(std::make_unique<SpikeSlime>(5800, 916));
}

void GameScene::Update() {
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

        // ゲームオーバー表示完了後、キー入力でリスタート
        if (m_gameOverIndex >= static_cast<int>(m_gameOverText.size()) &&
            m_gameOverTimer > 60) { // 文字表示完了後、1秒待機
            if (CheckHitKey(KEY_INPUT_R) || CheckHitKey(KEY_INPUT_SPACE) ||
                CheckHitKey(KEY_INPUT_RETURN)) {
                RestartGame();
            }
        }
        return;
    }

    // ── プレイヤー更新 ──
    m_player->Update();

    // ── カメラ更新（プレイヤーに追従） ──
    m_camera->Update(*m_player, *m_stage1);
    m_stage1->SetCameraX(m_camera->GetX());

    float vy = m_player->GetVY();

    // ── 床着地判定 ──
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

    // ── スプリングアニメ更新 ──
    m_stage1->UpdateSprings();

    // ── スプリング踏み判定 ──
    {
        int px = m_player->GetX(), py = m_player->GetY();
        int pw = m_player->GetW(), ph = m_player->GetH();
        float vy = m_player->GetVY();

        if (vy > 0.0f) {  // 落下中
            for (const auto& spring : m_stage1->GetSprings()) {
                int sx = spring.x, sy = spring.y;
                if (px + pw > sx && px < sx + 64 &&
                    py + ph >= sy - 32 && py + ph <= sy) {
                    m_player->SetVY(-20.0f);  // 強いジャンプ
                    m_player->SetOnGround(false);
                    // スプリングアニメーション（Stage1Generateで処理）
                    break;
                }
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
        int prevFootY = static_cast<int>((py - vy) + ph);

        for (auto& r : m_stage1->GetSpikeRects()) {
            if (px + pw > r.x && px < r.x + r.w &&
                prevFootY <= r.y && footY >= r.y) {
                m_player->TakeDamage(-1);
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
                m_player->TakeDamage(-1);

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
    }

    // ── 敵更新 ──
    for (auto& e : m_enemies) {
        e->Update();
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
                        static_cast<int>(sx - m_camera->GetX()),  // カメラ座標を考慮
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

    // ── ゴール判定 ──
    {
        int px = m_player->GetX(), py = m_player->GetY();
        int pw = m_player->GetW(), ph = m_player->GetH();
        Point goal = m_stage1->GetGoalPos();

        if (px + pw > goal.x && px < goal.x + 100 &&
            py + ph > goal.y && py < goal.y + 100) {
            // ゴール到達！（必要に応じてシーン切り替えなど）
            printfDx("Goal Reached!\n");
        }
    }

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

    // カメラ座標を取得
    int cameraX = m_camera->GetX();

    // 描画座標変換のためのオフセットを設定
    // DxLibのカメラ機能を使用して座標変換
    SetCameraScreenCenter(960.0f, 540.0f);  // 画面中央を基準
 

    // 空中プラットフォーム（元のDraw()メソッドを使用）
    for (auto& b : m_blocks) {
        int blockX = b->GetX();
        if (blockX + b->GetW() >= cameraX && blockX < cameraX + 1920) {  // 画面内のみ描画
            b->Draw();  // 元のDraw()メソッドを使用
        }
    }

    // コイン（未収集のみ、元のDraw()メソッドを使用）
    for (auto& c : m_coins) {
        int coinX = c->GetX();
        if (coinX + 32 >= cameraX && coinX < cameraX + 1920) {  // 画面内のみ描画
            c->Draw();  // 元のDraw()メソッドを使用
        }
    }

    // プレイヤー（元のDraw()メソッドを使用）
    m_player->Draw();

    // 敵（元のDraw()メソッドを使用）
    for (auto& e : m_enemies) {
        int enemyX = e->GetX();
        if (enemyX + e->GetW() >= cameraX && enemyX < cameraX + 1920) {  // 画面内のみ描画
            e->Draw();  // 元のDraw()メソッドを使用
        }
    }

    // カメラをリセットして UI 描画用に戻す
 

    // コインエフェクト（スクリーン座標なのでそのまま）
    for (auto& e : m_coinEffects) e->Draw();

    // HUD（ハート数, コイン数）- 常に画面に固定
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
        int y = 1080 / 2 - 32;

        DrawStringToHandle(
            x, y,
            s.c_str(),
            GetColor(255, 0, 0),  // 赤文字
            m_fontHandle
        );

        // リスタート指示を表示（文字表示完了後）
        if (m_gameOverIndex >= static_cast<int>(m_gameOverText.size()) &&
            m_gameOverTimer > 60) {
            std::string restartText = "Press R, SPACE or ENTER to Restart";
            int restartW = GetDrawStringWidthToHandle(restartText.c_str(), restartText.size(), m_fontHandle);
            int restartX = (1920 - restartW) / 2;
            int restartY = y + 80;

            DrawStringToHandle(
                restartX, restartY,
                restartText.c_str(),
                GetColor(255, 255, 255),  // 白文字
                m_fontHandle
            );
        }
    }

    // デバッグ情報表示（カメラとプレイヤー位置）
    DrawFormatString(10, 10, GetColor(255, 255, 255),
        "Camera: %d, Player: %d, Screen: %d",
        cameraX, m_player->GetX(), m_player->GetX() - cameraX);

    // フェードインオーバーレイ
    if (m_fadeIn) {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_fadeAlpha);
        DrawBox(0, 0, 1920, 1080, GetColor(0, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
}

bool GameScene::IsHitBottom(const Rect& r, int px, int py, int pw, int ph, float vy)
{
    int prevB = static_cast<int>((py - vy) + ph); // 前フレームの足位置
    int nowB = py + ph;                          // 現在の足位置
    return px + pw > r.x && px < r.x + r.w &&
        prevB <= r.y && nowB >= r.y;
}

void GameScene::RestartGame()
{
    // ゲームオーバー状態をリセット
    m_isGameOver = false;
    m_gameOverTimer = 0;
    m_gameOverIndex = 0;

    // フェードイン再開
    m_fadeIn = true;
    m_fadeAlpha = 255;

    // プレイヤーをリセット（新しいPlayerインスタンスを作成）
    m_player.reset();
    m_player = std::make_unique<Player>();

    // カメラリセット
    m_camera.reset();
    m_camera = std::make_unique<Camera>();

    // 敵をリセット
    m_enemies.clear();
    m_enemies.push_back(std::make_unique<NormalSlime>(600, 916, 980.0f));
    m_enemies.push_back(std::make_unique<SpikeSlime>(1200, 916));
    m_enemies.push_back(std::make_unique<NormalSlime>(2000, 916, 980.0f));
    m_enemies.push_back(std::make_unique<SpikeSlime>(2800, 916));
    m_enemies.push_back(std::make_unique<NormalSlime>(3500, 916, 980.0f));
    m_enemies.push_back(std::make_unique<SpikeSlime>(4200, 916));
    m_enemies.push_back(std::make_unique<NormalSlime>(5000, 916, 980.0f));
    m_enemies.push_back(std::make_unique<SpikeSlime>(5800, 916));

    // コインをリセット
    m_coins.clear();
    for (auto& r : m_stage1->GetCoinRects()) {
        m_coins.push_back(std::make_unique<Coin>(r.x, r.y));
    }

    // コインエフェクトをクリア
    m_coinEffects.clear();

    // ブロックをリセット（必要に応じて）
    m_blocks.clear();
    for (auto& r : m_stage1->GetPlatformRects()) {
        m_blocks.push_back(std::make_unique<Block>(r.x, r.y));
    }
}