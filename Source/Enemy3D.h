#pragma once
#include "Object3D.h"
#include <string>

class Model;

// =========================================================================
// 敵を管理するクラス（最小限の表示用ベース版）
// =========================================================================
class Enemy3D : public Object3D
{
public:
	// コンストラクタ（初期位置、敵用モデルのファイル名を受け取る）
	Enemy3D(VECTOR initPos, std::string filename);

	// デストラクタ
	~Enemy3D() override;

	// 毎フレームの更新処理
	void Update() override;

	// 毎フレームの描画処理
	void Draw() override;

	void HitBullet(); // 弾とのあたり判定

private:
	Model* mpModel; // 敵の3Dモデルを管理するポインタ
};