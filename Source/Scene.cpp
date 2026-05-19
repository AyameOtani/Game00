#include "Scene.h"
#include "ObjectManager.h"

Scene::Scene()
{
	// オブジェクトマネージャーの作成
	mpObjectManager = new ObjectManager();
}

Scene::~Scene()
{
	delete mpObjectManager; // オブジェクトマネージャーの削除
}

// 更新
void Scene::Update()
{
	if (mpObjectManager != nullptr) //空じゃないならアプデする
	{
		mpObjectManager->Update();
	}

}

// 描画
void Scene::Draw()
{

	if (mpObjectManager != nullptr) // 空じゃないなら描画する
	{
		mpObjectManager->Draw();
	}

}