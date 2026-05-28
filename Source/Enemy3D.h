#pragma once
#include "Character3D.h"
#include <string>

class Model;

// 敵クラス：Character3D の当たり判定/物理処理を共有して利用する
class Enemy3D : public Character3D
{
public:
	enum EnemyType
	{
		Attacker,
		Jumper,
		Runner,
	};

public:
	Enemy3D(VECTOR initPos, EnemyType type);
	~Enemy3D() override;

	void Update() override;
	void Draw() override;
	void DebugDraw(); // デバッグ描画
	void Shot(); // 発射処理

	// 敵のタイプに応じたモデルのパスを返す関数
	std::string GetModelPath(Enemy3D::EnemyType type);

	// モデル同期（Character3D のフックを実装）
	void SyncModel() override;

public:
	void SetID(int id) { m_id = id; }
	int GetID() const { return m_id; }

	// Character3D のインタフェースをオーバーライド
	VECTOR GetHitCenter() const override;
	float GetRadius() const override;

private:
	Model* mpModel = nullptr;      // 敵の 3D モデル
	EnemyType m_type;              // 敵のタイプ

	float mfAngle = 0.0f;         // 回転
	float mfTargetAngle = 0.0f;   // 目標回転

	// 発射管理
	float mfShotTimer = 0.0f;
	static constexpr float SHOT_INTERVAL = 120.0f;

	// 表示用 ID
	int m_id = -1;
	static int s_nextEnemyID;
private:
	float mfJumpTimer = 0.0f; // ジャンプ用のタイマー
};