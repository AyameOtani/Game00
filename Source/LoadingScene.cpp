#include "LoadingScene.h"
#include "DxLib.h"
#include "Master.h"
#include "SceneManager.h"
#include "Utility.h"

// ==========================
// ロードタスク
// ==========================
struct LoadTask
{
    ModelSet* target;
    const char* modelPath;
    const char* collPath;
};

// ==========================
// ロード一覧
// ==========================
static LoadTask tasks[] =
{
    { &Master::sky,
      "Resource/3D/SkyBox/sky.mqo",
      nullptr },

    { &Master::stage,
      "Resource/3D/Stage1/stage.mqo",
      "Resource/3D/Stage1/stage.mqo" },

    { &Master::slideStage,
      "Resource/3D/Stage1/slideStage.mqo",
      "Resource/3D/Stage1/slideStage.mqo" },

    { &Master::updownStage,
      "Resource/3D/Stage1/updownStage.mqo",
      "Resource/3D/Stage1/updownStage.mqo" },

    { &Master::rotaStage,
      "Resource/3D/Stage1/rotaStage.mqo",
      "Resource/3D/Stage1/rotaStage.mqo" },
};

LoadingScene::LoadingScene()
    : mnProgressCounter(0)
    , mMinWaitFrame(30)
{
    mnBagHandle = LoadGraph("Resource/2D/TitleBag.png");
    if (mnBagHandle == -1) printfDx("画像がありません\n");

    SetFontSize(100);
}

LoadingScene::~LoadingScene()
{
}

void LoadingScene::Initialize()
{
    mnProgressCounter = 0;
    mMinWaitFrame = 30;
}

void LoadingScene::Update()
{
    const int taskCount = sizeof(tasks) / sizeof(tasks[0]);

    // 全ロード完了
    if (mnProgressCounter >= taskCount)
    {
        if (mMinWaitFrame > 0)
        {
            mMinWaitFrame--;
            return;
        }

        Master::mpSceneManager->SetNextScene(SceneManager::SCENE_TYPE::SCENE_3D);
        return;
    }

    LoadTask& t = tasks[mnProgressCounter];

    // ==== モデル ====
    if (t.target->model == -1)
    {
        t.target->model = MV1LoadModel(t.modelPath);
    }

    // ==== コリジョン ====
    if (t.collPath && t.target->collision == -1)
    {
        t.target->collision = MV1LoadModel(t.collPath);

        if (t.target->collision != -1)
        {
            MV1SetupCollInfo(t.target->collision, -1);
        }
    }

    mnProgressCounter++;
}

void LoadingScene::Draw()
{
    SetUseZBufferFlag(FALSE);
    SetWriteZBufferFlag(FALSE);

    DrawGraph(0, 0, mnBagHandle, TRUE);

    int frameWidth = 1020;
    int frameHeight = 520;

    DrawBox(
        Utility::SCREEN_WIDTH / 2 - frameWidth / 2,
        Utility::SCREEN_HEIGHT / 2 - frameHeight / 2,
        Utility::SCREEN_WIDTH / 2 + frameWidth / 2,
        Utility::SCREEN_HEIGHT / 2 + frameHeight / 2,
        GetColor(139, 69, 19),
        TRUE
    );

    int boxWidth = 1000;
    int boxHeight = 500;

    DrawBox(
        Utility::SCREEN_WIDTH / 2 - boxWidth / 2,
        Utility::SCREEN_HEIGHT / 2 - boxHeight / 2,
        Utility::SCREEN_WIDTH / 2 + boxWidth / 2,
        Utility::SCREEN_HEIGHT / 2 + boxHeight / 2,
        GetColor(227, 190, 152),
        TRUE
    );

    const char* text = "LOADING...";
    int width = GetDrawStringWidth(text, (int)strlen(text));

    DrawFormatString(
        Utility::SCREEN_WIDTH / 2 - width / 2,
        Utility::SCREEN_HEIGHT / 2 - 120,
        GetColor(255, 255, 255),
        "%s",
        text
    );

    const int barWidth = 700;
    const int barHeight = 50;
    const int radius = 20;

    int startX = Utility::SCREEN_WIDTH / 2 - barWidth / 2;
    int startY = Utility::SCREEN_HEIGHT / 2;

    float progress = (float)mnProgressCounter /
        (float)(sizeof(tasks) / sizeof(tasks[0]));

    if (progress > 1.0f) progress = 1.0f;

    int fillWidth = (int)(barWidth * progress);

    DrawRoundRect(
        startX,
        startY,
        startX + barWidth,
        startY + barHeight,
        radius,
        radius,
        GetColor(40, 40, 40),
        TRUE
    );

    if (fillWidth > 0)
    {
        if (fillWidth > barWidth) fillWidth = barWidth;

        DrawRoundRect(
            startX,
            startY,
            startX + fillWidth,
            startY + barHeight,
            radius,
            radius,
            GetColor(255, (int)(60 + 140 * progress), 0),
            TRUE
        );
    }

    BeginAADraw();

    DrawRoundRectAA(
        (float)startX,
        (float)startY,
        (float)(startX + barWidth),
        (float)(startY + barHeight),
        (float)radius,
        (float)radius,
        16,
        GetColor(255, 255, 255),
        FALSE,
        1.5f
    );

    SetUseZBufferFlag(TRUE);
    SetWriteZBufferFlag(TRUE);
}

