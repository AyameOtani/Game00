#include "Enemy3D.h"
#include "Model.h"
#include "Bullet3D.h"
#include "Master.h"
#include "SceneManager.h"
#include "Scene.h"
#include "ObjectManager.h"
#include <cmath>


// =========================================================================
// コンストラクタ
// =========================================================================
Enemy3D::Enemy3D(VECTOR initPos, std::string filename)
	: Object3D(initPos) // 基底クラス Object3D に初期座標を渡す
{
	// 自身の mnTag に敵のタグ（T_Enemy3D）をセットして識別できるようにする
	SetTag(Object3D::T_Enemy3D);

	// 敵用の3Dモデルを生成して初期位置に配置
	mpModel = new Model(filename, initPos);
}

// =========================================================================
// デストラクタ
// =========================================================================
Enemy3D::~Enemy3D()
{
	// 生成した3Dモデルを安全に削除
	if (mpModel != nullptr)
	{
		delete mpModel;
		mpModel = nullptr;
	}
}

// =========================================================================
// 更新処理（今は位置を同期させるだけ）
// =========================================================================
void Enemy3D::Update()
{
	// モデルの位置を現在の座標（mvPosition）に同期
	if (mpModel != nullptr)
	{
		mpModel->SetPosition(mvPosition);
		mpModel->Update();
	}

	HitBullet();
	
	// 基底クラス（Object3D）のUpdateを呼び出す
	Object3D::Update();
}

// =========================================================================
// 描画処理
// =========================================================================
void Enemy3D::Draw()
{
	// 敵の3Dモデルを描画
	if (mpModel != nullptr)
	{
		mpModel->Draw();
	}

	// -------------------------------------------------------------------------
// 【追加】デバッグ用：敵のあたり判定の球体をワイヤーフレームで描画（緑色）
// -------------------------------------------------------------------------
	float bulletRadius = 40.0f;
	DrawSphere3D(mvPosition, bulletRadius, 8, GetColor(0, 255, 0), GetColor(0, 255, 0), false);


	// 基底クラス（Object3D）のDrawを呼び出す
	Object3D::Draw();
}


// =========================================================================
// 弾との当たり判定
// =========================================================================
void Enemy3D::HitBullet()
{
	auto bulletList = Master::mpSceneManager->GetCurrentScene()
		->GetObjectManager()
		->GetObject3DListByTag(Object3D::T_Bullet3D);

	for (auto obj : bulletList)
	{
		// Bullet3Dとして扱う
		Bullet3D* pBullet = dynamic_cast<Bullet3D*>(obj);
		if (!pBullet) continue;

		bool isHit = HitCheck_Sphere_Sphere(
			mvPosition, 30.0f,                 // 敵の中心位置と半径（当たり判定）
			pBullet->GetPosition(), 10.0f   // 弾の中心位置と半径（当たり判定）
		);

		if (isHit)
		{
			pBullet->SetDeleteFlag(true);
			SetDeleteFlag(true);
			break;
		}
	}
}