#include "Master.h"
#include "SoundManager.h"
#include "ResourceManager.h"
#include "Camera.h"
#include "InputManager.h"

// ==== システム系 ====
//SceneManager* Master::mpSceneManager = new SceneManager();
//SoundManager* Master::mpSoundManager = new SoundManager();
//Camera* Master::mpCamera = new Camera();
//ResourceManager* Master::mpResourceManager = new ResourceManager();


// ==== モデルセットの実体定義 ====

ModelSet Master::sky;
ModelSet Master::stage;
ModelSet Master::slideStage;
ModelSet Master::updownStage;
ModelSet Master::rotaStage;


// ==== パス定義====
static const char* kSkyModelPath = "Resource/3D/SkyBox/sky.mqo";

static const char* kStageModelPath = "Resource/3D/Stage1/stage.mqo";
static const char* kStageCollisionPath = "Resource/3D/Stage1/stage.mqo";



//#include "Master.h"
//#include "SoundManager.h"
//#include "ResourceManager.h"
//#include "Camera.h"
//#include "InputManager.h"
//
////// Master クラスの静的メンバ変数定義
////SceneManager* Master::mpSceneManager = new SceneManager();
////SoundManager* Master::mpSoundManager = new SoundManager();
////Camera* Master::mpCamera = new Camera();
////ResourceManager* Master::mpResourceManager = new ResourceManager();
//
//// 追加：共有モデルハンドルの初期化（未ロードは -1）
//int Master::mnSkyModelHandle = -1;
//int Master::mnStageModelHandle = -1;
//int Master::mnStageCollisionHandle = -1;
//
//
//int Master::mnSlideStageHandle = -1;
//int Master::mnSlideStageCollHandle = -1;
//int Master::mnUpdownStageHandle = -1;
//int Master::mnUpdownStageCollHandle = -1;
//int Master::mnRotaStageHandle = -1;
//int Master::mnRotaStageCollHandle = -1;
//
//
//
//
//// ロード対象ファイル
//static const char* kSkyModelPath = "Resource/3D/SkyBox/sky.mqo";
//
//static const char* kStageModelPath = "Resource/3D/Stage1/stage.mqo";
//static const char* kStageCollisionPath = "Resource/3D/Stage1/stage.mqo";
//
//static const char* kSlideStagePath = "Resource/3D/Stage1/slideStage.mqo";
//static const char* kSlideStageCollPath = "Resource/3D/Stage1/slideStage.mqo";
//
//static const char* kUpdownStagePath = "Resource/3D/Stage1/rotaStage.mqo";
//static const char* kUpdownStageCollPath = "Resource/3D/Stage1/rotaStage.mqo";
//
//static const char* kRotaStagePath = "Resource/3D/Stage1/littleRota.mqo";
//static const char* kRotaStageCollPath = "Resource/3D/Stage1/littleRota.mqo";
//