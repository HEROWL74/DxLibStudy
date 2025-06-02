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

    // メイン地面
    CreatePlatform(0, groundLevel, 60, groundHeight);

    // 地面に穴を作る（ギャップジャンプ用）
    CreatePlatform(15, groundLevel, 8, groundHeight);  // 左の地面
    CreatePlatform(28, groundLevel, 8, groundHeight);  // 右の地面
    // 23-28の間が穴

    // 後半の地面
    CreatePlatform(40, groundLevel, 20, groundHeight);
}

void GrassStage::CreatePlatforms() {
    // 浮遊プラットフォーム
    CreatePlatform(5, 8, 4, 1);   // 小さなプラットフォーム
    CreatePlatform(12, 6, 3, 1);  // ジャンプ用プラットフォーム
    CreatePlatform(20, 4, 5, 1);  // 高いプラットフォーム

    // ギャップを渡るためのプラットフォーム
    CreatePlatform(24, 9, 2, 1);  // 穴の上のプラットフォーム
    CreatePlatform(26, 7, 2, 1);  // より高いプラットフォーム

    // 後半のプラットフォーム
    CreatePlatform(45, 8, 6, 1);  // 大きなプラットフォーム
    CreatePlatform(52, 5, 3, 1);  // 高いプラットフォーム
    CreatePlatform(57, 3, 2, 1);  // 最高点のプラットフォーム
}