#include "TitleScene.h"
#include "DxLib.h"
#include <algorithm>
#include <cmath>

TitleScene::TitleScene()
    : m_state(TitleState::FADE_IN)
    , m_startRequested(false)
    , m_exitRequested(false)
    , m_fadeAlpha(255)
    , m_fadeSpeed(3)
    , m_prevUpKey(false)
    , m_prevDownKey(false)
    , m_prevLeftKey(false)
    , m_prevRightKey(false)
    , m_prevEnterKey(false)
    , m_prevSpaceKey(false)
    , m_prevEscapeKey(false)
    , m_selectedMainMenu(MenuOption::START_GAME)
    , m_selectedOption(OptionItem::VOLUME_MASTER)
    , m_menuAnimationTimer(0)
    , m_masterVolume(80)
    , m_bgmVolume(70)
    , m_seVolume(90)
    , m_isFullscreen(false)
    , m_sparkleTimer(0)
{
    InitializeUI();

    // パーティクルシステムの初期化
    m_particles.reserve(50);

    // BGM再生開始
    // PlayMusic("Data/BGM/title.mp3", DX_PLAYTYPE_LOOP);
}

TitleScene::~TitleScene() {
    // リソース解放
    // StopMusic();
}

void TitleScene::InitializeUI() {
    // 画面サイズを取得
    int screenWidth = 1920;
    int screenHeight = 1080;

    // タイトルロゴ（より大きく、中央上部に配置）
    m_titleLogo = {
        screenWidth / 2 - 300, 150,
        600, 150,
        true, 255
    };

    // メインメニューボタン（より大きく、間隔も調整）
    int buttonWidth = 300;
    int buttonHeight = 70;
    int buttonStartY = 500;
    int buttonSpacing = 100;

    m_startButton = {
        screenWidth / 2 - buttonWidth / 2, buttonStartY,
        buttonWidth, buttonHeight,
        true, 255
    };

    m_optionsButton = {
        screenWidth / 2 - buttonWidth / 2, buttonStartY + buttonSpacing,
        buttonWidth, buttonHeight,
        true, 255
    };

    m_exitButton = {
        screenWidth / 2 - buttonWidth / 2, buttonStartY + buttonSpacing * 2,
        buttonWidth, buttonHeight,
        true, 255
    };

    // オプションパネル（より大きく中央に配置）
    m_optionsPanel = {
        screenWidth / 2 - 400, screenHeight / 2 - 250,
        800, 500,
        false, 0
    };
}

void TitleScene::Update() {
    m_menuAnimationTimer++;
    m_sparkleTimer++;

    // スパークルエフェクトの生成
    if (m_sparkleTimer % 30 == 0) {
        CreateSparkleEffect();
    }

    UpdateParticles();

    switch (m_state) {
    case TitleState::FADE_IN:
        UpdateFadeIn();
        break;
    case TitleState::MAIN_MENU:
        UpdateMainMenu();
        break;
    case TitleState::OPTIONS_MENU:
        UpdateOptionsMenu();
        break;
    case TitleState::FADE_OUT:
        UpdateFadeOut();
        break;
    case TitleState::FINISHED:
        break;
    }

    // 前フレームのキー状態を保存
    m_prevUpKey = CheckHitKey(KEY_INPUT_UP) != 0;
    m_prevDownKey = CheckHitKey(KEY_INPUT_DOWN) != 0;
    m_prevLeftKey = CheckHitKey(KEY_INPUT_LEFT) != 0;
    m_prevRightKey = CheckHitKey(KEY_INPUT_RIGHT) != 0;
    m_prevEnterKey = CheckHitKey(KEY_INPUT_RETURN) != 0;
    m_prevSpaceKey = CheckHitKey(KEY_INPUT_SPACE) != 0;
    m_prevEscapeKey = CheckHitKey(KEY_INPUT_ESCAPE) != 0;
}

void TitleScene::UpdateFadeIn() {
    m_fadeAlpha -= m_fadeSpeed;
    if (m_fadeAlpha <= 0) {
        m_fadeAlpha = 0;
        m_state = TitleState::MAIN_MENU;
    }
}

void TitleScene::UpdateMainMenu() {
    HandleMainMenuInput();
}

