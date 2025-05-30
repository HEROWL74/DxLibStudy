#pragma once
#include "DxLib.h"
#include <vector>
#include <math.h>

// パーティクル構造体
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

// 星エフェクト構造体
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
    // パーティクルシステム
    std::vector<Particle> particles;
    std::vector<StarEffect> stars;

    // タイマー
    int frameCounter;
    int particleSpawnTimer;

    // エフェクト用の色
    static const int PARTICLE_COLORS[];
    static const int PARTICLE_COLOR_COUNT;

public:
    EffectManager();
    ~EffectManager();

    void Initialize();
    void Update();
    void Draw();
    void Finalize();

    // タイトル画面用エフェクト開始
    void StartTitleEffects();

    // パーティクル生成
    void CreateParticle(float x, float y, float vx, float vy, int color, float life);
    void CreateRandomParticles();

    // 星エフェクト生成
    void CreateStarEffects();

    // 各種エフェクト描画
    void DrawParticles();
    void DrawStars();
    void DrawTitleGlow();
};