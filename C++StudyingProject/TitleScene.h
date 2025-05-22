#pragma once

class TitleScene {
public:
    TitleScene();
    void Update();
    void Draw();
    bool IsStartRequested() const;
    bool IsFadeOutFinished() const;
private:
    bool m_start;
    bool m_fadeOut;
    int m_fadeAlpha;
};
