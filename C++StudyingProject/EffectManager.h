#pragma once
#include "DxLib.h"
#include <vector>
#include <math.h>

// �p�[�e�B�N���\����
struct Particle
{
    float x, y;
    float vx, vy;
    float life;
    float maxLife;
    int color;
    float size;
    float alpha;

    Particle() : x(0), y(0), vx(0), vy(0), life(0), maxLife(0), color(0), size(0), alpha(1.0f) {}
};

// ���G�t�F�N�g�\����
struct StarEffect
{
    float x, y;
    float scale;
    float rotation;
    float rotSpeed;
    float alpha;
    float pulseSpeed;
    float pulsePhase;

    StarEffect() : x(0), y(0), scale(1.0f), rotation(0), rotSpeed(0), alpha(1.0f), pulseSpeed(0), pulsePhase(0) {}
};

class EffectManager
{
private:
    // �p�[�e�B�N���V�X�e��
    std::vector<Particle> particles;
    std::vector<StarEffect> stars;

    // �^�C�}�[
    int frameCounter;
    int particleSpawnTimer;

    // �G�t�F�N�g�p�̐F
    static const int PARTICLE_COLORS[];
    static const int PARTICLE_COLOR_COUNT;

public:
    EffectManager();
    ~EffectManager();

    void Initialize();
    void Update();
    void Draw();
    void Finalize();

    // �^�C�g����ʗp�G�t�F�N�g�J�n
    void StartTitleEffects();

    // �p�[�e�B�N������
    void CreateParticle(float x, float y, float vx, float vy, int color, float life);
    void CreateRandomParticles();

    // ���G�t�F�N�g����
    void CreateStarEffects();

    // �e��G�t�F�N�g�`��
    void DrawParticles();
    void DrawStars();
    void DrawTitleGlow();
};