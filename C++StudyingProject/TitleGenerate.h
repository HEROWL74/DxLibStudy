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
    std::map<std::string, int> tileHandles;//�������ID�����т���ϐ��B
    std::vector<std::vector<std::string>> mapLayout;//������ŕ`��̎�ނ𕪂��邽�߁Astring��vector���g�p

    void LoadTiles();
    void SetupMap();
};
