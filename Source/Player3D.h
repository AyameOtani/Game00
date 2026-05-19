#pragma once
#include "Object3D.h"
#include <string>

// プレイヤーを管理するクラス
class Player3D : public Object3D
{
private:
	static constexpr float ROTATE_SPEED = 0.2f; // 回転速度

public:
	// コンストラクタ（初期座標とモデルのファイル名を受け取る）
	Player3D(VECTOR initPos, std::string filename);
	~Player3D() override;

	void Update() override;
	void Draw() override;
	void MoveEx(); // 移動処理（ステージとのあたり判定用）
	void RotationByMove(); // 回転処理の関数

	void Jump();

private:
	Model* mpModel; // プレイヤーの3Dモデルを管理するポインタ

private:
	float mfSpeed = 8.0f; // プレイヤーの移動速度


	float mfAngle;       // 現在の回転値
	float mfTargetAngle; // 目標の回転値
	VECTOR mvOldPosition; // 古いポジション

	// ジャンプ関係
	float mfJumpPower = 25.0f;
	float mfGravity = -1.2f;
	float mfYVelocity = 0.0f;
	bool  mbIsGround = true;
};

