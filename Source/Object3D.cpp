#include "Object3D.h"
#include "Master.h"
#include "ObjectManager.h"
#include "Scene.h"
#include "Model.h"

Object3D::Object3D(VECTOR initPos)
	:mvPosition(initPos)
	, mvRotation(VGet(0.0f, 0.0f, 0.0f))
	, mbDeleteFlag(false)
	, mnTag(Tag3D::None3D) //初期値は設定なし
	, mbAttack(false)
	, mb_EAttack(false)
	, mfCurrentCameraDistance(0.0f)
	, mpModel(nullptr)
{
	// モデルの作成
	//mpModel = new Model(filename, initPos);

	// 現在シーンの ObjectManager に自身(this) を追加する
	// 
	// ★★  AddObject は名前を変えていないのに呼びだせる
	//       -> 同じ名前の時は、引数の型や個数が違う場合なら同じ関数名を使える
	Master::mpSceneManager->GetCurrentScene()->GetObjectManager()->AddObject(this);
}

Object3D::~Object3D()
{

}

void Object3D::Update()
{

}

void Object3D::Draw()
{

}
void Object3D::LateUpdate()
{
	if (mpModel)
	{
		mpModel->GetPosition();
		mpModel->SetPosition(mvPosition);
		mpModel->SetRotation(mvRotation);

	}
}


