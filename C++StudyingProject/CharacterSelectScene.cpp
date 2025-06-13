#include "CharacterSelectScene.h"
#include "SoundManager.h"
using namespace std;

CharacterSelectScene::CharacterSelectScene()
    : characterSelected(false)
    , backRequested(false)
    , tutorialRequested(false)      // **新追加**
    , selectConfirmed(false)
    , selectedCharacterIndex(-1)
    , hoveredCharacterIndex(-1)
    , selectionState(SELECTING)
    , rotationAngle(0.0f)
    , titlePulsePhase(0.0f)
    , mousePressed(false)
    , mousePressedPrev(false)
    , backspacePressed(false)
    , backspacePressedPrev(false)
    ,tutorialEnabled(true) //デフォルトで有効にしておく
{
}

CharacterSelectScene::~CharacterSelectScene()
{
    DeleteGraph(backgroundHandle);
    DeleteGraph(buttonBackgroundHandle);
    DeleteGraph(uiButtonHandle);
    DeleteFontToHandle(fontHandle);
    DeleteFontToHandle(largeFontHandle);

    // キャラクターアイコンの削除
    for (auto& character : characters) {
        DeleteGraph(character.iconHandle);
    }
}

void CharacterSelectScene::Initialize()
{
    // テクスチャ読み込み
    backgroundHandle = LoadGraph("Sprites/Backgrounds/background_fade_trees.png");
    buttonBackgroundHandle = LoadGraph("UI/PNG/Yellow/button_square_gradient.png");
    uiButtonHandle = LoadGraph("UI/PNG/Yellow/button_rectangle_depth_gradient.png");
    fontHandle = CreateFontToHandle(NULL, 28, 3);
    largeFontHandle = CreateFontToHandle(NULL, 48, 5);

    // キャラクター情報設定
    vector<string> characterNames = {
        "Beige Knight", "Green Ranger", "Pink Warrior", "Purple Mage", "Yellow Hero"
    };

    vector<string> iconPaths = {
        "Sprites/Tiles/hud_player_helmet_beige.png",
        "Sprites/Tiles/hud_player_helmet_green.png",
        "Sprites/Tiles/hud_player_helmet_pink.png",
        "Sprites/Tiles/hud_player_helmet_purple.png",
        "Sprites/Tiles/hud_player_helmet_yellow.png"
    };

    // キャラクター配置（円形配置）
    int centerX = SCREEN_W / 2 - 100; // 100ピクセル左に移動
    int centerY = SCREEN_H / 2 - 50; // 少し上に配置
    int radius = 280;

    characters.clear();
    for (int i = 0; i < CHARACTER_COUNT; i++) {
        Character character;
        character.name = characterNames[i];
        character.iconPath = iconPaths[i];
        character.iconHandle = LoadGraph(iconPaths[i].c_str());

        // 円形配置の計算
        float angle = (float)i * (2.0f * 3.14159265359f / CHARACTER_COUNT) - 3.14159265359f / 2;
        character.baseX = centerX + (int)(cos(angle) * radius) - 50;
        character.baseY = centerY + (int)(sin(angle) * radius) - 50;
        character.currentX = character.baseX;
        character.currentY = character.baseY;
        character.targetX = character.baseX;
        character.targetY = character.baseY;

        character.scale = NORMAL_SCALE;
        character.hoverProgress = 0.0f;
        character.popupProgress = 0.0f;
        character.glowIntensity = 0.0f;
        character.floatPhase = (float)i * 0.5f;
        character.hovered = false;
        character.selected = false;

        characters.push_back(character);
    }

    // UIボタン設定の修正
    uiButtons.clear();
    int buttonY = SCREEN_H - 150;
    int buttonCenterX = SCREEN_W / 2 - 100;

    // **TUTORIALボタン（条件付きで追加）**
    if (tutorialEnabled) {
        UIButton tutorialButton;
        tutorialButton.x = buttonCenterX - 300;  // 左端に配置
        tutorialButton.y = buttonY;
        tutorialButton.w = 180;
        tutorialButton.h = 60;
        tutorialButton.label = "TUTORIAL";
        tutorialButton.scale = NORMAL_SCALE;
        tutorialButton.glowIntensity = 0.0f;
        tutorialButton.hovered = false;
        tutorialButton.enabled = true;
        uiButtons.push_back(tutorialButton);
    }

    // **SELECTボタン（チュートリアル有無に関係なく中央に配置）**
    UIButton selectButton;
    selectButton.x = buttonCenterX - 90;  // 常に中央
    selectButton.y = buttonY;
    selectButton.w = 180;
    selectButton.h = 60;
    selectButton.label = "SELECT";
    selectButton.scale = NORMAL_SCALE;
    selectButton.glowIntensity = 0.0f;
    selectButton.hovered = false;
    selectButton.enabled = false;
    uiButtons.push_back(selectButton);

    // **BACKボタン（右側に配置）**
    UIButton backButton;
    backButton.x = buttonCenterX + 120;  // 右側
    backButton.y = buttonY;
    backButton.w = 180;
    backButton.h = 60;
    backButton.label = "BACK";
    backButton.scale = NORMAL_SCALE;
    backButton.glowIntensity = 0.0f;
    backButton.hovered = false;
    backButton.enabled = true;
    uiButtons.push_back(backButton);
}

