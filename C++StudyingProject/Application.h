#pragma once
#include <memory>
#include <DxLib.h>
class TitleScene;
class GameScene;

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
