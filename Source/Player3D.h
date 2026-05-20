#pragma once
#include "Object3D.h"
#include "Bullet3D.h"  // 【変更】Bullet3D クラスを使えるようにする
#include <string>

// プレイヤーを管理するクラス
class Player3D : public Object3D
{
private:
	static constexpr float ROTATE_SPEED = 0.2f; // 回転速度

	// 弾を撃つ間隔(フレーム数)   10.0f なら 10フレームに1発
	static constexpr float SHOT_INTERVAL = 20.0f;

public:
	// コンストラクタ（初期座標とモデルのファイル名を受け取る）
	Player3D(VECTOR initPos, std::string filename);
	~Player3D() override;

	void Update() override;
	void Draw() override;
	void MoveEx(); // 移動処理
	void RotationByMove(); // 回転処理の関数
	void Jump();
	void ResolveCollision3D(); // 当たり判定の関数

	void Shot(); // 発射処理

private:
	Model* mpModel; // プレイヤーの3Dモデルを管理するポインタ

private:
	float mfSpeed = 15.0f; // プレイヤーの移動速度

	// 滑らか移動用の慣性  加速度
	VECTOR mvVelocity = VGet(0.0f, 0.0f, 0.0f); // 現在の水平速度（XZ）
	float mfAccel = 40.0f;     // 地上での加速係数
	float mfDecel = 80.0f;     // 地上での減速係数（地上の急減速）
	float mfAirAccel = 15.0f;  // 空中での加速（空中操作量を増やす）
	float mfAirDecel = 10.0f;  // 空中での減速（地上より緩やかにする）


	float mfAngle;        // 現在の回転値
	float mfTargetAngle;  // 目標の回転値
	VECTOR mvOldPosition; // 古いポジション

	// ジャンプ関係
	float mfJumpPower = 40.0f; // ジャンプ力
	float mfGravity = -1.2f;    // 重力

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


private:
	// 連射を制限するためのタイマー用変数
	float mfShotTimer = 0.0f;
};