void CharacterSelectScene::Update()
{
    // マウス入力更新
    GetMousePoint(&mouseX, &mouseY);
    mousePressedPrev = mousePressed;
    mousePressed = (GetMouseInput() & MOUSE_INPUT_LEFT) != 0;

    // キーボード入力更新
    backspacePressedPrev = backspacePressed;
    backspacePressed = CheckHitKey(KEY_INPUT_BACK) != 0;

    // タイトル脈動効果
    titlePulsePhase += 0.02f;

    // 回転アニメーション（選択中のみ）
    if (selectionState == SELECTING) {
        rotationAngle += ROTATION_SPEED;
        if (rotationAngle >= 2.0f * 3.14159265359f) {
            rotationAngle -= 2.0f * 3.14159265359f;
        }
    }

    // Backspaceキーで戻る
    if (backspacePressed && !backspacePressedPrev) {
        if (selectionState == SELECTED) {
            // 選択状態から回転状態に戻る
            selectionState = SELECTING;
            selectedCharacterIndex = -1;
            characterSelected = false;

            // 全キャラクターの選択状態をクリア
            for (auto& c : characters) {
                c.selected = false;
            }

            // SELECTボタンを無効化
            if (uiButtons.size() > 1) {
                uiButtons[1].enabled = false;
            }
        }
        else {
            // 選択中状態からタイトル画面に戻る
            backRequested = true;
        }
    }

    // キャラクター更新
    UpdateCharacters();

    // キャラクター位置更新
    UpdateCharacterPositions();

    // UIボタン更新
    UpdateUIButtons();
}

