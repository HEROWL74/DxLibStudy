#include "DxLib.h"
#include "GameScreen.h"
#include "EffectManager.h"
#include "UIManager.h"
#include "SoundManager.h"

// メイン関数
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // ウィンドウモードに設定
    ChangeWindowMode(TRUE);

    // 画面サイズを設定
    SetGraphMode(1280, 720, 32);

    // DXライブラリ初期化処理
    if (DxLib_Init() == -1)
    {
        return -1;
    }

    // 描画先を裏画面にセット
    SetDrawScreen(DX_SCREEN_BACK);

    // ゲーム画面の初期化
    GameScreen gameScreen;
    gameScreen.Initialize();

    // メインループ
    while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
    {
        // 画面をクリア
        ClearDrawScreen();

        // ゲーム画面の更新と描画
        gameScreen.Update();
        gameScreen.Draw();

        // 裏画面の内容を表画面に反映
        ScreenFlip();
    }

    // 終了処理
    gameScreen.Finalize();

    // DXライブラリ使用の終了処理
    DxLib_End();

    return 0;
}