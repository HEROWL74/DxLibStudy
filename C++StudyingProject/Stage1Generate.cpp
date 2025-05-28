#include "Stage1Generate.h"
#include "DxLib.h"

Stage1Generate::Stage1Generate()
    : m_cameraX(0)
    , m_startPos(100, 800)
    , m_goalPos(6200, 800)
    , m_grassImg(-1)
    , m_stoneImg(-1)
    , m_spikeImg(-1)
    , m_ladderImg(-1)
    , m_springImg(-1)
    , m_flagImg(-1)
    , m_bgImg(-1)
{
    LoadGraphics();
    GenerateStage();
}

Stage1Generate::~Stage1Generate() {
    // グラフィックハンドルの解放
    if (m_grassImg != -1) DeleteGraph(m_grassImg);
    if (m_stoneImg != -1) DeleteGraph(m_stoneImg);
    if (m_spikeImg != -1) DeleteGraph(m_spikeImg);
    if (m_ladderImg != -1) DeleteGraph(m_ladderImg);
    if (m_springImg != -1) DeleteGraph(m_springImg);
    if (m_flagImg != -1) DeleteGraph(m_flagImg);
    if (m_bgImg != -1) DeleteGraph(m_bgImg);
}

void Stage1Generate::LoadGraphics() {
    // 画像ファイルの読み込み（存在しない場合は色付きの矩形で代用）
    m_grassImg = LoadGraph("Graphics/grass_tile.png");
    m_stoneImg = LoadGraph("Graphics/stone_tile.png");
    m_spikeImg = LoadGraph("Graphics/spike.png");
    m_ladderImg = LoadGraph("Graphics/ladder.png");
    m_springImg = LoadGraph("Graphics/spring.png");
    m_flagImg = LoadGraph("Graphics/flag.png");
    m_bgImg = LoadGraph("Graphics/background.png");
}