void TitleScene::UpdateOptionsMenu() {
    HandleOptionsInput();

    // オプションパネルのフェードイン
    if (m_optionsPanel.alpha < 255) {
        m_optionsPanel.alpha += 8;
        if (m_optionsPanel.alpha > 255) {
            m_optionsPanel.alpha = 255;
        }
    }
}

void TitleScene::UpdateFadeOut() {
    m_fadeAlpha += m_fadeSpeed;
    if (m_fadeAlpha >= 255) {
        m_fadeAlpha = 255;
        m_state = TitleState::FINISHED;
    }
}

void TitleScene::HandleMainMenuInput() {
    // 現在のキー状態を取得
    bool currentUpKey = CheckHitKey(KEY_INPUT_UP) != 0;
    bool currentDownKey = CheckHitKey(KEY_INPUT_DOWN) != 0;
    bool currentEnterKey = CheckHitKey(KEY_INPUT_RETURN) != 0;
    bool currentSpaceKey = CheckHitKey(KEY_INPUT_SPACE) != 0;

    // 上下キーでメニュー選択（キーが押された瞬間のみ反応）
    if (currentUpKey && !m_prevUpKey) {
        PlayMenuSound();
        int current = static_cast<int>(m_selectedMainMenu);
        current = (current - 1 + static_cast<int>(MenuOption::COUNT)) % static_cast<int>(MenuOption::COUNT);
        m_selectedMainMenu = static_cast<MenuOption>(current);
    }

    if (currentDownKey && !m_prevDownKey) {
        PlayMenuSound();
        int current = static_cast<int>(m_selectedMainMenu);
        current = (current + 1) % static_cast<int>(MenuOption::COUNT);
        m_selectedMainMenu = static_cast<MenuOption>(current);
    }

    // 決定キーでアクション実行（キーが押された瞬間のみ反応）
    if ((currentEnterKey && !m_prevEnterKey) || (currentSpaceKey && !m_prevSpaceKey)) {
        PlaySelectSound();

        switch (m_selectedMainMenu) {
        case MenuOption::START_GAME:
            m_state = TitleState::FADE_OUT;
            m_startRequested = true;
            break;
        case MenuOption::OPTIONS:
            m_state = TitleState::OPTIONS_MENU;
            m_optionsPanel.isVisible = true;
            m_optionsPanel.alpha = 0;
            break;
        case MenuOption::EXIT:
            m_exitRequested = true;
            break;
        }
    }
}

void TitleScene::HandleOptionsInput() {
    // 現在のキー状態を取得
    bool currentUpKey = CheckHitKey(KEY_INPUT_UP) != 0;
    bool currentDownKey = CheckHitKey(KEY_INPUT_DOWN) != 0;
    bool currentLeftKey = CheckHitKey(KEY_INPUT_LEFT) != 0;
    bool currentRightKey = CheckHitKey(KEY_INPUT_RIGHT) != 0;
    bool currentEnterKey = CheckHitKey(KEY_INPUT_RETURN) != 0;
    bool currentEscapeKey = CheckHitKey(KEY_INPUT_ESCAPE) != 0;

    // 上下キーでオプション選択（キーが押された瞬間のみ反応）
    if (currentUpKey && !m_prevUpKey) {
        PlayMenuSound();
        int current = static_cast<int>(m_selectedOption);
        current = (current - 1 + static_cast<int>(OptionItem::COUNT)) % static_cast<int>(OptionItem::COUNT);
        m_selectedOption = static_cast<OptionItem>(current);
    }

    if (currentDownKey && !m_prevDownKey) {
        PlayMenuSound();
        int current = static_cast<int>(m_selectedOption);
        current = (current + 1) % static_cast<int>(OptionItem::COUNT);
        m_selectedOption = static_cast<OptionItem>(current);
    }

    // 左右キーで値変更（キーが押された瞬間のみ反応）
    if ((currentLeftKey && !m_prevLeftKey) || (currentRightKey && !m_prevRightKey)) {
        PlayMenuSound();
        bool isRightPressed = currentRightKey && !m_prevRightKey;

        switch (m_selectedOption) {
        case OptionItem::VOLUME_MASTER:
            m_masterVolume += isRightPressed ? 10 : -10;
            m_masterVolume = std::clamp(m_masterVolume, 0, 100);
            break;
        case OptionItem::VOLUME_BGM:
            m_bgmVolume += isRightPressed ? 10 : -10;
            m_bgmVolume = std::clamp(m_bgmVolume, 0, 100);
            break;
        case OptionItem::VOLUME_SE:
            m_seVolume += isRightPressed ? 10 : -10;
            m_seVolume = std::clamp(m_seVolume, 0, 100);
            break;
        case OptionItem::FULLSCREEN:
            m_isFullscreen = !m_isFullscreen;
            break;
        }
    }

    // 決定キーまたはESCキーで戻る（キーが押された瞬間のみ反応）
    if ((currentEnterKey && !m_prevEnterKey) || (currentEscapeKey && !m_prevEscapeKey)) {
        if (m_selectedOption == OptionItem::BACK || (currentEscapeKey && !m_prevEscapeKey)) {
            PlaySelectSound();
            m_state = TitleState::MAIN_MENU;
            m_optionsPanel.isVisible = false;
        }
    }
}

