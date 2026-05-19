#include "DxLib.h"
#include "Texture.h"  // ../は一階層上にあるものを読み込むという意味
#include "TextureAnimation.h"
#include "Collision.h"
#include "SceneManager.h"
#include "Master.h"
#include "ObjectManager.h"
#include "Scene.h"
#include "Camera.h"
#include "ResourceManager.h"
#include "InputManager.h"
#include "Utility.h"


#ifdef  _DEBUG
#else // リソースモード
#endif //  _DEBUG
// ここにDrawStringとかを書くとReleaseモードにしても表示されない。



/*
 @note リファレンス https://dxlib.xsrv.jp/dxfunc.html
*/

// Master クラスの静的メンバ変数定義
SceneManager* Master::mpSceneManager = new SceneManager();
SoundManager* Master::mpSoundManager = new SoundManager();
Camera* Master::mpCamera = new Camera();

// 追加　リソースまねーじゃーのやつ
ResourceManager* Master::mpResourceManager = new ResourceManager();


/**
* @fn WinMain
* @brief Main関数
* @param[in] HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow
* @return int 0 正常終了／-1 エラー
* @details Main関数
*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	// ウインドウモードで起動  falseにすると全画面
	ChangeWindowMode(true);
	// ChangeWindowMode(false); 

	//SetGraphMode(1280, 800, 0.0f); // 元々のサイズ
	SetGraphMode(1920, 1080, 0);  // モニター解像度に合わせる
	//ChangeWindowMode(false);       // フルスクリーン

	SetWindowIconID(101); // アイコンを変更する★★

	// DXライブラリ初期化
	if (DxLib_Init() == -1)
	{
		return -1;
	}


	// サウンドマネージャーの初期化 （シーンより先にやらないと流れないので注意）
	Master::mpSoundManager->Initialize();   // 全てのサウンドが読み込まれる（BGMやS

	// シーンマネージャーの生成と初期化
	Master::mpSceneManager->Initialize();

	// カメラの初期化
	Master::mpCamera->Initialize(); // 初期化処理を呼ぶ




	// 描画先画面を裏画面に設定する
	SetDrawScreen(DX_SCREEN_BACK);


	// Texture クラスを使って描画
	VECTOR position;
	position.x = 640 / 2;
	position.y = 480 / 2;

	// Zバッファに書き込む準備
	// ここで奥行がちゃんとなる
	SetUseZBufferFlag(true);
	SetWriteZBufferFlag(true);


	// DX_FOGMODE_NONEの方も試してみたけど何もならんから調べる

	//// ------フォグ設定 ------
	//SetFogEnable(TRUE);// フォグ有効
	//SetFogMode(DX_FOGMODE_LINEAR);// フォグモード
	//SetFogStartEnd(3000.0f, 18000.0f);// フォグが始まる距離と終了する距離を設定する
	//SetFogColor(255, 200, 180); // 色
	//SetFogDensity(0.001f);// 濃度らしいけど変わっているか分からん

	// メモ　フォグのレンジとかで３０％だったりにするとSkyBoxとかとの両立Ｏらしい。

	// ゲームのメインループ
	// ProcessMessage() == 0  ウィンドウの☓ボタン押されていないかどうか
	// CheckHitKey(KEY_INPUT_ESCAPE) == 0  エスケープキーが押されていないかどうか
	int animationCounter = 0;
	int textureCurrentNum = 0;
	while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
	{
		//画面を初期化する
		ClearDrawScreen();

		int time = GetNowCount();

		// カメラの更新
		Master::mpCamera->Update();

		// 更新
		Master::mpSceneManager->Update();

		// 描画
		Master::mpSceneManager->Draw();


		// スクショのやつ
		if (InputManager::CheckDownKey(KEY_INPUT_F1))
		{
			SaveDrawScreenToJPEG(0, 0, Utility::SCREEN_WIDTH, Utility::SCREEN_HEIGHT,
				"screenshot1.png");
		}

		// 裏画面の内容を表画面に映す
		ScreenFlip();


		// 17ミリ秒（秒間約60フレームだった場合の１フレーム当たりの経過時間）
		// 経過するまでここで待つ
		while (GetNowCount() - time < 17)
		{
			// 待つだけなので何も処理はしない
		}

		// 削除する必要のあるオブジェクトがあれば削除する
		Master::mpSceneManager->GetCurrentScene()->GetObjectManager()->DeleteAll2DIfNeeded();
		Master::mpSceneManager->GetCurrentScene()->GetObjectManager()->DeleteAll3DIfNeeded();

		// ループする直前にシーン遷移チェックをいれておく
		Master::mpSceneManager->ChangeSceneIfNeeded();
	}


	// 終了処理
	Master::mpSceneManager->Finalize();
	//★
	delete Master::mpSceneManager; // いらなくなるのでdelete する

	// サウンドの終了処理
	Master::mpSoundManager->Finalize();
	delete Master::mpSoundManager;

	delete Master::mpResourceManager; // リソースマネージャーの削除
	//★
	//delete pAnimation;

	// DXライブラリ使用の終了
	DxLib_End();

	// ソフトの終了
	return 0;
}