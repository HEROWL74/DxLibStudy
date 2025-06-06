#pragma once
#include "DxLib.h"
#include <vector>
#include <memory>
#include <string>

// タイル情報構造体
struct Tile {
    int textureHandle;
    int x, y;           // ワールド座標
    bool isSolid;       // 当たり判定があるか

    Tile(int handle, int posX, int posY, bool solid = true)
        : textureHandle(handle), x(posX), y(posY), isSolid(solid) {}
};

// ステージ基底クラス
class Stage {
public:
    static const int TILE_SIZE = 64;        // タイルサイズ
    static const int STAGE_WIDTH = 7680;    // ステージ幅（60タイル → 120タイルに拡張）
    static const int STAGE_HEIGHT = 1080;   // ステージ高さ
    static const int GROUND_LEVEL = 800;    // 地面レベル

    Stage();
    virtual ~Stage();

    virtual void Initialize() = 0;
    virtual void Update(float cameraX);
    virtual void Draw(float cameraX);

    // 当たり判定
    bool CheckCollision(float playerX, float playerY, float playerWidth, float playerHeight);
    float GetGroundY(float playerX, float playerWidth);

protected:
    std::vector<std::unique_ptr<Tile>> tiles;
    std::string terrainType;

    // タイル配置用ヘルパー
    void PlaceTile(const std::string& tileType, int gridX, int gridY);
    void PlacePlatformBlock(int gridX, int gridY); // **新追加：単一ブロック配置**
    void CreatePlatform(int startX, int startY, int width, int height);
    void LoadTerrainTextures();

    // **新追加：プラットフォーム専用テクスチャ管理**
    void LoadPlatformTexture();
    void CreatePlatformBlock(int startX, int startY, int width, int height);

    // テクスチャハンドル管理
    struct TerrainTextures {
        int bottom, bottom_left, bottom_right;
        int center, left, right;
        int top, top_left, top_right;
    } textures;

    // **新追加：単一プラットフォームテクスチャ**
    int platformBlockTexture;

private:
    void LoadTileTexture(int& handle, const std::string& filePath);
};