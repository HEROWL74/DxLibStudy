#include "GrassStage.h"

GrassStage::GrassStage() {
    terrainType = "grass";
}

GrassStage::~GrassStage() {
    // 基底クラスのデストラクタが呼ばれる
}

void GrassStage::Initialize() {
    // テクスチャ読み込み
    LoadTerrainTextures();

    // 地形生成
    CreateMainTerrain();
    CreatePlatforms();
}

void GrassStage::CreateMainTerrain() {
    // 基本地面の生成（全体にわたる地面）
    int groundLevel = 12; // グリッド座標
    int groundHeight = 4; // 地面の厚さ

    // 拡張したメインの部分のプラットフォーム（120タイル使用）
    CreatePlatform(0, groundLevel, 120, groundHeight);

    // 地面に隙間を作る（ギャップジャンプ用）- 間隔を広げて詰まりを防止
    // 隙間1（初級）- 幅を3タイルに拡張
    CreatePlatform(15, groundLevel, 8, groundHeight);  // 左の地面
    CreatePlatform(26, groundLevel, 8, groundHeight);  // 右の地面
    // 23-26の間が隙間（3タイル幅）

    // 隙間2（中級）- 幅を4タイルに拡張
    CreatePlatform(40, groundLevel, 8, groundHeight);  // 左の地面
    CreatePlatform(52, groundLevel, 8, groundHeight);  // 右の地面
    // 48-52の間が隙間（4タイル幅）

    // 隙間3（上級）- 幅を5タイルに拡張
    CreatePlatform(70, groundLevel, 8, groundHeight);  // 左の地面
    CreatePlatform(83, groundLevel, 8, groundHeight);  // 右の地間
    // 78-83の間が隙間（5タイル幅）

    // 最後の地面
    CreatePlatform(95, groundLevel, 25, groundHeight);
}

void GrassStage::CreatePlatforms() {
    // 浮遊プラットフォーム（拡張版）- プレイヤーが詰まらないよう配置

    // 序盤エリア - ジャンプの練習用
    CreatePlatform(5, 8, 4, 1);   // 小さなプラットフォーム
    CreatePlatform(12, 6, 3, 1);  // ジャンプ用プラットフォーム
    CreatePlatform(18, 4, 5, 1);  // 高いプラットフォーム

    // ギャップ1を渡るためのプラットフォーム - 配置を調整
    CreatePlatform(20, 9, 3, 1);  // 隙間手前の足場
    CreatePlatform(24, 7, 2, 1);  // 中間の高い足場

    // 中盤エリア - より高度なジャンプ
    CreatePlatform(32, 8, 6, 1);  // 大きなプラットフォーム
    CreatePlatform(45, 6, 3, 1);  // 高いプラットフォーム
    CreatePlatform(50, 3, 2, 1);  // 最高点のプラットフォーム

    // ギャップ2を渡るためのプラットフォーム - 配置を調整
    CreatePlatform(48, 9, 2, 1);  // 隙間上のプラットフォーム
    CreatePlatform(50, 7, 2, 1);  // より高いプラットフォーム

    // 中上盤エリア
    CreatePlatform(60, 8, 6, 1);  // 大きなプラットフォーム
    CreatePlatform(67, 5, 3, 1);  // 高いプラットフォーム
    CreatePlatform(73, 3, 2, 1);  // チャレンジプラットフォーム

    // ギャップ3を渡るためのプラットフォーム - 配置を調整
    CreatePlatform(79, 9, 2, 1);  // 隙間上のプラットフォーム
    CreatePlatform(81, 7, 2, 1);  // より高いプラットフォーム

    // 終盤エリア - ゴール前の最終チャレンジ
    CreatePlatform(88, 8, 4, 1);  // 大きなプラットフォーム
    CreatePlatform(95, 5, 3, 1);  // 高いプラットフォーム
    CreatePlatform(100, 3, 2, 1); // 最終チャレンジ
    CreatePlatform(108, 8, 4, 1); // ゴール前プラットフォーム
    CreatePlatform(115, 6, 3, 1); // ゴール直前の足場
}