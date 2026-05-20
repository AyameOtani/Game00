#include "Bullet3D.h"
#include "Model.h"
#include "Master.h"
#include "SceneManager.h"
#include "Scene.h"
#include "ObjectManager.h"
#include "Stage.h" // 【追加】Stageクラスを使うためにインクルード

// =========================================================================
// コンストラクタ（既存のまま）
// =========================================================================
Bullet3D::Bullet3D(VECTOR initPos, std::string filename, VECTOR Direction)
	: Object3D(initPos)
	, mvDirection(Direction)
	, mfSpeed(20.0f)
	, mfMoveSpeed(0.0f)
{
	mvDirection = VNorm(mvDirection);
	SetTag(Object3D::T_Bullet3D);
	mpModel = new Model(filename, initPos);
}

// =========================================================================
// デストラクタ（既存のまま）
// =========================================================================
Bullet3D::~Bullet3D()
{
	if (mpModel != nullptr)
	{
		delete mpModel;
		mpModel = nullptr;
	}
}

// =========================================================================
// 更新処理
// =========================================================================
void Bullet3D::Update()
{
	Move();

	// 【修正】ステージとのあたり判定の呼び出しを有効化！
	HitStage();

	// HitEnemy();
	// HitPlayer();

	Object3D::Update();
}

// =========================================================================
// 移動処理（既存のまま）
// =========================================================================
void Bullet3D::Move()
{
	VECTOR velocity = VScale(mvDirection, mfSpeed);
	mvPosition = VAdd(mvPosition, velocity);
	mfMoveSpeed += mfSpeed;

	if (mpModel != nullptr)
	{
		mpModel->SetPosition(mvPosition);
		mpModel->Update();
	}

	if (mfMoveSpeed > 1500.0f)
	{
		SetDeleteFlag(true);
	}
}

// =========================================================================
// 描画処理
// =========================================================================
void Bullet3D::Draw()
{
	// -------------------------------------------------------------------------
	// 【追加】デバッグ用：弾のあたり判定の球体をワイヤーフレームで描画（緑色）
	// -------------------------------------------------------------------------
	float bulletRadius = 10.0f; // 弾のあたり判定の半径（好みのサイズに調整してね）
	DrawSphere3D(mvPosition, bulletRadius, 8, GetColor(0, 255, 0), GetColor(0, 255, 0), false);

	if (mpModel != nullptr)
	{
		mpModel->Draw();
	}

	Object3D::Draw();
}

// =========================================================================
// 【修正】ステージとのあたり判定
// =========================================================================
void Bullet3D::HitStage()
{
	// 弾の判定用半径（Drawのデバッグ球とサイズを合わせてね）
	float bulletRadius = 10.0f;

	// ゲーム内のステージオブジェクトのリストを取得する
	auto stageList = Master::mpSceneManager->GetCurrentScene()
		->GetObjectManager()->GetObject3DListByTag(Object3D::T_Stage3D);

	// ステージのリストを一つずつループして、弾と当たっているか調べる
	for (auto obj : stageList)
	{
		Stage* pStage = dynamic_cast<Stage*>(obj);
		if (!pStage) continue;

		// 衝突点と法線を受け取るための変数（関数に渡すために必要）
		VECTOR hitPos = VGet(0.0f, 0.0f, 0.0f);
		VECTOR hitNormal = VGet(0.0f, 0.0f, 0.0f);

		// -------------------------------------------------------------------------
		// 【ポイント】始点と終点にどちらも「mvPosition」を渡す！
		// これでカプセル判定が、半径 bulletRadius の「球体判定」として動きます！
		// -------------------------------------------------------------------------
		if (pStage->CheckHit_Capsule_Wall(mvPosition, mvPosition, bulletRadius, hitPos, hitNormal))
		{
			// ステージに当たったら、削除フラグを true にして弾を消去！
			SetDeleteFlag(true);
			break; // ループを抜ける
		}
	}
}

void Bullet3D::HitEnemy()
{
}

void Bullet3D::HitPlayer()
{
}