void TitleScene::Draw() {
    // 背景のグラデーション（1920x1080対応）
    DrawBox(0, 0, 1920, 1080, GetColor(20, 30, 60), TRUE);
    DrawBox(0, 0, 1920, 1080, GetColor(40, 20, 80), FALSE);

    // パーティクルエフェクト
    DrawParticles();

    switch (m_state) {
    case TitleState::MAIN_MENU:
    case TitleState::FADE_IN:
        DrawMainMenu();
        break;
    case TitleState::OPTIONS_MENU:
        DrawMainMenu(); // 背景として描画
        DrawOptionsMenu();
        break;
    case TitleState::FADE_OUT:
        DrawMainMenu();
        break;
    }

    DrawFade();
}

void TitleScene::DrawMainMenu() {
    // タイトルロゴ（アニメーション付き、フォントサイズも調整）
    int logoY = m_titleLogo.y + static_cast<int>(sin(m_menuAnimationTimer * 0.02) * 8);
    DrawBox(m_titleLogo.x, logoY,
        m_titleLogo.x + m_titleLogo.width, logoY + m_titleLogo.height,
        GetColor(255, 215, 0), FALSE);

    // より大きなフォントでタイトル表示（フォントハンドルがある場合）
    DrawStringToHandle(m_titleLogo.x + 100, logoY + 50, "GAME TITLE",
        GetColor(255, 255, 255), GetDefaultFontHandle());

    // メニューボタン
    const char* menuTexts[] = { "START GAME", "OPTIONS", "EXIT" };
    UIElement* buttons[] = { &m_startButton, &m_optionsButton, &m_exitButton };

    for (int i = 0; i < 3; i++) {
        UIElement* button = buttons[i];
        bool isSelected = (static_cast<int>(m_selectedMainMenu) == i);

        // 選択されているボタンの強調表示
        int color = isSelected ? GetColor(100, 150, 255) : GetColor(60, 60, 100);
        int textColor = isSelected ? GetColor(255, 255, 255) : GetColor(200, 200, 200);

        // 選択時のアニメーション（より大きな動き）
        int offset = isSelected ? static_cast<int>(sin(m_menuAnimationTimer * 0.1) * 5) : 0;

        DrawBox(button->x - offset, button->y,
            button->x + button->width + offset, button->y + button->height,
            color, TRUE);
        DrawBox(button->x - offset, button->y,
            button->x + button->width + offset, button->y + button->height,
            GetColor(255, 255, 255), FALSE);

        // テキスト中央揃え（より大きなフォントに対応）
        int textWidth = GetDrawStringWidth(menuTexts[i], strlen(menuTexts[i]));
        int textX = button->x + (button->width - textWidth) / 2;
        int textY = button->y + (button->height - 20) / 2;  // フォント高さ調整

        DrawString(textX, textY, menuTexts[i], textColor);
    }
}

