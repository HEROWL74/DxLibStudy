#pragma once
#include "Dxlib.h"

class TitleScene {
public:
    TitleScene();
    void Update();
    void Draw();
    bool IsStartRequested() const;
private:
    bool m_start;
};
