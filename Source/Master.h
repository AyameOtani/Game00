#pragma once

#include "SceneManager.h"
#include "SoundManager.h"
#include "ResourceManager.h"
#include "Camera.h"
#include <memory>

class Master
{
public: 
	static SceneManager* mpSceneManager;     // シーンマネージャーのポインタ
	static SoundManager* mpSoundManager;     // サウンドマネージャーのポインタ
	static ResourceManager* mpResourceManager; // リソースマネージャーのポインタ
	static Camera* mpCamera;                 // カメラのポインタ
};




