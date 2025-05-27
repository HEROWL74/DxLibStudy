#pragma once
#include <map>
#include <string>
#include <vector>

struct Rect { int x, y, w, h; };

class Stage1Generate {
public:
    Stage1Generate();
    ~Stage1Generate();

    // マップの「床」と「はしご・旗」の描画
    void DrawMap() const;
    // 毎フレーム呼び出してスプリングアニメ管理
    void UpdateSprings();

    // 衝突判定用データ取得
    Rect                          GetStartPos()     const { return m_start; }
    const std::vector<Rect>& GetLadders()      const { return m_ladders; }
    Rect                          GetGoalRect()     const { return m_goal; }
    const std::vector<Rect>& GetPlatformRects() const { return m_platforms; }
    const std::vector<Rect>& GetCoinRects()    const { return m_coins; }
    const std::vector<Rect>& GetSpringRects()  const;
    const std::vector<Rect>& GetSpikeRects()   const;

    // 踏んだときに呼ぶ
    void ActivateSpring(int index);

private:
    static constexpr int TILE = 64;

    std::map<std::string, int>            m_th;          // タイルグラフィック
    std::vector<std::vector<std::string>> m_layout;     // 床だけ保持

    // 衝突判定用
    Rect                                 m_start;
    std::vector<Rect>                    m_ladders;
    Rect                                 m_goal;
    std::vector<Rect>                    m_platforms;
    std::vector<Rect>                    m_coins;
    std::vector<Rect>                    m_spikes;

    struct Spring { Rect r; bool active; int timer; };
    std::vector<Spring>                  m_springs;
    int                                  m_springDuration;

    void LoadTiles();
    void SetupMap();
};
