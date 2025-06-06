#include "SnowStage.h"

SnowStage::SnowStage() {
    terrainType = "snow";
}

SnowStage::~SnowStage() {
    // 基底クラスのデストラクタが呼ばれる
}

void SnowStage::Initialize() {
    // メインテクスチャ読み込み（地面用）
    LoadTerrainTextures();

    // **プラットフォーム専用テクスチャ読み込み（terrain_snow_block.png）**
    LoadPlatformTexture();

    // 地形生成
    CreateMainTerrain();
    CreatePlatforms();
}

void SnowStage::CreateMainTerrain() {
    // 雪山の険しい地形 - より登りやすく調整
    int groundHeight = 4;

    // 山の斜面を表現 - 段差を小さく
    CreatePlatform(0, 14, 10, groundHeight);    // 麓
    CreatePlatform(10, 13, 8, groundHeight);    // 少し上
    CreatePlatform(18, 12, 8, groundHeight);    // 中腹
    CreatePlatform(26, 11, 8, groundHeight);    // 山頂付近
    CreatePlatform(34, 10, 8, groundHeight);    // 頂上
    CreatePlatform(42, 11, 8, groundHeight);    // 下り坂
    CreatePlatform(50, 12, 10, groundHeight);   // 中腹
    CreatePlatform(60, 13, 12, groundHeight);   // 麓に戻る
    CreatePlatform(72, 12, 15, groundHeight);   // 平らな部分
    CreatePlatform(87, 10, 20, groundHeight);   // 高原
    CreatePlatform(107, 12, 13, groundHeight);  // ゴール前
}

void SnowStage::CreatePlatforms() {
    // **雪の足場 - terrain_snow_block.pngテクスチャ使用**
    // ジャンプしやすい配置
    CreatePlatformBlock(6, 11, 2, 1);   // 小さな雪塊
    CreatePlatformBlock(14, 9, 3, 1);   // 雪の足場
    CreatePlatformBlock(22, 8, 2, 1);   // 高い雪塊
    CreatePlatformBlock(30, 7, 3, 1);   // 山頂近くの足場

    // 雪だるまの台座のような円形に近い形
    CreatePlatformBlock(38, 8, 1, 1);   // 上
    CreatePlatformBlock(37, 9, 3, 1);   // 中
    CreatePlatformBlock(38, 10, 1, 1);  // 下

    // 氷柱風の突き出し構造 - 登りやすく調整
    CreatePlatformBlock(48, 9, 1, 1);   // 最上段
    CreatePlatformBlock(46, 10, 1, 1);  // 2段目
    CreatePlatformBlock(50, 10, 1, 1);  // 2段目（右）
    CreatePlatformBlock(48, 11, 1, 1);  // 3段目

    // 雪原の浮島
    CreatePlatformBlock(58, 9, 4, 1);   // 大きな浮島
    CreatePlatformBlock(65, 7, 3, 1);   // 高い浮島
    CreatePlatformBlock(75, 8, 4, 1);   // 中間の浮島
    CreatePlatformBlock(83, 6, 3, 1);   // さらに高い浮島

    // 雪山の突起
    CreatePlatformBlock(92, 7, 2, 1);   // 突起1
    CreatePlatformBlock(98, 5, 2, 1);   // 突起2
    CreatePlatformBlock(104, 8, 3, 1);  // 突起3

    // つらら風の細い足場
    CreatePlatformBlock(16, 6, 1, 1);
    CreatePlatformBlock(35, 5, 1, 1);
    CreatePlatformBlock(55, 6, 1, 1);
    CreatePlatformBlock(78, 4, 1, 1);
    CreatePlatformBlock(101, 9, 1, 1);
}