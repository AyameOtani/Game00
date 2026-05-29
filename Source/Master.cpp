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


int Master::mnDeleteEnemyCount = 0; // カウントの初期化