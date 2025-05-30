#include "EffectManager.h"

// �ÓI�����o�̒�`
const int EffectManager::PARTICLE_COLORS[] = {
    GetColor(255, 215, 0),  // �S�[���h
    GetColor(255, 255, 255), // �z���C�g
    GetColor(255, 192, 203), // �s���N
    GetColor(135, 206, 250), // ���C�g�u���[
    GetColor(255, 165, 0),   // �I�����W
    GetColor(50, 205, 50),   // ���C���O���[��
};

const int EffectManager::PARTICLE_COLOR_COUNT = sizeof(PARTICLE_COLORS) / sizeof(PARTICLE_COLORS[0]);

EffectManager::EffectManager()
{
    frameCounter = 0;
    particleSpawnTimer = 0;
}

EffectManager::~EffectManager()
{
    Finalize();
}

void EffectManager::Initialize()
{
    particles.clear();
    stars.clear();
    frameCounter = 0;
    particleSpawnTimer = 0;
}

void EffectManager::Update()
{
    frameCounter++;
    particleSpawnTimer++;

    // �p�[�e�B�N���̍X�V
    for (auto it = particles.begin(); it != particles.end();)
    {
        it->x += it->vx;
        it->y += it->vy;
        it->life -= 1.0f;
        it->alpha = it->life / it->maxLife;

        // �d�͌���
        it->vy += 0.05f;

        // �������s�����p�[�e�B�N�����폜
        if (it->life <= 0)
        {
            it = particles.erase(it);
        }
        else
        {
            ++it;
        }
    }

    // ���G�t�F�N�g�̍X�V
    for (auto& star : stars)
    {
        star.rotation += star.rotSpeed;
        star.pulsePhase += star.pulseSpeed;
        star.alpha = 0.5f + 0.5f * sin(star.pulsePhase);
    }

    // ����I�Ƀp�[�e�B�N���𐶐�
    if (particleSpawnTimer > 30)  // 0.5�b�Ԋu
    {
        CreateRandomParticles();
        particleSpawnTimer = 0;
    }
}

void EffectManager::Draw()
{
    DrawTitleGlow();
    DrawStars();
    DrawParticles();
}

void EffectManager::StartTitleEffects()
{
    CreateStarEffects();
    CreateRandomParticles();
}

void EffectManager::CreateParticle(float x, float y, float vx, float vy, int color, float life)
{
    Particle p;
    p.x = x;
    p.y = y;
    p.vx = vx;
    p.vy = vy;
    p.life = life;
    p.maxLife = life;
    p.color = color;
    p.size = 2.0f + (rand() % 4);
    p.alpha = 1.0f;

    particles.push_back(p);
}

void EffectManager::CreateRandomParticles()
{
    // ��ʏ㕔���烉���_���Ƀp�[�e�B�N���𐶐�
    for (int i = 0; i < 5; i++)
    {
        float x = rand() % 1280;
        float y = -10;
        float vx = -2 + (rand() % 5);
        float vy = 1 + (rand() % 3);
        int colorIndex = rand() % PARTICLE_COLOR_COUNT;
        float life = 120 + (rand() % 120);  // 2-4�b

        CreateParticle(x, y, vx, vy, PARTICLE_COLORS[colorIndex], life);
    }
}

void EffectManager::CreateStarEffects()
{
    stars.clear();

    // ��ʂɃ����_���ɐ���z�u
    for (int i = 0; i < 20; i++)
    {
        StarEffect star;
        star.x = rand() % 1280;
        star.y = rand() % 720;
        star.scale = 0.5f + (rand() % 100) / 100.0f;
        star.rotSpeed = -0.02f + (rand() % 100) / 2500.0f;
        star.pulseSpeed = 0.05f + (rand() % 100) / 2000.0f;
        star.pulsePhase = (rand() % 628) / 100.0f;  // 0-2��

        stars.push_back(star);
    }
}

void EffectManager::DrawParticles()
{
    for (const auto& p : particles)
    {
        // �A���t�@�l��K�p�����F�ŕ`��
        int alpha = (int)(p.alpha * 255);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

        DrawCircle((int)p.x, (int)p.y, (int)p.size, p.color, TRUE);

        // �������
        SetDrawBlendMode(DX_BLENDMODE_ADD, alpha / 2);
        DrawCircle((int)p.x, (int)p.y, (int)(p.size * 1.5f), p.color, FALSE);
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

void EffectManager::DrawStars()
{
    for (const auto& star : stars)
    {
        int alpha = (int)(star.alpha * 180);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

        // ���̕`��i�\���`�j
        int size = (int)(10 * star.scale);
        int color = GetColor(255, 255, 255);

        // ��]���l�������\���̕`��
        float cos_r = cos(star.rotation);
        float sin_r = sin(star.rotation);

        int x1 = (int)(star.x + size * cos_r);
        int y1 = (int)(star.y + size * sin_r);
        int x2 = (int)(star.x - size * cos_r);
        int y2 = (int)(star.y - size * sin_r);

        int x3 = (int)(star.x + size * sin_r);
        int y3 = (int)(star.y - size * cos_r);
        int x4 = (int)(star.x - size * sin_r);
        int y4 = (int)(star.y + size * cos_r);

        DrawLine(x1, y1, x2, y2, color, 2);
        DrawLine(x3, y3, x4, y4, color, 2);

        // ���S�ɏ����ȉ~
        DrawCircle((int)star.x, (int)star.y, 2, color, TRUE);
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

void EffectManager::DrawTitleGlow()
{
    // �^�C�g���̈�Ɍ������
    int alpha = (int)(100 + 50 * sin(frameCounter * 0.05f));
    SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);

    // �O���f�[�V�����~��`��
    for (int i = 0; i < 5; i++)
    {
        int radius = 150 + i * 30;
        int centerX = 640;
        int centerY = 200;

        DrawCircle(centerX, centerY, radius, GetColor(100, 150, 255), FALSE);
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

void EffectManager::Finalize()
{
    particles.clear();
    stars.clear();
}