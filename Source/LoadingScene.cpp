#include "LoadingScene.h"
#include "DxLib.h"
#include "Master.h"
#include "SceneManager.h"
#include "Utility.h"

// ロード対象データ（モデル＋コリジョン設定）
struct LoadTask
{
    int* handle;        // 読み込み先ハンドル
    const char* path;   // モデルパス
    bool needColl;      // コリジョン生成するか
};

// ロード順リスト
static LoadTask tasks[] =
{
    { &Master::mnSkyModelHandle, "Resource/3D/SkyBox/sky.mqo", false },

    { &Master::mnStageModelHandle, "Resource/3D/Stage1/stage.mqo", false },
    { &Master::mnStageCollisionHandle, "Resource/3D/Stage1/stage.mqo", true },

    { &Master::mnSlideStageHandle, "Resource/3D/Stage1/slideStage.mqo", false },
    { &Master::mnSlideStageCollHandle, "Resource/3D/Stage1/slideStage.mqo", true },

    { &Master::mnUpdownStageHandle, "Resource/3D/Stage1/updownStage.mqo", false },
    { &Master::mnUpdownStageCollHandle, "Resource/3D/Stage1/updownStage.mqo", true },

    { &Master::mnRotaStageHandle, "Resource/3D/Stage1/rotaStage.mqo", false },
    { &Master::mnRotaStageCollHandle, "Resource/3D/Stage1/rotaStage.mqo", true },
};

LoadingScene::LoadingScene()
    : mnProgressCounter(0)
    , mMinWaitFrame(30)
{
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

    // 全ロード完了後、少し待ってからシーン遷移
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

    // 現在のロード対象
    LoadTask& t = tasks[mnProgressCounter];

    // 未ロードなら読み込み
    if (*t.handle == -1)
    {
        *t.handle = MV1LoadModel(t.path);

        // コリジョン設定が必要なら生成
        if (t.needColl && *t.handle != -1)
        {
            MV1SetupCollInfo(*t.handle, -1);
        }
    }

    mnProgressCounter++;
}

void LoadingScene::Draw()
{
    SetUseZBufferFlag(FALSE);
    SetWriteZBufferFlag(FALSE);

    const char* text = "LOADING...";
    int width = GetDrawStringWidth(text, (int)strlen(text));
    DrawFormatString(
        Utility::SCREEN_WIDTH / 2 - width / 2,
        Utility::SCREEN_HEIGHT / 2 - 120,
        GetColor(255, 255, 255),
        "%s",
        text
    );

    const int barWidth = 600;
    const int barHeight = 40;
    const int radius = 20;

    int startX = Utility::SCREEN_WIDTH / 2 - barWidth / 2;
    int startY = Utility::SCREEN_HEIGHT / 2;

    float progress = (float)mnProgressCounter /
        (float)(sizeof(tasks) / sizeof(tasks[0]));

    if (progress > 1.0f) progress = 1.0f;

    int fillWidth = (int)(barWidth * progress);

    // 背景バー（角丸）
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

    // 進捗バー（角丸対応・はみ出し防止）
    if (fillWidth > 0)
    {
        // 角丸のせいで端が飛び出すのを防ぐため制限
        int safeWidth = fillWidth;

        if (safeWidth > barWidth) safeWidth = barWidth;

        DrawRoundRect(
            startX,
            startY,
            startX + safeWidth,
            startY + barHeight,
            radius,
            radius,
            GetColor(
                255,
                (int)(60 + 140 * progress),
                0
            ),
            TRUE
        );
    }

    // 枠線（アンチエイリアス）
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