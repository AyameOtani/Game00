#include "SkyBox.h"
#include "Model.h"
#include "Master.h"
#include "Scene.h"
#include "ObjectManager.h"

// コンストラクタ
SkyBox::SkyBox(std::string filename)
	: Object3D(VGet(0.0f, 0.0f, 0.0f))
{
	// スカイボックスモデルの生成
	// note: スカイボックスの座標は、基本的には原点。
	//       ものによっては、座標を常にプレイヤーの座標にすることで、空が見切れないようにすることもある。
	mpModel = new Model(filename, VGet(0.0f, 0.0f, 0.0f));
	mfRotation = 0.0f; //初期化
}

// デストラクタ
SkyBox::~SkyBox()
{
	// モデルクラスの破棄
	if (mpModel != nullptr)
	{
		delete mpModel;
	}
}

// 更新処理
void SkyBox::Update()
{
	if (mpModel != nullptr)
	{
		mpModel->Update();
	}


	//// 回転するようにした　1210
	//mfRotation += 0.0003f; // ここで回転速度
	//if (mfRotation > DX_TWO_PI_F) // 360を越したら
	//{
	//	mfRotation -= DX_TWO_PI_F; // 今の回転角から３６０分引く
	//}
	//mvRotation.y = mfRotation;
	//mpModel->SetRotation(mvRotation); // modelに回転速度をセット


	// プレイヤーの情報取得
	auto pPlayer = Master::mpSceneManager->GetCurrentScene()
		->GetObjectManager()->GetObject3DByTag(Object3D::T_Player3D);

	if (pPlayer != nullptr)
	{
		// でかくしすぎたらカメラがバグるので、追跡するようにした。
		//　追跡させたらおおきくて敵が隠れるからやめたほうが良いかも？
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
	if (mpModel != nullptr)
	{
		mpModel->SetScale(scale);
	}
}

// モデルのテクスチャ変更（Modelクラスへの橋渡し）
void SkyBox::SetModelTexture(std::string filename, int index)
{
	if (mpModel != nullptr)
	{
		mpModel->SetTexture(filename, index);
	}
}