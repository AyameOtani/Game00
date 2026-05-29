#pragma once

#include "SceneManager.h"
#include "SoundManager.h"
#include "ResourceManager.h"
#include "Camera.h"
#include <memory>

/**
 * @brief モデル＋コリジョンをセットで持つ構造体
 */
struct ModelSet
{
    int model = -1;      // 描画用
    int collision = -1;  // 当たり判定用
};

/**
 * @brief ゲーム全体で共有するリソース管理クラス
 */
class Master
{
public:
    static SceneManager* mpSceneManager;
    static SoundManager* mpSoundManager;
    static ResourceManager* mpResourceManager;
    static Camera* mpCamera;

    // ===== モデルセットで管理 =====

    static ModelSet sky;         // SkyBox
    static ModelSet stage;       // 通常ステージ
    static ModelSet slideStage;  // 移動ステージ
    static ModelSet updownStage; // 回転ステージ
    static ModelSet rotaStage;   // 微回転ステージ
};


//#pragma once
//
//#include "SceneManager.h"
//#include "SoundManager.h"
//#include "ResourceManager.h"
//#include "Camera.h"
//#include <memory>
//
///**
// * @brief ゲーム全体で共有するリソースやシステムを管理するクラス
// * 全てのシーンからアクセス可能な静的メンバーを保持します。
// */
//class Master
//{
//public:
//	static SceneManager* mpSceneManager;      // ゲームのシーン遷移を管理するマネージャー
//	static SoundManager* mpSoundManager;      // BGMやSEの再生・制御を行うマネージャー
//	static ResourceManager* mpResourceManager; // 画像・モデル・音声などのアセットを一括管理するマネージャー
//	static Camera* mpCamera;                  // 描画の基準となるメインカメラの制御
//
//	// メモリ節約のため、一度読み込んだモデルを使い回すためのハンドル
//	static int mnSkyModelHandle;        // SkyBox背景モデル
//	static int mnStageModelHandle;      // ステージの描画用モデル
//	static int mnStageCollisionHandle;  // ステージの当たり判定用コリジョンモデル
//
//	// ステージの動きに合わせて共有するハンドル
//	static int mnSlideStageHandle;			  // 移動ステージ用
//	static int mnSlideStageCollHandle;         // 移動ステージ用当たり判定用コリジョンモデル
//	static int mnUpdownStageHandle;			  // 回転ステージ用
//	static int mnUpdownStageCollHandle;         // 回転ステージ用当たり判定用コリジョンモデル
//	static int mnRotaStageHandle;       // 微回転ステージ用
//	static int mnRotaStageCollHandle;   // 微回転ステージ用当たり判定用コリジョンモデル
//};