#include "CharacterSelectScene.h"
#include "SoundManager.h"
using namespace std;

CharacterSelectScene::CharacterSelectScene()
    : characterSelected(false)
    , backRequested(false)
    , tutorialRequested(false)      // **�V�ǉ�**
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
    ,tutorialEnabled(true) //�f�t�H���g�ŗL���ɂ��Ă���
{
}

CharacterSelectScene::~CharacterSelectScene()
{
    DeleteGraph(backgroundHandle);
    DeleteGraph(buttonBackgroundHandle);
    DeleteGraph(uiButtonHandle);
    DeleteFontToHandle(fontHandle);
    DeleteFontToHandle(largeFontHandle);

    // �L�����N�^�[�A�C�R���̍폜
    for (auto& character : characters) {
        DeleteGraph(character.iconHandle);
    }
}

void CharacterSelectScene::Initialize()
{
    // �e�N�X�`���ǂݍ���
    backgroundHandle = LoadGraph("Sprites/Backgrounds/background_fade_trees.png");
    buttonBackgroundHandle = LoadGraph("UI/PNG/Yellow/button_square_gradient.png");
    uiButtonHandle = LoadGraph("UI/PNG/Yellow/button_rectangle_depth_gradient.png");
    fontHandle = CreateFontToHandle(NULL, 28, 3);
    largeFontHandle = CreateFontToHandle(NULL, 48, 5);

    // �L�����N�^�[���ݒ�
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

    // �L�����N�^�[�z�u�i�~�`�z�u�j
    int centerX = SCREEN_W / 2 - 100; // 100�s�N�Z�����Ɉړ�
    int centerY = SCREEN_H / 2 - 50; // ������ɔz�u
    int radius = 280;

    characters.clear();
    for (int i = 0; i < CHARACTER_COUNT; i++) {
        Character character;
        character.name = characterNames[i];
        character.iconPath = iconPaths[i];
        character.iconHandle = LoadGraph(iconPaths[i].c_str());

        // �~�`�z�u�̌v�Z
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

    // UI�{�^���ݒ�̏C��
    uiButtons.clear();
    int buttonY = SCREEN_H - 150;
    int buttonCenterX = SCREEN_W / 2 - 100;

    // **TUTORIAL�{�^���i�����t���Œǉ��j**
    if (tutorialEnabled) {
        UIButton tutorialButton;
        tutorialButton.x = buttonCenterX - 300;  // ���[�ɔz�u
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

    // **SELECT�{�^���i�`���[�g���A���L���Ɋ֌W�Ȃ������ɔz�u�j**
    UIButton selectButton;
    selectButton.x = buttonCenterX - 90;  // ��ɒ���
    selectButton.y = buttonY;
    selectButton.w = 180;
    selectButton.h = 60;
    selectButton.label = "SELECT";
    selectButton.scale = NORMAL_SCALE;
    selectButton.glowIntensity = 0.0f;
    selectButton.hovered = false;
    selectButton.enabled = false;
    uiButtons.push_back(selectButton);

    // **BACK�{�^���i�E���ɔz�u�j**
    UIButton backButton;
    backButton.x = buttonCenterX + 120;  // �E��
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
    // �}�E�X���͍X�V
    GetMousePoint(&mouseX, &mouseY);
    mousePressedPrev = mousePressed;
    mousePressed = (GetMouseInput() & MOUSE_INPUT_LEFT) != 0;

    // �L�[�{�[�h���͍X�V
    backspacePressedPrev = backspacePressed;
    backspacePressed = CheckHitKey(KEY_INPUT_BACK) != 0;

    // �^�C�g����������
    titlePulsePhase += 0.02f;

    // ��]�A�j���[�V�����i�I�𒆂̂݁j
    if (selectionState == SELECTING) {
        rotationAngle += ROTATION_SPEED;
        if (rotationAngle >= 2.0f * 3.14159265359f) {
            rotationAngle -= 2.0f * 3.14159265359f;
        }
    }

    // Backspace�L�[�Ŗ߂�
    if (backspacePressed && !backspacePressedPrev) {
        if (selectionState == SELECTED) {
            // �I����Ԃ����]��Ԃɖ߂�
            selectionState = SELECTING;
            selectedCharacterIndex = -1;
            characterSelected = false;

            // �S�L�����N�^�[�̑I����Ԃ��N���A
            for (auto& c : characters) {
                c.selected = false;
            }

            // SELECT�{�^���𖳌���
            if (uiButtons.size() > 1) {
                uiButtons[1].enabled = false;
            }
        }
        else {
            // �I�𒆏�Ԃ���^�C�g����ʂɖ߂�
            backRequested = true;
        }
    }

    // �L�����N�^�[�X�V
    UpdateCharacters();

    // �L�����N�^�[�ʒu�X�V
    UpdateCharacterPositions();

    // UI�{�^���X�V
    UpdateUIButtons();
}

void CharacterSelectScene::Draw()
{
    // �w�i�`��
    DrawExtendGraph(0, 0, SCREEN_W, SCREEN_H, backgroundHandle, TRUE);

    // �^�C�g���`��i�������ʕt���j
    float titleScale = 1.0f + sinf(titlePulsePhase) * 0.03f;
    int titleAlpha = (int)(255 * (0.9f + sinf(titlePulsePhase * 2) * 0.1f));
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, titleAlpha);

    string titleText = "SELECT CHARACTER";
    int titleWidth = GetDrawStringWidthToHandle(titleText.c_str(), (int)titleText.length(), largeFontHandle);
    int titleX = SCREEN_W / 2 - (int)(titleWidth * titleScale) / 2;
    int titleY = 150;

    // �^�C�g���̃O���[����
    DrawGlowEffect(titleX - 20, titleY - 10, (int)(titleWidth * titleScale) + 40, 60, 0.2f, GetColor(255, 215, 0));
    DrawStringToHandle(titleX, titleY, titleText.c_str(), GetColor(255, 255, 255), largeFontHandle);

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    // �L�����N�^�[�`��
    for (int i = 0; i < (int)characters.size(); i++) {
        DrawCharacter(characters[i], i);
    }

    // UI�{�^���`��
    for (const auto& button : uiButtons) {
        DrawUIButton(button);
    }

    // �������
    if (selectionState == SELECTING) {
        DrawStringToHandle(50, SCREEN_H - 120, "Click on a character to select, or click TUTORIAL for quick practice", GetColor(200, 200, 200), fontHandle);
        DrawStringToHandle(50, SCREEN_H - 80, "Press [Backspace] or BACK button to return to title", GetColor(150, 150, 150), fontHandle);
    }
    else if (selectionState == SELECTED) {
        DrawStringToHandle(50, SCREEN_H - 120, "Click SELECT to start game, TUTORIAL to practice, or BACK to reselect", GetColor(200, 200, 200), fontHandle);
        DrawStringToHandle(50, SCREEN_H - 80, "Press [Backspace] to return to character selection", GetColor(150, 150, 150), fontHandle);
    }

    // �I�����ꂽ�L�����N�^�[�̖��O�\��
    if (hoveredCharacterIndex >= 0 && hoveredCharacterIndex < (int)characters.size()) {
        const Character& hoveredChar = characters[hoveredCharacterIndex];
        string nameText = hoveredChar.name;
        int nameWidth = GetDrawStringWidthToHandle(nameText.c_str(), (int)nameText.length(), fontHandle);
        int nameX = SCREEN_W / 2 - nameWidth / 2;
        int nameY = SCREEN_H - 200;

        // ���O�̔w�i
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
        DrawBox(nameX - 20, nameY - 10, nameX + nameWidth + 20, nameY + 40, GetColor(0, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

        DrawStringToHandle(nameX, nameY, nameText.c_str(), GetColor(255, 215, 0), fontHandle);
    }
}

// CharacterSelectScene.cpp �� UpdateCharacters�֐����C��

void CharacterSelectScene::UpdateCharacters()
{
    hoveredCharacterIndex = -1;

    for (int i = 0; i < (int)characters.size(); i++) {
        Character& character = characters[i];

        // �A�C�R���T�C�Y��100x100�Ɖ���
        int iconSize = 100;
        bool over = IsMouseOver(character.currentX, character.currentY, iconSize, iconSize);
        character.hovered = over;

        if (over) {
            hoveredCharacterIndex = i;
        }

        // �X�P�[���A�j���[�V����
        float targetScale = over ? HOVER_SCALE : NORMAL_SCALE;
        if (character.selected && selectionState == SELECTED) {
            targetScale = 1.3f; // �I�����ꂽ�L�����N�^�[�͂���ɑ傫��
        }
        character.scale = Lerp(character.scale, targetScale, SCALE_LERP);

        // �z�o�[�v���O���X
        float targetHover = over ? 1.0f : 0.0f;
        character.hoverProgress = Lerp(character.hoverProgress, targetHover, POPUP_SPEED);

        // �|�b�v�A�b�v����
        character.popupProgress = Lerp(character.popupProgress, targetHover, POPUP_SPEED);

        // �O���[����
        float targetGlow = over ? 1.0f : 0.0f;
        if (character.selected) {
            targetGlow = 1.0f; // �I�����ꂽ�L�����N�^�[�͏�ɃO���[
        }
        character.glowIntensity = Lerp(character.glowIntensity, targetGlow, GLOW_SPEED);

        // ���V����
        character.floatPhase += FLOAT_SPEED;

        // �N���b�N����
        if (over && mousePressed && !mousePressedPrev && selectionState != CONFIRMED) {
            // �ȑO�̑I�����N���A
            for (auto& c : characters) {
                c.selected = false;
            }

            selectedCharacterIndex = i;
            character.selected = true;
            characterSelected = true;
            selectionState = SELECTED;

            // **�C��: SELECT�{�^���𓮓I�Ɍ������ėL����**
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
    int centerX = SCREEN_W / 2 - 100; // 100�s�N�Z�����Ɉړ�
    int centerY = SCREEN_H / 2 - 50;
    int radius = 280;

    for (int i = 0; i < (int)characters.size(); i++) {
        Character& character = characters[i];

        if (selectionState == SELECTING) {
            // ��]���̈ʒu�v�Z
            float angle = (float)i * (2.0f * 3.14159265359f / CHARACTER_COUNT) - 3.14159265359f / 2 + rotationAngle;
            character.targetX = centerX + (int)(cos(angle) * radius) - 50;
            character.targetY = centerY + (int)(sin(angle) * radius) - 50;
        }
        else if (selectionState == SELECTED && character.selected) {
            // �I�����ꂽ�L�����N�^�[�𒆉��Ɉړ�
            character.targetX = centerX - 50;
            character.targetY = centerY - 100;
        }
        else if (selectionState == SELECTED) {
            // ���̃L�����N�^�[�͏������Ɉړ�
            float angle = (float)i * (2.0f * 3.14159265359f / CHARACTER_COUNT) - 3.14159265359f / 2;
            character.targetX = centerX + (int)(cos(angle) * radius * 0.6f) - 50;
            character.targetY = centerY + 100 + (int)(sin(angle) * radius * 0.3f) - 50;
        }

        // ���炩�ɖڕW�ʒu�Ɉړ�
        character.currentX = (int)Lerp((float)character.currentX, (float)character.targetX, MOVE_SPEED);
        character.currentY = (int)Lerp((float)character.currentY, (float)character.targetY, MOVE_SPEED);
    }
}

// CharacterSelectScene.cpp �� UpdateUIButtons �֐����C��

// CharacterSelectScene.cpp �� UpdateUIButtons�֐������S�C��

void CharacterSelectScene::UpdateUIButtons()
{
    for (int i = 0; i < (int)uiButtons.size(); i++) {
        UIButton& button = uiButtons[i];

        bool over = button.enabled && IsMouseOver(button.x, button.y, button.w, button.h);
        button.hovered = over;

        // �X�P�[���A�j���[�V����
        float targetScale = over ? HOVER_SCALE : NORMAL_SCALE;
        button.scale = Lerp(button.scale, targetScale, SCALE_LERP);

        // �O���[����
        float targetGlow = over ? 1.0f : 0.0f;
        button.glowIntensity = Lerp(button.glowIntensity, targetGlow, GLOW_SPEED);

        // **�f�o�b�O���F�}�E�X�I�[�o�[���**
        if (over) {
            char debugMsg[256];
            sprintf_s(debugMsg, "CharacterSelectScene: Button %d (%s) hovered at (%d,%d) size(%d,%d)\n",
                i, button.label.c_str(), button.x, button.y, button.w, button.h);
            OutputDebugStringA(debugMsg);
        }

        // �N���b�N����
        if (over && mousePressed && !mousePressedPrev) {
            char debugMsg[256];
            sprintf_s(debugMsg, "CharacterSelectScene: Button %d (%s) clicked, enabled=%s\n",
                i, button.label.c_str(), button.enabled ? "true" : "false");
            OutputDebugStringA(debugMsg);

            // **�{�^�����x���œ��I����**
            if (button.label == "TUTORIAL") {
                if (button.enabled) {
                    // �L�����N�^�[���I������Ă��Ȃ��ꍇ�͍ŏ��̃L�����N�^�[�������I��
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
                    // �I����Ԃ����]��Ԃɖ߂�
                    selectionState = SELECTING;
                    selectedCharacterIndex = -1;
                    characterSelected = false;

                    for (auto& c : characters) {
                        c.selected = false;
                    }

                    // SELECT�{�^���𖳌����i���I�Ƀ��x���Ō����j
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

// �`��֐��Q�i�����̂��̂����̂܂܎g�p�j
void CharacterSelectScene::DrawCharacter(const Character& character, int index)
{
    // ���V����
    float yOffset = sinf(character.floatPhase) * FLOAT_AMP;

    // �|�b�v�A�b�v�G�t�F�N�g�i�C�[�W���O�K�p�j
    float easedPopup = EaseOutBack(character.popupProgress);

    // �{�^���w�i�̃T�C�Y�ƈʒu
    int bgSize = (int)(120 * easedPopup);
    int bgX = character.currentX + 50 - bgSize / 2;
    int bgY = (int)(character.currentY + 50 + yOffset - bgSize / 2);

    // �{�^���w�i�`��i�|�b�v�A�b�v���ʁj
    if (easedPopup > 0.01f) {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255 * easedPopup));
        DrawExtendGraph(bgX, bgY, bgX + bgSize, bgY + bgSize, buttonBackgroundHandle, TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }

    // �O���[����
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

    // �L�����N�^�[�A�C�R���`��
    float iconScale = character.scale;
    int iconSize = (int)(100 * iconScale);
    int iconX = character.currentX + 50 - iconSize / 2;
    int iconY = (int)(character.currentY + 50 + yOffset - iconSize / 2);

    // �I����Ԃ̏ꍇ�A���ʂȃG�t�F�N�g
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
    // �{�^���̐F�ݒ�
    int buttonAlpha = button.enabled ? 255 : 100;
    int textColor = button.enabled ? GetColor(50, 50, 50) : GetColor(100, 100, 100);

    // �X�P�[������
    float sw = button.w * button.scale;
    float sh = button.h * button.scale;
    float dx = button.x - (sw - button.w) * 0.5f;
    float dy = button.y - (sh - button.h) * 0.5f;

    // �O���[����
    if (button.glowIntensity > 0.01f && button.enabled) {
        DrawGlowEffect((int)dx - 5, (int)dy - 5, (int)sw + 10, (int)sh + 10,
            button.glowIntensity * 0.6f, GetColor(255, 215, 0));
    }

    // �{�^���w�i
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, buttonAlpha);
    DrawExtendGraph((int)dx, (int)dy, (int)(dx + sw), (int)(dy + sh), uiButtonHandle, TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    // �{�^���e�L�X�g
    int textWidth = GetDrawStringWidthToHandle(button.label.c_str(), (int)button.label.length(), fontHandle);
    int textX = button.x + button.w / 2 - textWidth / 2;
    int textY = button.y + button.h / 2 - 14;

    // �e�L�X�g�̉e����
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

    // �����̃��C���[�ŃO���[��`��
    for (int i = 0; i < 4; i++) {
        int offset = (i + 1) * 2;
        DrawBox(x - offset, y - offset, x + w + offset, y + h + offset, color, FALSE);
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

// ���[�e�B���e�B�֐��̎���
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
    // �S�Ă̏�Ԃ�������
    characterSelected = false;
    backRequested = false;
    tutorialRequested = false;  // **�V�ǉ�**
    selectConfirmed = false;
    selectedCharacterIndex = -1;
    hoveredCharacterIndex = -1;
    selectionState = SELECTING;
    rotationAngle = 0.0f;

    // �S�L�����N�^�[�̏�Ԃ����Z�b�g
    for (auto& character : characters) {
        character.selected = false;
        character.hovered = false;
        character.scale = NORMAL_SCALE;
        character.hoverProgress = 0.0f;
        character.popupProgress = 0.0f;
        character.glowIntensity = 0.0f;
    }

    // UI�{�^���̏�Ԃ����Z�b�g
    for (auto& button : uiButtons) {
        if (button.label == "SELECT") {
            button.enabled = false; // SELECT�{�^���𖳌���
        }
        button.hovered = false;
        button.scale = NORMAL_SCALE;
        button.glowIntensity = 0.0f;
    }
}

// �V�K�ǉ����\�b�h
void CharacterSelectScene::SetTutorialEnabled(bool enabled)
{
    if (tutorialEnabled != enabled) {
        tutorialEnabled = enabled;
        Initialize(); // �{�^���z�u���ď�����

        char debugMsg[128];
        sprintf_s(debugMsg, "CharacterSelectScene: Tutorial button %s\n",
            enabled ? "enabled" : "disabled");
        OutputDebugStringA(debugMsg);
    }
}