#pragma once
#include "DxLib.h"
#include <vector>

struct Rect {
    int x, y, w, h;
    Rect(int _x, int _y, int _w, int _h) : x(_x), y(_y), w(_w), h(_h) {}
};

struct Point {
    int x, y;
    Point(int _x, int _y) : x(_x), y(_y) {}
};

struct Spring {
    int x, y;
    int animFrame;
    bool isPressed;
    Spring(int _x, int _y) : x(_x), y(_y), animFrame(0), isPressed(false) {}
};

class Stage1Generate {
public:
    Stage1Generate();
    ~Stage1Generate();

    void DrawMap();
    void UpdateSprings();
    void SetCameraX(int cameraX) { m_cameraX = cameraX; }
    
    // ゲッター
    const std::vector<Rect>& GetFloorRects() const { return m_floorRects; }
    const std::vector<Rect>& GetPlatformRects() const { return m_platformRects; }
    const std::vector<Rect>& GetCoinRects() const { return m_coinRects; }
    const std::vector<Rect>& GetSpikeRects() const { return m_spikeRects; }
    const std::vector<Rect>& GetLadders() const { return m_ladders; }
    const std::vector<Spring>& GetSprings() const { return m_springs; }
    Point GetStartPos() const { return m_startPos; }
    Point GetGoalPos() const { return m_goalPos; }
    
    // ステージの全体幅を取得
    int GetStageWidth() const { return m_stageWidth; }

private:
    void LoadGraphics();
    void GenerateStage();
    void DrawTiles();
    void DrawObjects();

    // カメラ位置
    int m_cameraX;
    
    // ステージデータ
    std::vector<Rect> m_floorRects;
    std::vector<Rect> m_platformRects;
    std::vector<Rect> m_coinRects;
    std::vector<Rect> m_spikeRects;
    std::vector<Rect> m_ladders;
    std::vector<Spring> m_springs;
    
    Point m_startPos;
    Point m_goalPos;
    
    // ステージサイズ
    static const int m_stageWidth = 6400;  // 6400ピクセル幅の長いステージ
    static const int m_stageHeight = 1080;
    
    // タイルサイズ
    static const int TILE_SIZE = 64;
    
    // グラフィックハンドル
    int m_grassImg;
    int m_stoneImg;
    int m_spikeImg;
    int m_ladderImg;
    int m_springImg;
    int m_flagImg;
    int m_bgImg;
};