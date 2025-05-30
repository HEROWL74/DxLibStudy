#include "DxLib.h"
#include "GameScreen.h"
#include "EffectManager.h"
#include "UIManager.h"
#include "SoundManager.h"

// ���C���֐�
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // �E�B���h�E���[�h�ɐݒ�
    ChangeWindowMode(TRUE);

    // ��ʃT�C�Y��ݒ�
    SetGraphMode(1280, 720, 32);

    // DX���C�u��������������
    if (DxLib_Init() == -1)
    {
        return -1;
    }

    // �`���𗠉�ʂɃZ�b�g
    SetDrawScreen(DX_SCREEN_BACK);

    // �Q�[����ʂ̏�����
    GameScreen gameScreen;
    gameScreen.Initialize();

    // ���C�����[�v
    while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
    {
        // ��ʂ��N���A
        ClearDrawScreen();

        // �Q�[����ʂ̍X�V�ƕ`��
        gameScreen.Update();
        gameScreen.Draw();

        // ����ʂ̓��e��\��ʂɔ��f
        ScreenFlip();
    }

    // �I������
    gameScreen.Finalize();

    // DX���C�u�����g�p�̏I������
    DxLib_End();

    return 0;
}