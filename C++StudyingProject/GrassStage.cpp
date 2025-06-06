#include "GrassStage.h"

GrassStage::GrassStage() {
    terrainType = "grass";
}

GrassStage::~GrassStage() {
    // 基底クラスのデストラクタが呼ばれる
}

void GrassStage::Initialize() {
    // メインテクスチャ読み込み（地面用）
    LoadTerrainTextures();

    // **プラットフォーム専用テクスチャ読み込み（terrain_grass_block.png）**
    LoadPlatformTexture();

    // 地形生成
    CreateMainTerrain();
    CreatePlatforms();
}

void GrassStage::CreateMainTerrain() {
    // 基本地面の生成（全体にわたる地面）- メインテクスチャ使用
    int groundLevel = 12; // グリッド座標
    int groundHeight = 4; // 地面の厚さ

    // 拡張したメインの部分のプラットフォーム（120タイル分使用）
    CreatePlatform(0, groundLevel, 120, groundHeight);

    // 地面に隙間を作る（ギャップジャンプ用）- 隙間を狭めて難易度を防止
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
    CreatePlatform(83, groundLevel, 8, groundHeight);  // 右の地面
    // 78-83の間が隙間（5タイル幅）

    // 最後の地面
    CreatePlatform(95, groundLevel, 25, groundHeight);
}

void GrassStage::CreatePlatforms() {
    // **浮遊プラットフォーム（terrain_grass_block.pngテクスチャ使用）**
    // プレイヤーが詰まらないよう配置

    // 序盤エリア - ジャンプの練習用（grass blockテクスチャ）
    CreatePlatformBlock(5, 8, 4, 1);   // 小さなプラットフォーム
    CreatePlatformBlock(12, 6, 3, 1);  // ジャンプ用プラットフォーム
    CreatePlatformBlock(18, 4, 5, 1);  // 高いプラットフォーム

    // ギャップ1を渡るためのプラットフォーム - 配置を調整
    CreatePlatformBlock(20, 9, 3, 1);  // 隙間直前の足場
    CreatePlatformBlock(24, 7, 2, 1);  // 中間の高い足場

    // 中盤エリア - より高度なジャンプ
    CreatePlatformBlock(32, 8, 6, 1);  // 大きなプラットフォーム
    CreatePlatformBlock(45, 6, 3, 1);  // 高いプラットフォーム
    CreatePlatformBlock(50, 3, 2, 1);  // 最高点のプラットフォーム

    // ギャップ2を渡るためのプラットフォーム - 配置を調整
    CreatePlatformBlock(48, 9, 2, 1);  // 隙間上のプラットフォーム
    CreatePlatformBlock(50, 7, 2, 1);  // より高いプラットフォーム

    // 中上盤エリア
    CreatePlatformBlock(60, 8, 6, 1);  // 大きなプラットフォーム
    CreatePlatformBlock(67, 5, 3, 1);  // 高いプラットフォーム
    CreatePlatformBlock(73, 3, 2, 1);  // チャレンジプラットフォーム

    // ギャップ3を渡るためのプラットフォーム - 配置を調整
    CreatePlatformBlock(79, 9, 2, 1);  // 隙間上のプラットフォーム
    CreatePlatformBlock(81, 7, 2, 1);  // より高いプラットフォーム

    // 終盤エリア - ゴール前の最終チャレンジ
    CreatePlatformBlock(88, 8, 4, 1);  // 大きなプラットフォーム
    CreatePlatformBlock(95, 5, 3, 1);  // 高いプラットフォーム
    CreatePlatformBlock(100, 3, 2, 1); // 最終チャレンジ
    CreatePlatformBlock(108, 8, 4, 1); // ゴール前プラットフォーム
    CreatePlatformBlock(115, 6, 3, 1); // ゴール直前の足場
}