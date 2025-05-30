#include "GameScreen.h"

GameScreen::GameScreen()
{
    currentState = GAME_STATE_START;
    effectManager = nullptr;
    uiManager = nullptr;
    soundManager = nullptr;
    backgroundHandle = -1;
    frameCounter = 0;
}

GameScreen::~GameScreen()
{
    Finalize();
}

void GameScreen::Initialize()
{
    // 各マネージャーの初期化
    effectManager = new EffectManager();
    uiManager = new UIManager();
    soundManager = new SoundManager();

    effectManager->Initialize();
    uiManager->Initialize();
    soundManager->Initialize();

    // 背景画像の読み込み
    backgroundHandle = LoadGraph("Sprites/Backgrounds/mountain_background.png");

    // 初期状態をスタート画面に設定
    ChangeState(GAME_STATE_START);
}

void GameScreen::Update()
{
    frameCounter++;

    switch (currentState)
    {
    case GAME_STATE_START:
        // スタート画面の更新
        uiManager->UpdateStartScreen();
        effectManager->Update();

        // スタートボタンが押されたらゲーム開始
        if (uiManager->IsStartButtonPressed())
        {
            ChangeState(GAME_STATE_PLAYING);
        }
        break;

    case GAME_STATE_PLAYING:
        // ゲーム中の処理
        // 今はスタート画面のデモなので簡易実装
        break;

    case GAME_STATE_PAUSE:
        // ポーズ画面の処理
        break;

    case GAME_STATE_GAMEOVER:
        // ゲームオーバー画面の処理
        break;
    }
}

void GameScreen::Draw()
{
    switch (currentState)
    {
    case GAME_STATE_START:
        // 背景描画
        if (backgroundHandle != -1)
        {
            DrawExtendGraph(0, 0, 1280, 720, backgroundHandle, TRUE);
        }

        // エフェクト描画
        effectManager->Draw();

        // UI描画
        uiManager->DrawStartScreen();
        break;

    case GAME_STATE_PLAYING:
        // ゲーム中の描画
        break;

    case GAME_STATE_PAUSE:
        // ポーズ画面の描画
        break;

    case GAME_STATE_GAMEOVER:
        // ゲームオーバー画面の描画
        break;
    }
}

void GameScreen::ChangeState(GameState newState)
{
    currentState = newState;

    // 状態変更時の処理
    switch (newState)
    {
    case GAME_STATE_START:
        effectManager->StartTitleEffects();
        soundManager->PlayBGM("title");
        break;

    case GAME_STATE_PLAYING:
        soundManager->PlayBGM("game");
        break;
    }
}

void GameScreen::Finalize()
{
    // リソースの解放
    if (backgroundHandle != -1)
    {
        DeleteGraph(backgroundHandle);
        backgroundHandle = -1;
    }

    // マネージャーの終了処理
    if (effectManager)
    {
        effectManager->Finalize();
        delete effectManager;
        effectManager = nullptr;
    }

    if (uiManager)
    {
        uiManager->Finalize();
        delete uiManager;
        uiManager = nullptr;
    }

    if (soundManager)
    {
        soundManager->Finalize();
        delete soundManager;
        soundManager = nullptr;
    }
}