#include "StoneStage.h"

StoneStage::StoneStage() {
    terrainType = "stone";
}

StoneStage::~StoneStage() {
    // 基底クラスのデストラクタが呼ばれる
}

void StoneStage::Initialize() {
    // テクスチャ読み込み
    LoadTerrainTextures();

    // 地形生成
    CreateMainTerrain();
    CreatePlatforms();
}

void StoneStage::CreateMainTerrain() {
    // 城のような石造りステージ
    int groundLevel = 13;
    int groundHeight = 3;

    // 基本地面
    CreatePlatform(0, groundLevel, 15, groundHeight);
    CreatePlatform(20, groundLevel, 15, groundHeight);
    CreatePlatform(40, groundLevel, 20, groundHeight);

    // 階段状の地形
    CreatePlatform(35, 12, 5, 1);
    CreatePlatform(36, 11, 3, 1);
    CreatePlatform(37, 10, 1, 1);
}

void StoneStage::CreatePlatforms() {
    // 城の塔のような構造
    CreatePlatform(8, 8, 2, 5);   // 左の塔
    CreatePlatform(25, 6, 2, 7);  // 中央の高い塔
    CreatePlatform(45, 9, 2, 4);  // 右の塔

    // 塔を繋ぐプラットフォーム
    CreatePlatform(10, 10, 6, 1); // 左の塔から延びる
    CreatePlatform(27, 8, 8, 1);  // 中央の塔から延びる
    CreatePlatform(47, 11, 6, 1); // 右の塔から延びる

    // 跳び箱のようなブロック
    CreatePlatform(18, 11, 1, 2);
    CreatePlatform(38, 10, 1, 3);
    CreatePlatform(55, 9, 1, 4);
}