void Stage1Generate::GenerateStage() {
    // ステージ全体の床を生成
    // 基本の床（地面）
    for (int x = 0; x < m_stageWidth; x += TILE_SIZE) {
        m_floorRects.emplace_back(x, 980, TILE_SIZE, 100);  // 下部の厚い床
    }

    // セクション1: 開始エリア（0-800）
    // 平坦な地面とコイン
    for (int i = 0; i < 5; i++) {
        m_coinRects.emplace_back(200 + i * 100, 900, 32, 32);
    }

    // 最初のプラットフォーム
    m_platformRects.emplace_back(300, 850, 128, 32);
    m_platformRects.emplace_back(500, 800, 128, 32);
    m_coinRects.emplace_back(520, 750, 32, 32);

    // セクション2: 階段エリア（800-1600）
    // 階段状のプラットフォーム
    for (int i = 0; i < 6; i++) {
        m_platformRects.emplace_back(800 + i * 120, 900 - i * 50, 96, 32);
        if (i % 2 == 0) {
            m_coinRects.emplace_back(820 + i * 120, 850 - i * 50, 32, 32);
        }
    }

    // セクション3: スパイクエリア（1600-2400）
    // スパイクの配置
    for (int i = 0; i < 8; i++) {
        m_spikeRects.emplace_back(1600 + i * 80, 980, 64, 32);
    }

    // スパイクを避けるための高いプラットフォーム
    m_platformRects.emplace_back(1650, 750, 128, 32);
    m_platformRects.emplace_back(1900, 700, 128, 32);
    m_platformRects.emplace_back(2150, 650, 128, 32);

    // 高いところにコイン
    m_coinRects.emplace_back(1670, 700, 32, 32);
    m_coinRects.emplace_back(1920, 650, 32, 32);
    m_coinRects.emplace_back(2170, 600, 32, 32);

    // セクション4: はしごエリア（2400-3200）
    // 縦の移動要素
    m_ladders.emplace_back(2500, 700, 64, 280);  // 床から上層へのはしご
    m_platformRects.emplace_back(2400, 700, 200, 32);  // はしご下のプラットフォーム
    m_platformRects.emplace_back(2450, 500, 150, 32);  // はしご上のプラットフォーム

    // はしご周辺のコイン
    for (int i = 0; i < 3; i++) {
        m_coinRects.emplace_back(2420 + i * 60, 650, 32, 32);
    }
    m_coinRects.emplace_back(2480, 450, 32, 32);

    // 2つ目のはしご
    m_ladders.emplace_back(2800, 600, 64, 380);
    m_platformRects.emplace_back(2750, 600, 150, 32);
    m_platformRects.emplace_back(2800, 400, 150, 32);

    // セクション5: ジャンプチャレンジエリア（3200-4000）
    // 連続ジャンプが必要な浮島
    for (int i = 0; i < 6; i++) {
        m_platformRects.emplace_back(3200 + i * 140, 800 - (i % 3) * 80, 96, 32);
        m_coinRects.emplace_back(3220 + i * 140, 750 - (i % 3) * 80, 32, 32);
    }

    // 高難度ジャンプエリア
    m_platformRects.emplace_back(3950, 650, 96, 32);
    m_coinRects.emplace_back(3970, 600, 32, 32);

    // セクション6: スプリングエリア（4000-4800）
    // スプリングで高く跳ぶエリア
    m_springs.emplace_back(4100, 980);
    m_platformRects.emplace_back(4050, 600, 128, 32);
    m_coinRects.emplace_back(4070, 550, 32, 32);

    m_springs.emplace_back(4400, 980);
    m_platformRects.emplace_back(4350, 500, 128, 32);

    // 複数段のプラットフォーム
    m_platformRects.emplace_back(4600, 800, 96, 32);
    m_platformRects.emplace_back(4700, 700, 96, 32);
    m_platformRects.emplace_back(4800, 600, 96, 32);

    // セクション7: 複合チャレンジエリア（4800-5600）
    // スパイク、はしご、プラットフォームの組み合わせ
    for (int i = 0; i < 4; i++) {
        m_spikeRects.emplace_back(4900 + i * 100, 980, 64, 32);
    }

    m_ladders.emplace_back(5000, 700, 64, 280);
    m_platformRects.emplace_back(4950, 700, 150, 32);
    m_platformRects.emplace_back(5100, 550, 150, 32);

    // 浮島チェーン
    for (int i = 0; i < 4; i++) {
        m_platformRects.emplace_back(5300 + i * 120, 750 - i * 30, 96, 32);
        m_coinRects.emplace_back(5320 + i * 120, 700 - i * 30, 32, 32);
    }

    // セクション8: 最終エリア（5600-6400）
    // ゴール前の最後のチャレンジ
    m_springs.emplace_back(5700, 980);
    m_platformRects.emplace_back(5650, 650, 128, 32);

    // 最終ジャンプ
    m_platformRects.emplace_back(5900, 750, 96, 32);
    m_platformRects.emplace_back(6050, 700, 96, 32);

    // ゴールエリア
    m_platformRects.emplace_back(6150, 850, 200, 32);

    // ボーナスコイン（高いところや隠れた場所）
    m_coinRects.emplace_back(100, 750, 32, 32);    // 開始直後の高い場所
    m_coinRects.emplace_back(1200, 600, 32, 32);   // 階段の上
    m_coinRects.emplace_back(3000, 300, 32, 32);   // はしごの最上部
    m_coinRects.emplace_back(4200, 400, 32, 32);   // スプリング到達地点
    m_coinRects.emplace_back(5500, 500, 32, 32);   // 複合エリアの隠しコイン
    m_coinRects.emplace_back(6000, 650, 32, 32);   // ゴール前
}

void Stage1Generate::DrawMap() {
    DrawTiles();
    DrawObjects();
}

void Stage1Generate::DrawTiles() {
    // 背景
    if (m_bgImg != -1) {
        // 背景をスクロールに合わせて描画（パララックス効果）
        int bgX = -(m_cameraX / 4) % 1920;  // 背景は1/4の速度でスクロール
        DrawGraph(bgX, 0, m_bgImg, TRUE);
        DrawGraph(bgX + 1920, 0, m_bgImg, TRUE);
    }
    else {
        // 背景がない場合は空色で塗りつぶし
        DrawBox(0, 0, 1920, 1080, GetColor(135, 206, 235), TRUE);
    }

    // 床（地面）の描画 - カメラオフセットを適用
    for (const auto& floor : m_floorRects) {
        int drawX = floor.x - m_cameraX;  // カメラオフセットを適用
        int drawY = floor.y;              // Y座標はそのまま

        // 画面外カリング
        if (drawX + floor.w >= 0 && drawX < 1920) {
            if (m_grassImg != -1) {
                // タイル状に描画
                for (int x = 0; x < floor.w; x += TILE_SIZE) {
                    for (int y = 0; y < floor.h; y += TILE_SIZE) {
                        int tileDrawX = drawX + x;
                        int tileDrawY = drawY + y;
                        if (tileDrawX + TILE_SIZE >= 0 && tileDrawX < 1920) {
                            DrawGraph(tileDrawX, tileDrawY, m_grassImg, TRUE);
                        }
                    }
                }
            }
            else {
                DrawBox(drawX, drawY, drawX + floor.w, drawY + floor.h,
                    GetColor(34, 139, 34), TRUE);  // 緑色
            }
        }
    }

    // プラットフォームの描画 - カメラオフセットを適用
    for (const auto& platform : m_platformRects) {
        int drawX = platform.x - m_cameraX;  // カメラオフセットを適用
        int drawY = platform.y;              // Y座標はそのまま

        // 画面外カリング
        if (drawX + platform.w >= 0 && drawX < 1920) {
            if (m_stoneImg != -1) {
                for (int x = 0; x < platform.w; x += TILE_SIZE) {
                    int tileDrawX = drawX + x;
                    if (tileDrawX + TILE_SIZE >= 0 && tileDrawX < 1920) {
                        DrawGraph(tileDrawX, drawY, m_stoneImg, TRUE);
                    }
                }
            }
            else {
                DrawBox(drawX, drawY, drawX + platform.w, drawY + platform.h,
                    GetColor(128, 128, 128), TRUE);  // 灰色
            }
        }
    }
}

