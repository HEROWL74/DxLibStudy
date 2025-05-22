#pragma once
#include <memory>
#include <DxLib.h>
class TitleScene;
class GameScene;


const int WINDOW_WID = 1980;
const int WINDOW_HIG = 1080;

class Application {
public:
    Application();
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    void Run();

private:
    bool Initialize();
    void Update();
    void Release();

    enum class SceneType { Title, Game };
    SceneType currentScene;

    std::unique_ptr<TitleScene> title;
    std::unique_ptr<GameScene> game;
    
};
