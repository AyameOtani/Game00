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
Enemy3D::Enemy3D(VECTOR initPos, std::string filename, EnemyType type)
	: Object3D(initPos), m_type(type) // 敵タイプを初期化
	, mfAngle(0.0f)
	, mfTargetAngle(0.0f)
	, mvOldPosition(initPos)
{
	// 自身の mnTag に敵のタグ（T_Enemy3D）をセットして識別できるようにする
	SetTag(Object3D::T_Enemy3D);
	// 敵用の3Dモデルを生成して初期位置に配置
	mpModel = new Model(filename, initPos);



	m_radius = 30.0f; // プレイヤーの当たり判定半径

	// 床判定用の各高さ設定
	m_floorCapsuleMinY = 3.0f;
	m_floorCapsuleMaxY = 40.0f;
	m_floorLinePos = 25.0f;
	m_floorLineMinY = 20.0f;
	m_floorLineMaxY = -300.0f;

	// 壁判定用の各高さ設定
	m_wallCapsuleMinY = 40.0f;
	m_wallCapsuleMaxY = 60.0f;

	// 天井判定用の各高さ設定
	m_ceilCapsuleMinY = 60.0f;
	m_ceilCapsuleMaxY = 80.0f;
	m_ceilLinePos = 15.0f;
	m_ceilLineMinY = 70.0f;
	m_ceilLineMaxY = 100.0f;

	SetFontSize(20);
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

	// デバッグ用：敵のあたり判定の球体をワイヤーフレームで描画（緑色）
	float bulletRadius = 40.0f;
	DrawSphere3D(mvPosition, bulletRadius, 8, GetColor(0, 255, 0), GetColor(0, 255, 0), false);


	// 基底クラス（Object3D）のDrawを呼び出す
	Object3D::Draw();
}