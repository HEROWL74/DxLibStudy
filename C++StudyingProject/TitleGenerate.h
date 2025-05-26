#pragma once
#include <map>
#include <string>
#include <vector>

class TitleGenerate {
public:
    TitleGenerate();
    ~TitleGenerate();
    void DrawMap();

    int GetMapHeight() const { return static_cast<int>(mapLayout.size()); }

private:
    std::map<std::string, int> tileHandles;//文字列とIDを結びつける変数。
    std::vector<std::vector<std::string>> mapLayout;//文字列で描画の種類を分けるため、stringとvectorを使用

    void LoadTiles();
    void SetupMap();
};
