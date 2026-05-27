#include "Master.h"
#include "SoundManager.h"
#include "ResourceManager.h"
#include "Camera.h"
#include "InputManager.h"

//// Master クラスの静的メンバ変数定義
//SceneManager* Master::mpSceneManager = new SceneManager();
//SoundManager* Master::mpSoundManager = new SoundManager();
//Camera* Master::mpCamera = new Camera();
//ResourceManager* Master::mpResourceManager = new ResourceManager();

// 追加：共有モデルハンドルの初期化（未ロードは -1）
int Master::mnSkyModelHandle = -1;
int Master::mnStageModelHandle = -1;
int Master::mnStageCollisionHandle = -1;


int Master::mnStageMoveHandle = -1;
int Master::mnStageMoveCollHandle = -1;
int Master::mnStageRotaHandle = -1;
int Master::mnStageRotaCollHandle = -1;
int Master::mnStageLittleRotaHandle = -1;
int Master::mnStageLittleCollRotaHandle = -1;