void CharacterSelectScene::Draw()
{
    // 背景描画
    DrawExtendGraph(0, 0, SCREEN_W, SCREEN_H, backgroundHandle, TRUE);

    // タイトル描画（脈動効果付き）
    float titleScale = 1.0f + sinf(titlePulsePhase) * 0.03f;
    int titleAlpha = (int)(255 * (0.9f + sinf(titlePulsePhase * 2) * 0.1f));
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, titleAlpha);

    string titleText = "SELECT CHARACTER";
    int titleWidth = GetDrawStringWidthToHandle(titleText.c_str(), (int)titleText.length(), largeFontHandle);
    int titleX = SCREEN_W / 2 - (int)(titleWidth * titleScale) / 2;
    int titleY = 150;

    // タイトルのグロー効果
    DrawGlowEffect(titleX - 20, titleY - 10, (int)(titleWidth * titleScale) + 40, 60, 0.2f, GetColor(255, 215, 0));
    DrawStringToHandle(titleX, titleY, titleText.c_str(), GetColor(255, 255, 255), largeFontHandle);

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    // キャラクター描画
    for (int i = 0; i < (int)characters.size(); i++) {
        DrawCharacter(characters[i], i);
    }

    // UIボタン描画
    for (const auto& button : uiButtons) {
        DrawUIButton(button);
    }

    // 操作説明
    if (selectionState == SELECTING) {
        DrawStringToHandle(50, SCREEN_H - 120, "Click on a character to select, or click TUTORIAL for quick practice", GetColor(200, 200, 200), fontHandle);
        DrawStringToHandle(50, SCREEN_H - 80, "Press [Backspace] or BACK button to return to title", GetColor(150, 150, 150), fontHandle);
    }
    else if (selectionState == SELECTED) {
        DrawStringToHandle(50, SCREEN_H - 120, "Click SELECT to start game, TUTORIAL to practice, or BACK to reselect", GetColor(200, 200, 200), fontHandle);
        DrawStringToHandle(50, SCREEN_H - 80, "Press [Backspace] to return to character selection", GetColor(150, 150, 150), fontHandle);
    }

    // 選択されたキャラクターの名前表示
    if (hoveredCharacterIndex >= 0 && hoveredCharacterIndex < (int)characters.size()) {
        const Character& hoveredChar = characters[hoveredCharacterIndex];
        string nameText = hoveredChar.name;
        int nameWidth = GetDrawStringWidthToHandle(nameText.c_str(), (int)nameText.length(), fontHandle);
        int nameX = SCREEN_W / 2 - nameWidth / 2;
        int nameY = SCREEN_H - 200;

        // 名前の背景
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
        DrawBox(nameX - 20, nameY - 10, nameX + nameWidth + 20, nameY + 40, GetColor(0, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

        DrawStringToHandle(nameX, nameY, nameText.c_str(), GetColor(255, 215, 0), fontHandle);
    }
}

// CharacterSelectScene.cpp の UpdateCharacters関数を修正

void CharacterSelectScene::UpdateCharacters()
{
    hoveredCharacterIndex = -1;

    for (int i = 0; i < (int)characters.size(); i++) {
        Character& character = characters[i];

        // アイコンサイズは100x100と仮定
        int iconSize = 100;
        bool over = IsMouseOver(character.currentX, character.currentY, iconSize, iconSize);
        character.hovered = over;

        if (over) {
            hoveredCharacterIndex = i;
        }

        // スケールアニメーション
        float targetScale = over ? HOVER_SCALE : NORMAL_SCALE;
        if (character.selected && selectionState == SELECTED) {
            targetScale = 1.3f; // 選択されたキャラクターはさらに大きく
        }
        character.scale = Lerp(character.scale, targetScale, SCALE_LERP);

        // ホバープログレス
        float targetHover = over ? 1.0f : 0.0f;
        character.hoverProgress = Lerp(character.hoverProgress, targetHover, POPUP_SPEED);

        // ポップアップ効果
        character.popupProgress = Lerp(character.popupProgress, targetHover, POPUP_SPEED);

        // グロー効果
        float targetGlow = over ? 1.0f : 0.0f;
        if (character.selected) {
            targetGlow = 1.0f; // 選択されたキャラクターは常にグロー
        }
        character.glowIntensity = Lerp(character.glowIntensity, targetGlow, GLOW_SPEED);

        // 浮遊効果
        character.floatPhase += FLOAT_SPEED;

        // クリック処理
        if (over && mousePressed && !mousePressedPrev && selectionState != CONFIRMED) {
            // 以前の選択をクリア
            for (auto& c : characters) {
                c.selected = false;
            }

            selectedCharacterIndex = i;
            character.selected = true;
            characterSelected = true;
            selectionState = SELECTED;

            // **修正: SELECTボタンを動的に検索して有効化**
            for (auto& button : uiButtons) {
                if (button.label == "SELECT") {
                    button.enabled = true;
                    OutputDebugStringA("CharacterSelectScene: SELECT button enabled dynamically\n");
                    break;
                }
            }

            char debugMsg[256];
            sprintf_s(debugMsg, "CharacterSelectScene: Character %d selected, SELECT button enabled\n", i);
            OutputDebugStringA(debugMsg);
        }
    }
}

void CharacterSelectScene::UpdateCharacterPositions()
{
    int centerX = SCREEN_W / 2 - 100; // 100ピクセル左に移動
    int centerY = SCREEN_H / 2 - 50;
    int radius = 280;

    for (int i = 0; i < (int)characters.size(); i++) {
        Character& character = characters[i];

        if (selectionState == SELECTING) {
            // 回転中の位置計算
            float angle = (float)i * (2.0f * 3.14159265359f / CHARACTER_COUNT) - 3.14159265359f / 2 + rotationAngle;
            character.targetX = centerX + (int)(cos(angle) * radius) - 50;
            character.targetY = centerY + (int)(sin(angle) * radius) - 50;
        }
        else if (selectionState == SELECTED && character.selected) {
            // 選択されたキャラクターを中央に移動
            character.targetX = centerX - 50;
            character.targetY = centerY - 100;
        }
        else if (selectionState == SELECTED) {
            // 他のキャラクターは少し下に移動
            float angle = (float)i * (2.0f * 3.14159265359f / CHARACTER_COUNT) - 3.14159265359f / 2;
            character.targetX = centerX + (int)(cos(angle) * radius * 0.6f) - 50;
            character.targetY = centerY + 100 + (int)(sin(angle) * radius * 0.3f) - 50;
        }

        // 滑らかに目標位置に移動
        character.currentX = (int)Lerp((float)character.currentX, (float)character.targetX, MOVE_SPEED);
        character.currentY = (int)Lerp((float)character.currentY, (float)character.targetY, MOVE_SPEED);
    }
}

// CharacterSelectScene.cpp の UpdateUIButtons 関数を修正

// CharacterSelectScene.cpp の UpdateUIButtons関数を完全修正

void CharacterSelectScene::UpdateUIButtons()
{
    for (int i = 0; i < (int)uiButtons.size(); i++) {
        UIButton& button = uiButtons[i];

        bool over = button.enabled && IsMouseOver(button.x, button.y, button.w, button.h);
        button.hovered = over;

        // スケールアニメーション
        float targetScale = over ? HOVER_SCALE : NORMAL_SCALE;
        button.scale = Lerp(button.scale, targetScale, SCALE_LERP);

        // グロー効果
        float targetGlow = over ? 1.0f : 0.0f;
        button.glowIntensity = Lerp(button.glowIntensity, targetGlow, GLOW_SPEED);

        // **デバッグ情報：マウスオーバー状態**
        if (over) {
            char debugMsg[256];
            sprintf_s(debugMsg, "CharacterSelectScene: Button %d (%s) hovered at (%d,%d) size(%d,%d)\n",
                i, button.label.c_str(), button.x, button.y, button.w, button.h);
            OutputDebugStringA(debugMsg);
        }

        // クリック処理
        if (over && mousePressed && !mousePressedPrev) {
            char debugMsg[256];
            sprintf_s(debugMsg, "CharacterSelectScene: Button %d (%s) clicked, enabled=%s\n",
                i, button.label.c_str(), button.enabled ? "true" : "false");
            OutputDebugStringA(debugMsg);

            // **ボタンラベルで動的判定**
            if (button.label == "TUTORIAL") {
                if (button.enabled) {
                    // キャラクターが選択されていない場合は最初のキャラクターを自動選択
                    if (selectedCharacterIndex < 0) {
                        selectedCharacterIndex = 0;
                        characters[0].selected = true;
                        OutputDebugStringA("CharacterSelectScene: Auto-selected first character for tutorial\n");
                    }
                    tutorialRequested = true;
                    selectionState = CONFIRMED;
                    SoundManager::GetInstance().PlaySE(SoundManager::SFX_SELECT);
                    OutputDebugStringA("CharacterSelectScene: TUTORIAL button clicked\n");
                }
            }
            else if (button.label == "SELECT") {
                if (button.enabled) {
                    selectConfirmed = true;
                    selectionState = CONFIRMED;
                    SoundManager::GetInstance().PlaySE(SoundManager::SFX_SELECT);
                    OutputDebugStringA("CharacterSelectScene: SELECT button clicked\n");
                }
                else {
                    OutputDebugStringA("CharacterSelectScene: SELECT button clicked but disabled\n");
                }
            }
            else if (button.label == "BACK") {
                if (selectionState == SELECTED) {
                    // 選択状態から回転状態に戻る
                    selectionState = SELECTING;
                    selectedCharacterIndex = -1;
                    characterSelected = false;

                    for (auto& c : characters) {
                        c.selected = false;
                    }

                    // SELECTボタンを無効化（動的にラベルで検索）
                    for (auto& btn : uiButtons) {
                        if (btn.label == "SELECT") {
                            btn.enabled = false;
                            OutputDebugStringA("CharacterSelectScene: SELECT button disabled in BACK processing\n");
                            break;
                        }
                    }

                    OutputDebugStringA("CharacterSelectScene: Returning to selection mode\n");
                }
                else {
                    backRequested = true;
                    OutputDebugStringA("CharacterSelectScene: BACK to title requested\n");
                }
            }
        }
    }
}

// 描画関数群（既存のものをそのまま使用）
void CharacterSelectScene::DrawCharacter(const Character& character, int index)
{
    // 浮遊効果
    float yOffset = sinf(character.floatPhase) * FLOAT_AMP;

    // ポップアップエフェクト（イージング適用）
    float easedPopup = EaseOutBack(character.popupProgress);

    // ボタン背景のサイズと位置
    int bgSize = (int)(120 * easedPopup);
    int bgX = character.currentX + 50 - bgSize / 2;
    int bgY = (int)(character.currentY + 50 + yOffset - bgSize / 2);

    // ボタン背景描画（ポップアップ効果）
    if (easedPopup > 0.01f) {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255 * easedPopup));
        DrawExtendGraph(bgX, bgY, bgX + bgSize, bgY + bgSize, buttonBackgroundHandle, TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }

    // グロー効果
    if (character.glowIntensity > 0.01f) {
        int glowColor = character.selected ? GetColor(255, 100, 100) : GetColor(255, 215, 0);
        DrawGlowEffect(
            (int)(character.currentX - 10),
            (int)(character.currentY + yOffset - 10),
            120, 120,
            character.glowIntensity * 0.8f,
            glowColor
        );
    }

    // キャラクターアイコン描画
    float iconScale = character.scale;
    int iconSize = (int)(100 * iconScale);
    int iconX = character.currentX + 50 - iconSize / 2;
    int iconY = (int)(character.currentY + 50 + yOffset - iconSize / 2);

    // 選択状態の場合、特別なエフェクト
    if (character.selected) {
        SetDrawBlendMode(DX_BLENDMODE_ADD, 100);
        DrawExtendGraph(iconX - 5, iconY - 5, iconX + iconSize + 5, iconY + iconSize + 5,
            character.iconHandle, TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }

    DrawExtendGraph(iconX, iconY, iconX + iconSize, iconY + iconSize, character.iconHandle, TRUE);
}

void CharacterSelectScene::DrawUIButton(const UIButton& button)
{
    // ボタンの色設定
    int buttonAlpha = button.enabled ? 255 : 100;
    int textColor = button.enabled ? GetColor(50, 50, 50) : GetColor(100, 100, 100);

    // スケール効果
    float sw = button.w * button.scale;
    float sh = button.h * button.scale;
    float dx = button.x - (sw - button.w) * 0.5f;
    float dy = button.y - (sh - button.h) * 0.5f;

    // グロー効果
    if (button.glowIntensity > 0.01f && button.enabled) {
        DrawGlowEffect((int)dx - 5, (int)dy - 5, (int)sw + 10, (int)sh + 10,
            button.glowIntensity * 0.6f, GetColor(255, 215, 0));
    }

    // ボタン背景
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, buttonAlpha);
    DrawExtendGraph((int)dx, (int)dy, (int)(dx + sw), (int)(dy + sh), uiButtonHandle, TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    // ボタンテキスト
    int textWidth = GetDrawStringWidthToHandle(button.label.c_str(), (int)button.label.length(), fontHandle);
    int textX = button.x + button.w / 2 - textWidth / 2;
    int textY = button.y + button.h / 2 - 14;

    // テキストの影効果
    if (button.enabled) {
        DrawStringToHandle(textX + 2, textY + 2, button.label.c_str(), GetColor(20, 20, 20), fontHandle);
    }
    DrawStringToHandle(textX, textY, button.label.c_str(), textColor, fontHandle);
}

void CharacterSelectScene::DrawGlowEffect(int x, int y, int w, int h, float intensity, int color)
{
    if (intensity <= 0.01f) return;

    int alpha = (int)(intensity * 80);
    SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);

    // 複数のレイヤーでグローを描画
    for (int i = 0; i < 4; i++) {
        int offset = (i + 1) * 2;
        DrawBox(x - offset, y - offset, x + w + offset, y + h + offset, color, FALSE);
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

// ユーティリティ関数の実装
float CharacterSelectScene::EaseOutBack(float t)
{
    const float c1 = 1.70158f;
    const float c3 = c1 + 1;
    return 1 + c3 * powf(t - 1, 3) + c1 * powf(t - 1, 2);
}

float CharacterSelectScene::EaseInOutCubic(float t)
{
    return t < 0.5f ? 4 * t * t * t : 1 - powf(-2 * t + 2, 3) / 2;
}

float CharacterSelectScene::Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

bool CharacterSelectScene::IsMouseOver(int x, int y, int w, int h) const
{
    return mouseX >= x && mouseX <= x + w &&
        mouseY >= y && mouseY <= y + h;
}

bool CharacterSelectScene::IsClicked(int x, int y, int w, int h) const
{
    return IsMouseOver(x, y, w, h) && mousePressed && !mousePressedPrev;
}

void CharacterSelectScene::ResetState()
{
    // 全ての状態を初期化
    characterSelected = false;
    backRequested = false;
    tutorialRequested = false;  // **新追加**
    selectConfirmed = false;
    selectedCharacterIndex = -1;
    hoveredCharacterIndex = -1;
    selectionState = SELECTING;
    rotationAngle = 0.0f;

    // 全キャラクターの状態をリセット
    for (auto& character : characters) {
        character.selected = false;
        character.hovered = false;
        character.scale = NORMAL_SCALE;
        character.hoverProgress = 0.0f;
        character.popupProgress = 0.0f;
        character.glowIntensity = 0.0f;
    }

    // UIボタンの状態をリセット
    for (auto& button : uiButtons) {
        if (button.label == "SELECT") {
            button.enabled = false; // SELECTボタンを無効化
        }
        button.hovered = false;
        button.scale = NORMAL_SCALE;
        button.glowIntensity = 0.0f;
    }
}

// 新規追加メソッド
void CharacterSelectScene::SetTutorialEnabled(bool enabled)
{
    if (tutorialEnabled != enabled) {
        tutorialEnabled = enabled;
        Initialize(); // ボタン配置を再初期化

        char debugMsg[128];
        sprintf_s(debugMsg, "CharacterSelectScene: Tutorial button %s\n",
            enabled ? "enabled" : "disabled");
        OutputDebugStringA(debugMsg);
    }
}