void Stage1Generate::DrawObjects() {
    // スパイクの描画 - カメラオフセットを適用
    for (const auto& spike : m_spikeRects) {
        int drawX = spike.x - m_cameraX;  // カメラオフセットを適用
        int drawY = spike.y;

        // 画面外カリング
        if (drawX + spike.w >= 0 && drawX < 1920) {
            if (m_spikeImg != -1) {
                DrawGraph(drawX, drawY, m_spikeImg, TRUE);
            }
            else {
                // 三角形でスパイクを表現
                DrawTriangle(drawX, drawY + spike.h,
                    drawX + spike.w / 2, drawY,
                    drawX + spike.w, drawY + spike.h,
                    GetColor(255, 0, 0), TRUE);
            }
        }
    }

    // はしごの描画 - カメラオフセットを適用
    for (const auto& ladder : m_ladders) {
        int drawX = ladder.x - m_cameraX;  // カメラオフセットを適用
        int drawY = ladder.y;

        // 画面外カリング
        if (drawX + ladder.w >= 0 && drawX < 1920) {
            if (m_ladderImg != -1) {
                for (int y = 0; y < ladder.h; y += TILE_SIZE) {
                    DrawGraph(drawX, drawY + y, m_ladderImg, TRUE);
                }
            }
            else {
                // 線でははしごを表現
                DrawBox(drawX + 10, drawY, drawX + 15, drawY + ladder.h,
                    GetColor(139, 69, 19), TRUE);
                DrawBox(drawX + ladder.w - 15, drawY, drawX + ladder.w - 10, drawY + ladder.h,
                    GetColor(139, 69, 19), TRUE);
                // 横棒
                for (int y = 0; y < ladder.h; y += 20) {
                    DrawBox(drawX + 10, drawY + y, drawX + ladder.w - 10, drawY + y + 5,
                        GetColor(139, 69, 19), TRUE);
                }
            }
        }
    }

    // スプリングの描画 - カメラオフセットを適用
    for (const auto& spring : m_springs) {
        int drawX = spring.x - m_cameraX;  // カメラオフセットを適用
        int drawY = spring.y;

        // 画面外カリング
        if (drawX + TILE_SIZE >= 0 && drawX < 1920) {
            if (m_springImg != -1) {
                DrawGraph(drawX, drawY - 32, m_springImg, TRUE);
            }
            else {
                // 簡単なスプリング表現
                int height = spring.isPressed ? 20 : 32;
                DrawBox(drawX + 16, drawY - height, drawX + 48, drawY,
                    GetColor(255, 215, 0), TRUE);  // 金色
                // コイル表現
                for (int i = 0; i < 3; i++) {
                    DrawCircle(drawX + 32, drawY - height + i * 8, 12,
                        GetColor(255, 165, 0), FALSE);
                }
            }
        }
    }

    // ゴールフラグの描画 - カメラオフセットを適用
    int flagDrawX = m_goalPos.x - m_cameraX;  // カメラオフセットを適用
    int flagDrawY = m_goalPos.y;

    // 画面外カリング
    if (flagDrawX >= -100 && flagDrawX < 1920) {
        if (m_flagImg != -1) {
            DrawGraph(flagDrawX, flagDrawY - 100, m_flagImg, TRUE);
        }
        else {
            // ポール
            DrawBox(flagDrawX, flagDrawY - 100, flagDrawX + 8, flagDrawY,
                GetColor(139, 69, 19), TRUE);
            // フラグ
            DrawBox(flagDrawX + 8, flagDrawY - 100, flagDrawX + 68, flagDrawY - 60,
                GetColor(255, 0, 0), TRUE);
        }
    }
}

void Stage1Generate::UpdateSprings() {
    for (auto& spring : m_springs) {
        if (spring.isPressed) {
            spring.animFrame++;
            if (spring.animFrame > 10) {
                spring.isPressed = false;
                spring.animFrame = 0;
            }
        }
    }
}