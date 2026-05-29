#pragma once

#include "Object3D.h"

class Model;

class Stage : public Object3D
{
public:
	// ステージの挙動タイプ
	enum class StageType
	{
		Static,     // 停止
		MoveSide,   // 左右移動
		MoveUpDown, // 上下移動
		Rotate      // 往復回転
	};

	// 回転軸の指定
	enum class RotateAxis
	{
		X,
		Z
	};

public:
	// ファイル名から読み込む場合（リソースの所有権あり）
	Stage(std::string stageModelName, std::string stageCollisionModelName, VECTOR iniPos = VGet(0.0f, 0.0f, 0.0f), StageType type = StageType::Static);

	// 外部からハンドルを受け取る場合（リソースの所有権なし）
	Stage(int modelHandle, int collisionHandle, VECTOR iniPos = VGet(0.0f, 0.0f, 0.0f), StageType type = StageType::Static);

	~Stage();

	void Update() override;
	void Draw() override;


	// --- 当たり判定関連 ---
	// カプセル形状による接触判定（衝突の有無を返す）
	bool CheckHit_Capsule(VECTOR pos1, VECTOR pos2, float r);

	// 線分による接触判定（ヒット時の法線ベクトルを算出し、壁の押し出し計算などに使用）
	bool CheckHit_Line_Normal(VECTOR pos1, VECTOR pos2, VECTOR& hitNormal);

	// 線分による接触判定（ヒット地点の座標を返す）
	VECTOR CheckHit_Line(VECTOR pos1, VECTOR pos2);

	// 壁や床に沿った移動用（接触時の位置と向きを特定し、補正値を計算する）
	bool CheckHit_Capsule_Wall(VECTOR pos1, VECTOR pos2, float r, VECTOR& hitPos, VECTOR& hitNormal);



	// --- モデル制御関連 ---
	// 描画・判定用モデルハンドルの取得
	int GetModelHandle() const;

	// ステージごとの独自回転演出処理
	void TitleRotate();

	// 回転演出のON/OFF制御
	bool GetRotate() const { return mbRota; }
	void SetRotate(bool rt) { mbRota = rt; }

	// モデルとコリジョンの表示・判定スケールの一括更新
	void SetScale(float scale);

	// 回転挙動（速度、振れ幅、軸）の動的設定
	void SetRotateParam(float speed, float range, RotateAxis axis = RotateAxis::Z);



	StageType GetType() const { return m_type; }

	// 移動量取得用（プレイヤーの追従計算などに使用）
	VECTOR GetPositionDelta() const { return m_posDelta; }
	VECTOR GetRotationDelta() const { return m_rotDelta; }

private:
	StageType m_type;
	int mnModelHandle;
	int mnCollisionHandle;
	bool m_ownsModel;     // モデルの破棄権限フラグ
	bool m_ownsCollision; // コリジョンの破棄権限フラグ

	float mfRotation;
	bool mbRota;
	float mfMoveTime;     // 移動・回転計算用タイマー

	VECTOR m_prevPosition; // 差分計算用の前フレーム座標
	VECTOR m_prevRotation; // 差分計算用の前フレーム回転量
	VECTOR m_posDelta;     // 1フレームの移動量
	VECTOR m_rotDelta;     // 1フレームの回転変化量
	VECTOR m_originPos;    // 初期位置（計算基準）
	VECTOR m_iniPos;       // 配置時の初期位置

public:
	VECTOR m_rotatePivot;  // 回転中心座標
	float m_rotateSpeed;   // 回転速度
	float m_rotateRange;   // 回転の振れ幅

private:
	RotateAxis m_rotateAxis = RotateAxis::Z;
};