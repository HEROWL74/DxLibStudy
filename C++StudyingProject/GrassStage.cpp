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

    // 拡張されたメイン地面（120タイル分）
    CreatePlatform(0, groundLevel, 120, groundHeight);

    // 地面に穴を作る（ギャップジャンプ用）
    // 穴1（初級）
    CreatePlatform(15, groundLevel, 8, groundHeight);  // 左の地面
    CreatePlatform(28, groundLevel, 8, groundHeight);  // 右の地面
    // 23-28の間が穴

    // 穴2（中級）
    CreatePlatform(45, groundLevel, 8, groundHeight);  // 左の地面
    CreatePlatform(58, groundLevel, 8, groundHeight);  // 右の地面
    // 53-58の間が穴

    // 穴3（上級）
    CreatePlatform(75, groundLevel, 8, groundHeight);  // 左の地面
    CreatePlatform(90, groundLevel, 8, groundHeight);  // 右の地面
    // 83-90の間が穴

    // 最後の地面
    CreatePlatform(98, groundLevel, 22, groundHeight);
}

void GrassStage::CreatePlatforms() {
    // 浮遊プラットフォーム（拡張版）

    // 序盤エリア
    CreatePlatform(5, 8, 4, 1);   // 小さなプラットフォーム
    CreatePlatform(12, 6, 3, 1);  // ジャンプ用プラットフォーム
    CreatePlatform(20, 4, 5, 1);  // 高いプラットフォーム

    // ギャップを渡るためのプラットフォーム（穴1）
    CreatePlatform(24, 9, 2, 1);  // 穴の上のプラットフォーム
    CreatePlatform(26, 7, 2, 1);  // より高いプラットフォーム

    // 中盤エリア
    CreatePlatform(35, 8, 6, 1);  // 大きなプラットフォーム
    CreatePlatform(42, 5, 3, 1);  // 高いプラットフォーム
    CreatePlatform(48, 3, 2, 1);  // 最高点のプラットフォーム

    // ギャップを渡るためのプラットフォーム（穴2）
    CreatePlatform(54, 9, 2, 1);  // 穴2の上のプラットフォーム
    CreatePlatform(56, 7, 2, 1);  // より高いプラットフォーム

    // 中後盤エリア
    CreatePlatform(65, 8, 6, 1);  // 大きなプラットフォーム
    CreatePlatform(72, 5, 3, 1);  // 高いプラットフォーム
    CreatePlatform(78, 3, 2, 1);  // チャレンジプラットフォーム

    // ギャップを渡るためのプラットフォーム（穴3）
    CreatePlatform(85, 9, 2, 1);  // 穴3の上のプラットフォーム
    CreatePlatform(87, 7, 2, 1);  // より高いプラットフォーム

    // 終盤エリア
    CreatePlatform(95, 8, 6, 1);  // 大きなプラットフォーム
    CreatePlatform(102, 5, 3, 1); // 高いプラットフォーム
    CreatePlatform(108, 3, 2, 1); // 最終チャレンジ
    CreatePlatform(115, 8, 4, 1); // ゴール前プラットフォーム
}