#pragma once

#include "Object3D.h"

class Model;

class Stage : public Object3D
{
public:
	// ステージの種類
	enum class StageType
	{
		Static,     // 動きなし
		MoveSide,   // 左右に動く
		MoveUpDown, // 上下に動く
		Rotate      // 回転する
	};

public:
	// 既存：ファイル名から読み込むコンストラクタ
	Stage(std::string stageModelName, std::string stageCollisionModelName, VECTOR iniPos = VGet(0.0f, 0.0f, 0.0f), StageType type = StageType::Static);

	// 追加：既に読み込まれたモデルハンドルを渡すコンストラクタ（所有権は渡さない）
	Stage(int modelHandle, int collisionHandle,
		VECTOR iniPos = VGet(0.0f, 0.0f, 0.0f), StageType type = StageType::Static);

	~Stage();

	void Update() override;
	void Draw() override;

	bool CheckHit_Capsule(VECTOR pos1, VECTOR pos2, float r);
	bool CheckHit_Line_Normal(VECTOR pos1, VECTOR pos2, VECTOR& hitNormal);
	VECTOR CheckHit_Line(VECTOR pos1, VECTOR pos2);

	int GetModelHandle() const;

	bool CheckHit_Capsule_Wall(VECTOR pos1, VECTOR pos2, float r, VECTOR& hitPos, VECTOR& hitNormal);

	void TitleRotate();
	bool GetRotate() const { return mbRota; }
	void SetRotate(bool rt) { mbRota = rt; }

	void SetScale(float scale);

	// セット用の関数
	void SetRotateParam(VECTOR pivot, float speed, float range);

	StageType GetType() const { return m_type; }

private:
	StageType m_type;
	int mnModelHandle;
	int mnCollisionHandle;
	bool m_ownsModel;
	bool m_ownsCollision;
	float mfRotation;
	bool mbRota;
	float mfMoveTime;

	VECTOR m_prevPosition; // 前フレームの座標
	VECTOR m_prevRotation; // 前フレームの回転
	VECTOR m_posDelta;     // 1フレームあたりの移動量
	VECTOR m_rotDelta;     // 1フレームあたりの回転変化量
	VECTOR m_originPos;

public:
	VECTOR GetPositionDelta() const { return m_posDelta; }
	VECTOR GetRotationDelta() const { return m_rotDelta; }

	// メンバ変数として追加
	VECTOR m_rotatePivot;   // 回転軸
	float m_rotateSpeed;    // 回転スピードの倍率
	float m_rotateRange;    // 回転の揺れ幅（角度）
	VECTOR m_iniPos;


public:
	// 回転軸の種類
	enum class RotateAxis
	{
		X,
		Z
	};

	// 既存の関数に加え、軸設定用を追加
	void SetRotateParam(float speed, float range, RotateAxis axis = RotateAxis::Z);

private:
	RotateAxis m_rotateAxis = RotateAxis::Z; // デフォルトはZ軸
};