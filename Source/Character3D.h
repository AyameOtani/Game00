#pragma once

#include "Object3D.h"
#include "Team.h"
#include "DxLib.h"

// Character3D: 共通の当たり判定ロジックとパラメータを保持
class Character3D : public Object3D
{
public:
	Character3D(VECTOR initPos, int maxHp, Team team, float radius);
	virtual ~Character3D();

	virtual void TakeDamage(int damage);

	int GetHp() const { return m_hp; }
	void SetHp(int hp) { m_hp = hp; }

	Team GetTeam() const { return m_team; }

	virtual VECTOR GetHitCenter() const { return VAdd(mvPosition, VGet(0.0f, 0.0f, 0.0f)); }
	virtual VECTOR GetCapsuleTop() const { return VAdd(mvPosition, VGet(0.0f, 60.0f, 0.0f)); }
	virtual VECTOR GetCapsuleBottom() const { return VAdd(mvPosition, VGet(0.0f, 0.0f, 0.0f)); }
	virtual float GetRadius() const { return m_radius; }

	// ステージ当たり判定（共通実装）
	virtual void ResolveCollision3D();

	// モデル同期フック（派生で実装）
	virtual void SyncModel() {}

	// デバッグ表示（派生でオーバーライド可）
	virtual void DebugDraw();

	// 天井判定用半径の取得・設定
	float GetCeilRadius() const { return m_ceilRadius; }
	void SetCeilRadius(float r) { m_ceilRadius = r; }

	// 判定パラメータの追加セッター（必要に応じて使用）
	void SetRadius(float r) { m_radius = r; }
	void SetFloorLinePos(float v) { m_floorLinePos = v; }

protected:
	// 基本ステータス
	int m_maxHp;
	int m_hp;
	Team m_team;

	// 共通の当たり判定半径（床・壁用）
	float m_radius;

	// 天井専用の当たり判定半径（m_radius と別にする）
	float m_ceilRadius;

	// 床判定用パラメータ
	float m_floorCapsuleMinY;
	float m_floorCapsuleMaxY;
	float m_floorLinePos;
	float m_floorLineMinY;
	float m_floorLineMaxY;

	// 壁判定用パラメータ
	float m_wallCapsuleMinY;
	float m_wallCapsuleMaxY;

	// 天井判定用パラメータ
	float m_ceilCapsuleMinY;
	float m_ceilCapsuleMaxY;
	float m_ceilLinePos;
	float m_ceilLineMinY;
	float m_ceilLineMaxY;

	// 物理関連
	VECTOR mvOldPosition;
	VECTOR mvVelocity = VGet(0.0f, 0.0f, 0.0f);
	float mfYVelocity = 0.0f;
	bool mbIsGround = false;
	bool mbJump = false;
	bool mbFall = false;
	float mfGroundY = -FLT_MAX;

	// 向き・運動パラメータ
	float mfAngle = 0.0f;
	float mfAccel = 40.0f;
	float mfDecel = 80.0f;
	float mfAirAccel = 5.0f;
	float mfAirDecel = 10.0f;

	// ジャンプ・重力
	float mfJumpPower = 30.0f;
	float mfGravity = -1.2f;
	float mfMaxFallSpeed = 60.0f;
};