#pragma once
#include "DxLib.h"

class EnemyBase {
public:
    virtual ~EnemyBase() {}

    // ���t���[���̍X�V�E�`��
    virtual void Update() = 0;
    virtual void Draw() = 0;

    // �G�̓����蔻��p�ʒu�E�傫���擾
    virtual int GetX()    const = 0;
    virtual int GetY()    const = 0;
    virtual int GetW()    const = 0;
    virtual int GetH()    const = 0;

    // �v���C���[�Ƃ� AABB �Փ˔���������Ŏ���
    bool IsColliding(int px, int py, int pw, int ph) const {
        int ex = GetX(), ey = GetY(), ew = GetW(), eh = GetH();
        return !(px + pw  < ex ||
            px      > ex + ew ||
            py + ph  < ey ||
            py      > ey + eh);
    }
};
