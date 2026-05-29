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

	static int mnDeleteEnemyCount; // 敵のカウント
};