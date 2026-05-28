#pragma once
#include "Character3D.h"
#include <string>

class Model;

// Player3D は Character3D の派生クラスとして、モデル同期を実装する。
// 当たり判定パラメータは Character3D 側で protected になっているため
// コンストラクタ内で個別に調整します。
class Player3D : public Character3D
{
private:
	static constexpr float ROTATE_SPEED = 0.2f;

public:
	Player3D(VECTOR initPos, std::string filename);
	~Player3D() override;

	void Update() override;
	void Draw() override;

	void MoveEx();
	void RotationByMove();
	void ResolveCharacterPush(); // キャラの押し戻し処理
	void Jump();
	void DrawHp(); // HPの画像の描画

	// Player3D 固有のショットなど
	void Shot();

	// モデル同期（Character3D::SyncModel を実装）
	void SyncModel() override;

public:
	bool IsMarkedForDelete() const { return mbMarkedForDelete; }

private:
	Model* mpModel = nullptr;

	// 移動・ジャンプパラメータ（Character3D 側にもあるが、Player固有の値を保持）
	float mfSpeed = 15.0f;
	VECTOR mvVelocity = VGet(0.0f, 0.0f, 0.0f);
	float mfAccel = 40.0f;
	float mfDecel = 80.0f;
	float mfAirAccel = 15.0f;
	float mfAirDecel = 10.0f;

	float mfAngle;
	float mfTargetAngle;

	// 連射を制限するためのタイマー用変数
	float mfShotTimer = 0.0f;// 弾を撃つ間隔(フレーム数)   10.0f なら 10フレームに1発
	static constexpr float SHOT_INTERVAL = 20.0f;

	// HP画像ハンドル
	int mnHeartFullImg; // ライフのハンドル
	int mnHeartEmptyImg; // 失ったHPのハンドル
	int mnHpBox; // HPの背景ボックスのハンドル

	bool mbMarkedForDelete = false; // OnDeathで立てる（本当に削除するのはObjectManager任せ）
};