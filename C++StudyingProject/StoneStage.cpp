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
    // 城のような石造りステージ - より登りやすく簡単に
    int groundLevel = 13;
    int groundHeight = 3;

    // 基本地面 - 間隔を狭めて安全に
    CreatePlatform(0, groundLevel, 20, groundHeight);    // スタート地点（拡張）
    CreatePlatform(22, groundLevel, 18, groundHeight);   // 中間地点1（間隔を狭く）
    CreatePlatform(42, groundLevel, 18, groundHeight);   // 中間地点2
    CreatePlatform(62, groundLevel, 18, groundHeight);   // 中間地点3
    CreatePlatform(82, groundLevel, 20, groundHeight);   // 終盤部
    CreatePlatform(104, groundLevel, 16, groundHeight);  // ゴール前

    // 緩やかな階段状の地形 - より登りやすく、段差を小さく
    CreatePlatform(35, 12, 8, 1);   // 1段目（幅を広く）
    CreatePlatform(37, 11, 6, 1);   // 2段目
    CreatePlatform(39, 10, 4, 1);   // 3段目
    CreatePlatform(40, 9, 3, 1);    // 4段目（頂上）
    CreatePlatform(41, 10, 2, 1);   // 下り1段目
    CreatePlatform(43, 11, 4, 1);   // 下り2段目
}

void StoneStage::CreatePlatforms() {
    // 城の塔のような構造 - 高さを抑えて登りやすく
    CreatePlatform(8, 10, 2, 4);   // 左の塔（低く調整）
    CreatePlatform(28, 9, 2, 5);   // 中央の塔（高さを抑制）
    CreatePlatform(68, 10, 2, 4);  // 右の塔（低く調整）

    // 塔を繋ぐプラットフォーム - より多くの足場で安全に
    CreatePlatform(11, 11, 10, 1); // 左の塔から延びる（長く）
    CreatePlatform(23, 10, 4, 1);  // 中間足場1
    CreatePlatform(31, 10, 8, 1);  // 中央塔への橋（長く）
    CreatePlatform(41, 11, 6, 1);  // 中間足場2
    CreatePlatform(48, 10, 12, 1); // 長い橋
    CreatePlatform(62, 11, 5, 1);  // 中間足場3
    CreatePlatform(71, 11, 8, 1);  // 右塔への橋

    // 追加の安全な足場 - ジャンプ失敗時の救済措置
    CreatePlatform(15, 8, 4, 1);   // 序盤の高い足場
    CreatePlatform(25, 7, 3, 1);   // 中間の足場
    CreatePlatform(45, 8, 4, 1);   // 中央の高い足場
    CreatePlatform(55, 7, 3, 1);   // 右側の足場
    CreatePlatform(75, 8, 4, 1);   // 終盤の足場
    CreatePlatform(85, 9, 3, 1);   // ゴール前の足場

    // 段階的な上昇用の足場 - より細かいステップ
    CreatePlatform(12, 9, 2, 1);   // ステップ1
    CreatePlatform(19, 8, 2, 1);   // ステップ2
    CreatePlatform(33, 8, 2, 1);   // ステップ3
    CreatePlatform(52, 8, 2, 1);   // ステップ4
    CreatePlatform(65, 9, 2, 1);   // ステップ5
    CreatePlatform(78, 8, 2, 1);   // ステップ6

    // 低い装飾的な石ブロック - 詰まりの原因となる高い柱を除去
    CreatePlatform(18, 12, 1, 1);  // 装飾用（低く）
    CreatePlatform(38, 12, 1, 1);  // 装飾用（低く）
    CreatePlatform(58, 12, 1, 1);  // 装飾用（低く）
    CreatePlatform(88, 12, 1, 1);  // 装飾用（低く）

    // 緊急避難用の低いプラットフォーム
    CreatePlatform(5, 12, 3, 1);   // スタート近くの避難場所
    CreatePlatform(35, 13, 4, 1);  // 中間の避難場所
    CreatePlatform(65, 13, 4, 1);  // 終盤の避難場所
    CreatePlatform(95, 12, 3, 1);  // ゴール前の避難場所
}