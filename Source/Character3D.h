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
	int m_maxHp;                  // HP最大値
	int m_hp;                     // 現在HP
	Team m_team;                  // チーム（Player / Enemyなど）

	// 共通の当たり判定半径（床・壁用）
	float m_radius;               // キャラ本体の当たり半径

	// 天井専用の当たり判定半径
	float m_ceilRadius;           // 天井判定専用半径（床と分離）

	// 床判定用パラメータ
	float m_floorCapsuleMinY;     // 床判定カプセル下端Y
	float m_floorCapsuleMaxY;     // 床判定カプセル上端Y
	float m_floorLinePos;         // 床判定の横サンプル距離
	float m_floorLineMinY;        // 床レイ開始Y
	float m_floorLineMaxY;        // 床レイ終了Y

	// 壁判定用パラメータ
	float m_wallCapsuleMinY;      // 壁判定カプセル下端Y
	float m_wallCapsuleMaxY;      // 壁判定カプセル上端Y

	// 天井判定用パラメータ
	float m_ceilCapsuleMinY;      // 天井判定カプセル下端Y
	float m_ceilCapsuleMaxY;      // 天井判定カプセル上端Y
	float m_ceilLinePos;          // 天井判定横サンプル距離
	float m_ceilLineMinY;         // 天井レイ開始Y
	float m_ceilLineMaxY;         // 天井レイ終了Y

	// 物理関連
	VECTOR mvOldPosition;         // 前フレーム位置
	VECTOR mvVelocity;            // 移動速度ベクトル
	float mfYVelocity;            // Y方向速度（ジャンプ・落下）
	bool mbIsGround;              // 接地フラグ
	bool mbJump;                  // ジャンプ中フラグ
	bool mbFall;                  // 落下中フラグ
	float mfGroundY;              // 接地している地面Y（拡張用）

	// 向き・運動パラメータ
	float mfAngle;                // キャラの向き角度（ラジアン）
	float mfAccel;                // 地上加速度
	float mfDecel;                // 地上減速度
	float mfAirAccel;             // 空中加速度
	float mfAirDecel;             // 空中減速度

	// ジャンプ・重力
	float mfJumpPower;            // ジャンプ初速
	float mfGravity;              // 重力加速度
	float mfMaxFallSpeed;         // 最大落下速度

};