void TitleScene::DrawOptionsMenu() {
    if (!m_optionsPanel.isVisible) return;

    // オプションパネルの背景
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_optionsPanel.alpha);
    DrawBox(m_optionsPanel.x, m_optionsPanel.y,
        m_optionsPanel.x + m_optionsPanel.width, m_optionsPanel.y + m_optionsPanel.height,
        GetColor(40, 40, 80), TRUE);
    DrawBox(m_optionsPanel.x, m_optionsPanel.y,
        m_optionsPanel.x + m_optionsPanel.width, m_optionsPanel.y + m_optionsPanel.height,
        GetColor(255, 255, 255), FALSE);

    // オプション項目（より大きな間隔で配置）
    const char* optionTexts[] = {
        "Master Volume", "BGM Volume", "SE Volume", "Fullscreen", "Back"
    };

    int startY = m_optionsPanel.y + 80;
    int spacing = 80;

    for (int i = 0; i < static_cast<int>(OptionItem::COUNT); i++) {
        bool isSelected = (static_cast<int>(m_selectedOption) == i);
        int textColor = isSelected ? GetColor(255, 255, 100) : GetColor(255, 255, 255);

        int y = startY + i * spacing;
        DrawString(m_optionsPanel.x + 80, y, optionTexts[i], textColor);

        // 値の表示
        char valueText[32];
        switch (static_cast<OptionItem>(i)) {
        case OptionItem::VOLUME_MASTER:
            sprintf_s(valueText, "%d", m_masterVolume);
            break;
        case OptionItem::VOLUME_BGM:
            sprintf_s(valueText, "%d", m_bgmVolume);
            break;
        case OptionItem::VOLUME_SE:
            sprintf_s(valueText, "%d", m_seVolume);
            break;
        case OptionItem::FULLSCREEN:
            strcpy_s(valueText, m_isFullscreen ? "ON" : "OFF");
            break;
        case OptionItem::BACK:
            strcpy_s(valueText, "");
            break;
        }

        if (strlen(valueText) > 0) {
            DrawString(m_optionsPanel.x + 450, y, valueText, textColor);
        }

        // 選択インジケーター
        if (isSelected) {
            DrawString(m_optionsPanel.x + 40, y, ">", GetColor(255, 255, 100));
        }
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void TitleScene::DrawParticles() {
    for (const auto& particle : m_particles) {
        if (particle.life > 0) {
            int alpha = (particle.life * 255) / 60;
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
            DrawCircle(static_cast<int>(particle.x), static_cast<int>(particle.y), 2, particle.color, TRUE);
        }
    }
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void TitleScene::DrawFade() {
    if (m_fadeAlpha > 0) {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_fadeAlpha);
        DrawBox(0, 0, 1920, 1080, GetColor(0, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
}

void TitleScene::CreateSparkleEffect() {
    Particle newParticle;
    newParticle.x = static_cast<float>(GetRand(1920));
    newParticle.y = static_cast<float>(GetRand(1080));
    newParticle.vx = (GetRand(20) - 10) * 0.1f;
    newParticle.vy = (GetRand(20) - 10) * 0.1f;
    newParticle.life = 60 + GetRand(60);
    newParticle.color = GetColor(255, 255, 200 + GetRand(55));

    m_particles.push_back(newParticle);
}

void TitleScene::UpdateParticles() {
    for (auto& particle : m_particles) {
        if (particle.life > 0) {
            particle.x += particle.vx;
            particle.y += particle.vy;
            particle.life--;
        }
    }

    // 寿命が尽きたパーティクルを削除
    m_particles.erase(
        std::remove_if(m_particles.begin(), m_particles.end(),
            [](const Particle& p) { return p.life <= 0; }),
        m_particles.end()
    );
}

void TitleScene::PlayMenuSound() {
    // PlaySoundFile("Data/SE/menu_move.wav", DX_PLAYTYPE_BACK);
}

void TitleScene::PlaySelectSound() {
    // PlaySoundFile("Data/SE/menu_select.wav", DX_PLAYTYPE_BACK);
}

bool TitleScene::IsStartRequested() const {
    return m_startRequested;
}

bool TitleScene::IsExitRequested() const {
    return m_exitRequested;
}

bool TitleScene::IsFadeOutFinished() const {
    return m_state == TitleState::FINISHED;
}