#pragma once
#include "Object3D.h"
#include "Team.h"
#include <string>

class Model;
class Character3D;

class Bullet3D : public Object3D
{
public:
    // 弾丸の生成：初期位置、モデル名、進行方向、発射チームを指定
    Bullet3D(VECTOR initPos, std::string filename, VECTOR Direction, Team shooterTeam);
    ~Bullet3D();

    void Update() override;
    void Draw() override;
    void Move();

    // 衝突判定：ステージ用とキャラクター用を分離
    void HitStage();
    void HitCharacter();

    float GetRadius() const { return m_radius; }

private:
    VECTOR mvDirection;
    Team m_shooterTeam; // 味方への誤射を防ぐための所属チーム情報

    float mfMoveSpeed = 0.0f;
    float mfSpeed = 40.0f;     // 毎フレームの移動量
    float m_radius = 10.0f;    // 衝突判定用の半径

    // エフェクト生成間隔を制御するための距離蓄積用変数
    float mEffectDistance = 0.0f;

    Model* mpModel = nullptr;

    float mfScale; // モデル描画時の拡大率
};