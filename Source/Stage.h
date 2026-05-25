#pragma once

#include "Object3D.h"

class Model;

class Stage : public Object3D
{

public:
	// ステージの種類
	enum class StageType
	{
		Static,    // 動かない
		Moving,    // 平行移動する
		Rotating,   // 回転する
		LittleRotation // 少しの回転
	};

public:
	 Stage(std::string stageModelName, std::string stageCollisionModelName, StageType type = StageType::Static);
	//Stage(int modelhandel, int collisionHandle);

	~Stage();

	void Update() override;
	void Draw() override;

	// ステージとカプセル型の当たり判定
	bool CheckHit_Capsule(VECTOR pos1, VECTOR pos2, float r);

	// 代替案：構造体を使わず、参照渡しで法線（傾き）を受け取る関数
	bool CheckHit_Line_Normal(VECTOR pos1, VECTOR pos2, VECTOR& hitNormal);

	// ステージと線分との当たり判定
	VECTOR CheckHit_Line(VECTOR pos1, VECTOR pos2);

	int GetModelHandle() const; // ステージモデルのコリジョンハンドル

	// ステージとカプセル型の当たり判定
	// 当たったらtrue を返し、&hitPosと&hitNormal に値を返す関数の作成（壁沿いベクトルのため）
	bool CheckHit_Capsule_Wall(VECTOR pos1, VECTOR pos2, float r, VECTOR& hitPos, VECTOR& hitNormal);

	void TitleRotate(); // タイトル画面では回転させるため
	// 回転するかのゲッターセッター
	bool GetRotate() const { return mbRota; }
	void SetRotate(bool rt) { mbRota = rt; }

	// スケール（拡大率）の設定
	void SetScale(float scale);

private:

	StageType m_type; // ステージのタイプ

	int mnModelHandle;     // ステージモデルのハンドル
	int mnCollisionHandle; // ステージのコリジョンモデルのハンドル
	float mfRotation; // タイトルで回転したい
	bool mbRota; // 回転をするかのフラグ

	float mfMoveTime;


};