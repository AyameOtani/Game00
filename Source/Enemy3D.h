#pragma once
#include "Character3D.h"
#include <string>

class Model;

// =========================================================================
// 敵を管理するクラス（最小限の表示用ベース版）
// =========================================================================
class Enemy3D : public Character3D
{

public:

	enum EnemyType
	{
		Attacker, // 攻撃型
		Jumper,   // ジャンプ型
		Runner,   // 走行型
	};

public:
	// コンストラクタ（初期位置、敵用モデルのファイル名、敵タイプを受け取る）
	Enemy3D(VECTOR initPos, std::string filename, EnemyType type);
	// デストラクタ
	~Enemy3D() override;

	void Update() override;
	void Draw() override;
	void DebugDraw(); // 描画デバッグ用
	void ResolveCollision3D(); // 当たり判定の関数
	void Shot(); // 発射処理


private:
	Model* mpModel; // 敵の3Dモデルを管理するポインタ
	EnemyType m_type; // 敵のタイプ（攻撃型、ジャンプ型、走行型など）

	float mfAngle;        // 現在の回転値
	float mfTargetAngle;  // 目標の回転値
	VECTOR mvOldPosition; // 古いポジション


	// 落下の最大速度（正の値）。これを超えないようにする。
	float mfMaxFallSpeed = 60.0f;
	float mfYVelocity = 0.0f;
	bool  mbIsGround = true;


	bool  mbJump = false;    // ジャンプ中フラグ（以前の床判定用）
	bool  mbFall = false;    // 落下中フラグ
	float mfGroundY = 0.0f;  // 現在の床の高さ
	bool  mbHitUp = false;   // 天井に頭が当たっているかフラグ

	// ゲッターセッター
public:
	void SetID(int id) { m_id = id; }
	int GetID() const { return m_id; }


	// Character3D クラスからのオーバーライド：当たり判定の中心位置を返す
	VECTOR GetHitCenter() const override
	{
		return VAdd(mvPosition, VGet(0.0f, 10.0f, 0.0f));
	}

	float GetRadius() const override
	{
		return m_radius;
	}


	// 弾の処理
private:
	float mfShotTimer = 0.0f; // 攻撃用タイマー
	static constexpr float SHOT_INTERVAL = 60.0f; // 弾をうつ間隔

private:
	// -------------------------------------------------------------------------
	// 当たり判定カプセル用パラメータ
	// -------------------------------------------------------------------------
	// ★共通の半径
	float m_radius = 60.0f;           // すべてのカプセルで共通の半径（50.0f）

	//// 床判定用位置 (緑)
	//float m_floorCapsuleMinY;
	//float m_floorCapsuleMaxY;
	//float m_floorLinePos;
	//float m_floorLineMinY;
	//float m_floorLineMaxY;
	//// 床判定でラインのどちらを採用するか（true=最高点、false=最低点）
	//bool m_floorUseHighest = true;

	//// 壁判定用位置 (赤)
	//float m_wallCapsuleMinY;
	//float m_wallCapsuleMaxY;

	//// 天井判定用位置 (青)
	//float m_ceilCapsuleMinY;
	//float m_ceilCapsuleMaxY;
	//float m_ceilLinePos;
	//float m_ceilLineMinY;
	//float m_ceilLineMaxY;
	// 天井判定でラインのどちらを採用するか（true=最低点、false=最高点）
	bool m_ceilUseLowest = true;

	int m_id = -1; // ID用メンバ変数

};