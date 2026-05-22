#pragma once
#include "Object3D.h"
#include "Team.h"
#include <string>

class Model;
class Character3D;

class Bullet3D : public Object3D
{
public:
    Bullet3D(VECTOR initPos, std::string filename, VECTOR Direction, Team shooterTeam);
    ~Bullet3D();

    void Update() override;
    void Draw() override;
    void Move();

    void HitStage();
    void HitCharacter();

    float GetRadius() const { return m_radius; }

private:
    VECTOR mvDirection;
    Team m_shooterTeam;

    float mfMoveSpeed = 0.0f;
    float mfSpeed = 40.0f;
    float m_radius = 10.0f;

    // エフェクトの距離蓄積
    float mEffectDistance = 0.0f;

    Model* mpModel = nullptr;
};