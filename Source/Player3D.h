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


	void ResolveCollision3D(); // 当たり判定の関数
private:
	Model* mpModel; // プレイヤーの3Dモデルを管理するポインタ

private:
	float mfSpeed = 15.0f; // プレイヤーの移動速度


	float mfAngle;       // 現在の回転値
	float mfTargetAngle; // 目標の回転値
	VECTOR mvOldPosition; // 古いポジション

	// ジャンプ関係
	float mfJumpPower = 40.0f;
	float mfGravity = -1.2f;
   // 落下の最大速度（正の値）。これを超えないようにする。
	float mfMaxFallSpeed = 60.0f;
	float mfYVelocity = 0.0f;
	bool  mbIsGround = true;


	bool  mbJump = false;    // ジャンプ中フラグ（以前の床判定用）
	bool  mbFall = false;    // 落下中フラグ
	float mfGroundY = 0.0f;  // 現在の床の高さ
	bool  mbHitUp = false;   // 天井に頭が当たっているかフラグ


private:
	// -------------------------------------------------------------------------
	// 当たり判定カプセル用パラメータ
	// -------------------------------------------------------------------------
	// ★共通の半径
	float m_radius;           // すべてのカプセルで共通の半径（50.0f）

	// 床判定用位置 (緑)
	float m_floorCapsuleMinY;
	float m_floorCapsuleMaxY;
	float m_floorLinePos;
	float m_floorLineMinY;
	float m_floorLineMaxY;
	// 床判定でラインのどちらを採用するか（true=最高点、false=最低点）
	bool m_floorUseHighest = true;

	// 壁判定用位置 (赤)
	float m_wallCapsuleMinY;
	float m_wallCapsuleMaxY;

	// 天井判定用位置 (青)
	float m_ceilCapsuleMinY;
	float m_ceilCapsuleMaxY;
	float m_ceilLinePos;
	float m_ceilLineMinY;
	float m_ceilLineMaxY;
    // 天井判定でラインのどちらを採用するか（true=最低点、false=最高点）
	bool m_ceilUseLowest = true;
};

