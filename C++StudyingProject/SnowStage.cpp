#include "SnowStage.h"

SnowStage::SnowStage() {
    terrainType = "snow";
}

SnowStage::~SnowStage() {
    // 基底クラスのデストラクタが呼ばれる
}

void SnowStage::Initialize() {
    // テクスチャ読み込み
    LoadTerrainTextures();

    // 地形生成
    CreateMainTerrain();
    CreatePlatforms();
}

void SnowStage::CreateMainTerrain() {
    // 雪山の険しい地形
    int groundHeight = 4;

    // 山の斜面を表現
    CreatePlatform(0, 14, 8, groundHeight);    // 麓
    CreatePlatform(8, 12, 6, groundHeight);    // 少し上
    CreatePlatform(14, 10, 6, groundHeight);   // 中腹
    CreatePlatform(20, 8, 8, groundHeight);    // 山頂付近
    CreatePlatform(28, 10, 6, groundHeight);   // 下り坂
    CreatePlatform(34, 12, 8, groundHeight);   // また上り
    CreatePlatform(42, 9, 18, groundHeight);   // 高原
}

void SnowStage::CreatePlatforms() {
    // 氷の足場
    CreatePlatform(6, 10, 1, 1);   // 小さな氷塊
    CreatePlatform(12, 7, 2, 1);   // 氷の足場
    CreatePlatform(18, 5, 1, 1);   // 高い氷塊

    // 雪だるまの台座のような円形に近い形
    CreatePlatform(25, 6, 1, 1);   // 上
    CreatePlatform(24, 7, 3, 1);   // 中
    CreatePlatform(25, 8, 1, 1);   // 下

    // つらら風の垂直構造
    CreatePlatform(40, 5, 1, 1);
    CreatePlatform(45, 4, 1, 2);
    CreatePlatform(50, 3, 1, 3);
    CreatePlatform(55, 2, 1, 4);
}