#pragma once
#include "DxLib.h"

class EnemyBase {
public:
    virtual ~EnemyBase() {}

    // 毎フレームの更新・描画
    virtual void Update() = 0;
    virtual void Draw() = 0;

    // 敵の当たり判定用位置・大きさ取得
    virtual int GetX()    const = 0;
    virtual int GetY()    const = 0;
    virtual int GetW()    const = 0;
    virtual int GetH()    const = 0;

    // プレイヤーとの AABB 衝突判定をここで実装
    bool IsColliding(int px, int py, int pw, int ph) const {
        int ex = GetX(), ey = GetY(), ew = GetW(), eh = GetH();
        return !(px + pw  < ex ||
            px      > ex + ew ||
            py + ph  < ey ||
            py      > ey + eh);
    }
};
