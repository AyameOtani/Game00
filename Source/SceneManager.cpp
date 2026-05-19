#include "SceneManager.h"
#include "Scene.h"
#include "Master.h"
#include "TitleScene.h"
#include "GameScene.h"
#include "WinResultScene.h"

SceneManager::SceneManager()
	: mnSceneType(SCENE_TYPE::SCENE_NONE)
	, mnNextSceneType(SCENE_TYPE::SCENE_NONE)
	, mpCurrentScene(nullptr)
{

}

SceneManager::~SceneManager()
{

}

void SceneManager::Initialize()
{
	// 初期シーンの設定
	/*mnNextSceneType = SCENE_TYPE::SCENE_3D;*/

	// 初期シーンの変更　　シーンの追加
	mnNextSceneType = SCENE_TYPE::SCENE_3D;
	//mnNextSceneType = SCENE_TYPE::SELECT_SCENE_3D;


	//mnNextSceneType = SCENE_TYPE::SELECT_SCENE_3D;


	// シーン遷移をさせる
	ChangeSceneIfNeeded();
}

void SceneManager::Update()
{
	// シーンの更新
	mpCurrentScene->Update();
}

void SceneManager::Draw()
{
	// シーンの描画
	mpCurrentScene->Draw();
}

void SceneManager::Finalize()
{

}

void SceneManager::ChangeSceneIfNeeded()
{
	// 現在シーンと次のシーンが一緒であれば何もしない
	if (mnSceneType == mnNextSceneType)
	{
		return;
	}

	if (mpCurrentScene != nullptr)
	{
		// 現在シーンの終了処理をする
		mpCurrentScene->Finalize();

		// 一旦シーン自体も破棄しておく
		delete mpCurrentScene;
		mpCurrentScene = nullptr;
	}

	// 次シーンにするためシーンタイプ更新
	mnSceneType = mnNextSceneType;


	// mnSceneType に応じてシーンを作成する
	switch (mnSceneType)
	{
	case SCENE_TYPE::SCENE_3D:
		mpCurrentScene = new GameScene(); // タイトル3Dシーンの生成 10/28
		break;

	case SCENE_TYPE::TITLE_3D:
		mpCurrentScene = new TitleScene(); // 3Dシーンの生成
		break;

	case SCENE_TYPE::WIN_RESULT_3D:
		mpCurrentScene = new WinResultScene(); // 勝った時の3Dシーンの生成
		break;

	default:
		break;
	}

	// シーンの生成がされているはずなので、初期化処理を呼んでおく
	mpCurrentScene->Initialize();

}