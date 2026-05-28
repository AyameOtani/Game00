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
	Stage(std::string stageModelName, std::string stageCollisionModelName, StageType type = StageType::Static);

	// 追加：既に読み込まれたモデルハンドルを渡すコンストラクタ（所有権は渡さない）
	Stage(int modelHandle, int collisionHandle, StageType type = StageType::Static);

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

private:
	StageType m_type;
	int mnModelHandle;
	int mnCollisionHandle;
	bool m_ownsModel;
	bool m_ownsCollision;
	float mfRotation;
	bool mbRota;
	float mfMoveTime;

	// --- 追加部分 ---
	VECTOR m_prevPosition; // 前フレームの座標
	VECTOR m_prevRotation; // 前フレームの回転
	VECTOR m_posDelta;     // 1フレームあたりの移動量
	VECTOR m_rotDelta;     // 1フレームあたりの回転変化量

public:
	VECTOR GetPositionDelta() const { return m_posDelta; }
	VECTOR GetRotationDelta() const { return m_rotDelta; }
};