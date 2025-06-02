#include "SandStage.h"

SandStage::SandStage() {
    terrainType = "sand";
}

SandStage::~SandStage() {
    // 基底クラスのデストラクタが呼ばれる
}

void SandStage::Initialize() {
    // テクスチャ読み込み
    LoadTerrainTextures();

    // 地形生成
    CreateMainTerrain();
    CreatePlatforms();
}

void SandStage::CreateMainTerrain() {
    // 砂漠の起伏のある地形
    int groundHeight = 3;

    // 起伏のある地面
    CreatePlatform(0, 13, 12, groundHeight);   // 低い地面
    CreatePlatform(12, 11, 8, groundHeight + 2); // 少し高い地面
    CreatePlatform(20, 14, 10, groundHeight);  // また低い地面
    CreatePlatform(30, 10, 10, groundHeight + 3); // 高い丘
    CreatePlatform(40, 12, 20, groundHeight + 1); // 最後の地面
}

void SandStage::CreatePlatforms() {
    // 砂漠のオアシス周りのプラットフォーム
    CreatePlatform(5, 9, 3, 1);   // 小さな島
    CreatePlatform(15, 7, 2, 1);  // ジャンプ台
    CreatePlatform(25, 8, 4, 1);  // 中くらいの島

    // ピラミッド風の構造
    CreatePlatform(35, 8, 1, 1);  // 頂上
    CreatePlatform(34, 9, 3, 1);  // 2段目
    CreatePlatform(33, 10, 5, 1); // 1段目（実際は3段目）

    // 砂丘を表現するプラットフォーム
    CreatePlatform(48, 8, 2, 1);
    CreatePlatform(52, 6, 3, 1);
    CreatePlatform(56, 4, 2, 1);
}