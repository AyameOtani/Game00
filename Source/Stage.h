#pragma once

#include "Object3D.h"

class Model;

class Stage : public Object3D
{

public:
	 Stage(std::string stageModelName, std::string stageCollisionModelName);
	//Stage(int modelhandel, int collisionHandle);

	~Stage();

	void Update() override;
	void Draw() override;

	// ステージとカプセル型の当たり判定
	bool CheckHit_Capsule(VECTOR pos1, VECTOR pos2, float r);

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

private:
	int mnModelHandle;     // ステージモデルのハンドル
	int mnCollisionHandle; // ステージのコリジョンモデルのハンドル
	float mfRotation; // タイトルで回転したい
	bool mbRota; // 回転をするかのフラグ

};