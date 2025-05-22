#include "DxLib.h"

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{
	SetMainWindowText("C++StudyingProject");
	SetGraphMode(1280, 720, 32);
	ChangeWindowMode(TRUE);
	if (DxLib_Init() == -1) {
		return -1;
	}
	SetDrawScreen(DX_SCREEN_BACK);

	while (ProcessMessage() == 0 && ClearDrawScreen() == 0) {
		// Main loop
		WaitTimer(16); // Wait for 16ms (60 FPS)
	}

	DxLib_End();
	return 0;
}