void LoadingScene::Finalize()
{
}



//#include "LoadingScene.h"
//#include "DxLib.h"
//#include "Master.h"
//#include "SceneManager.h"
//#include "Utility.h"
//
//// ロード対象データ（モデル＋コリジョン設定）
//struct LoadTask
//{
//    int* handle;        // 読み込み先ハンドル
//    const char* path;   // モデルパス
//    bool needColl;      // コリジョン生成するか
//};
//
//// ロード順リスト
//static LoadTask tasks[] =
//{
//    { &Master::mnSkyModelHandle, "Resource/3D/SkyBox/sky.mqo", false },
//
//    { &Master::mnStageModelHandle, "Resource/3D/Stage1/stage.mqo", false },
//    { &Master::mnStageCollisionHandle, "Resource/3D/Stage1/stage.mqo", true },
//
//    { &Master::mnSlideStageHandle, "Resource/3D/Stage1/slideStage.mqo", false },
//    { &Master::mnSlideStageCollHandle, "Resource/3D/Stage1/slideStage.mqo", true },
//
//    { &Master::mnUpdownStageHandle, "Resource/3D/Stage1/updownStage.mqo", false },
//    { &Master::mnUpdownStageCollHandle, "Resource/3D/Stage1/updownStage.mqo", true },
//
//    { &Master::mnRotaStageHandle, "Resource/3D/Stage1/rotaStage.mqo", false },
//    { &Master::mnRotaStageCollHandle, "Resource/3D/Stage1/rotaStage.mqo", true },
//};
//
//LoadingScene::LoadingScene()
//    : mnProgressCounter(0)
//    , mMinWaitFrame(30)
//{
//	mnBagHandle = LoadGraph("Resource/2D/TitleBag.png");
//	if (mnBagHandle == -1) printfDx("画像ない");
//
//    SetFontSize(100);
//}
//
//LoadingScene::~LoadingScene()
//{
//}
//
//void LoadingScene::Initialize()
//{
//    mnProgressCounter = 0;
//    mMinWaitFrame = 30;
//}
//
//void LoadingScene::Update()
//{
//    const int taskCount = sizeof(tasks) / sizeof(tasks[0]);
//
//    // 全ロード完了後、少し待ってからシーン遷移
//    if (mnProgressCounter >= taskCount)
//    {
//        if (mMinWaitFrame > 0)
//        {
//            mMinWaitFrame--;
//            return;
//        }
//
//        Master::mpSceneManager->SetNextScene(SceneManager::SCENE_TYPE::SCENE_3D);
//        return;
//    }
//
//    // 現在のロード対象
//    LoadTask& t = tasks[mnProgressCounter];
//
//    // 未ロードなら読み込み
//    if (*t.handle == -1)
//    {
//        *t.handle = MV1LoadModel(t.path);
//
//        // コリジョン設定が必要なら生成
//        if (t.needColl && *t.handle != -1)
//        {
//            MV1SetupCollInfo(*t.handle, -1);
//        }
//    }
//
//    mnProgressCounter++;
//}
//
//void LoadingScene::Draw()
//{
//    SetUseZBufferFlag(FALSE);
//    SetWriteZBufferFlag(FALSE);
//
//    // 背景の描画
//    DrawGraph(0, 0, mnBagHandle, TRUE);
//
//    // 濃い茶色の枠用ボックス
//    int frameWidth = 1020;
//    int frameHeight = 520;
//    DrawBox(
//        Utility::SCREEN_WIDTH / 2 - frameWidth / 2,
//        Utility::SCREEN_HEIGHT / 2 - frameHeight / 2,
//        Utility::SCREEN_WIDTH / 2 + frameWidth / 2,
//        Utility::SCREEN_HEIGHT / 2 + frameHeight / 2,
//        GetColor(139, 69, 19), // 濃い茶色
//        TRUE
//    );
//    // 先ほどのボックスを重ねて描画
//    int boxWidth = 1000;
//    int boxHeight = 500;
//
//    DrawBox(
//        Utility::SCREEN_WIDTH / 2 - boxWidth / 2,
//        Utility::SCREEN_HEIGHT / 2 - boxHeight / 2,
//        Utility::SCREEN_WIDTH / 2 + boxWidth / 2,
//        Utility::SCREEN_HEIGHT / 2 + boxHeight / 2,
//        GetColor(227, 190, 152),
//        TRUE
//    );
//
//
//
//    const char* text = "LOADING...";
//    int width = GetDrawStringWidth(text, (int)strlen(text));
//
//    DrawFormatString(
//        Utility::SCREEN_WIDTH / 2 - width / 2,
//        Utility::SCREEN_HEIGHT / 2 - 120,
//        GetColor(255, 255, 255),
//        "%s",
//        text
//    );
//
//    // バーの設定
//	const int barWidth = 700; // バーの全幅
//	const int barHeight = 50; // バーの高さ
//	const int radius = 20; // 角丸の半径
//
//    int startX = Utility::SCREEN_WIDTH / 2 - barWidth / 2;
//    int startY = Utility::SCREEN_HEIGHT / 2;
//
//    float progress = (float)mnProgressCounter /
//        (float)(sizeof(tasks) / sizeof(tasks[0]));
//
//    if (progress > 1.0f) progress = 1.0f;
//
//    int fillWidth = (int)(barWidth * progress);
//
//    // 背景バー（角丸）
//    DrawRoundRect(
//        startX,
//        startY,
//        startX + barWidth,
//        startY + barHeight,
//        radius,
//        radius,
//        GetColor(40, 40, 40),
//        TRUE
//    );
//
//    // 進捗バー（角丸対応・はみ出し防止）
//    if (fillWidth > 0)
//    {
//        // 角丸のせいで端が飛び出すのを防ぐため制限
//        int safeWidth = fillWidth;
//
//        if (safeWidth > barWidth) safeWidth = barWidth;
//
//        DrawRoundRect(
//            startX,
//            startY,
//            startX + safeWidth,
//            startY + barHeight,
//            radius,
//            radius,
//            GetColor(
//                255,
//                (int)(60 + 140 * progress),
//                0
//            ),
//            TRUE
//        );
//    }
//
//    // 枠線（アンチエイリアス）
//    BeginAADraw();
//
//    DrawRoundRectAA(
//        (float)startX,
//        (float)startY,
//        (float)(startX + barWidth),
//        (float)(startY + barHeight),
//        (float)radius,
//        (float)radius,
//        16,
//        GetColor(255, 255, 255),
//        FALSE,
//        1.5f
//    );
//
//    SetUseZBufferFlag(TRUE);
//    SetWriteZBufferFlag(TRUE);
//}
//
//void LoadingScene::Finalize()
//{
//}