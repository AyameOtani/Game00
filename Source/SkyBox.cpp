#include "SkyBox.h"
#include "Model.h"
#include "Master.h"
#include "Scene.h"
#include "ObjectManager.h"

// ファイル名版
SkyBox::SkyBox(std::string filename)
	: Object3D(VGet(0.0f, 0.0f, 0.0f))
	, mpModel(nullptr)
	, mfRotation(0.0f)
	, m_ownsModel(true)
{
	mpModel = new Model(filename, VGet(0.0f, 0.0f, 0.0f));
}

// ハンドル版
SkyBox::SkyBox(int modelHandle)
	: Object3D(VGet(0.0f, 0.0f, 0.0f))
	, mpModel(nullptr)
	, mfRotation(0.0f)
	, m_ownsModel(false)
{
	// Model(int handle, VECTOR initPos) を使ってラップ
	mpModel = new Model(modelHandle, VGet(0.0f, 0.0f, 0.0f));
}

SkyBox::~SkyBox()
{
	if (mpModel != nullptr)
	{
		delete mpModel;
		mpModel = nullptr;
	}
}

// 更新処理
void SkyBox::Update()
{
	if (mpModel != nullptr)
	{
		mpModel->Update();
	}


	// プレイヤーの情報取得
	auto pPlayer = Master::mpSceneManager->GetCurrentScene()
		->GetObjectManager()->GetObject3DByTag(Object3D::T_Player3D);

	if (pPlayer != nullptr)
	{
		VECTOR playerPos = pPlayer->GetPosition();
		mpModel->SetPosition(VGet(playerPos.x, 0.0f, playerPos.z));
	}
}

// 描画処理
void SkyBox::Draw()
{
	if (mpModel != nullptr)
	{
		// 一時的にライトの影響をOFFにして描画する
		// note: 影響すると影が出来て暗くなってしまう場合があるため。
		//       逆に影を付けたりしたい場合はこの処理は外してもよい。
		SetUseLighting(FALSE);
		mpModel->Draw();
		SetUseLighting(TRUE);
	}
}

// 拡大値（スケール値）の設定（Modelクラスへの橋渡し）
void SkyBox::SetScale(float scale)
{
	if (mpModel) mpModel->SetScale(scale);
}

// モデルのテクスチャ変更（Modelクラスへの橋渡し）
void SkyBox::SetModelTexture(std::string filename, int index)
{
	if (mpModel) mpModel->SetTexture(filename, index);
}