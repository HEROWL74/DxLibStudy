#pragma once
#include <map>
#include <string>
#include <vector>

class TitleGenerate {
public:
    TitleGenerate();
    ~TitleGenerate();
    void DrawMap();
private:
    std::map<std::string, int> tileHandles;
    std::vector<std::vector<std::string>> mapLayout;

    void LoadTiles();
    void SetupMap();
};
