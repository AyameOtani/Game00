#include "LoadingScene.h"
#include "DxLib.h"
#include "Master.h"
#include "SceneManager.h"
#include "Utility.h"

// ==========================
// ロードタスク定義
// ==========================
struct LoadTask
{
    ModelSet* target;
    const char* modelPath;
    const char* collPath;
};

// ==========================
// ロード対象リスト
// 順次ロードを行うため、依存関係がある場合は順序を考慮すること
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
    // 進行表示用の背景画像を読み込み
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
    // ロード画面が瞬時に消えるのを防ぐための最小表示フレーム数
    mMinWaitFrame = 30;
}

void LoadingScene::Update()
{
    const int taskCount = sizeof(tasks) / sizeof(tasks[0]);

    // 全ロード完了後の待機処理
    if (mnProgressCounter >= taskCount)
    {
        // 読み込みが高速な環境でも一定時間表示させる
        if (mMinWaitFrame > 0)
        {
            mMinWaitFrame--;
            return;
        }

        Master::mpSceneManager->SetNextScene(SceneManager::SCENE_TYPE::SCENE_3D);
        return;
    }

    LoadTask& t = tasks[mnProgressCounter];

    // ==== モデル読み込み ====
    if (t.target->model == -1)
    {
        t.target->model = MV1LoadModel(t.modelPath);
    }

    // ==== コリジョン読み込み ====
    // 物理演算による衝突判定が必要なモデルのみ構築
    if (t.collPath && t.target->collision == -1)
    {
        t.target->collision = MV1LoadModel(t.collPath);

        if (t.target->collision != -1)
        {
            // 衝突判定情報（ヒットボックス）の事前構築
            MV1SetupCollInfo(t.target->collision, -1);
        }
    }

    mnProgressCounter++;
}

void LoadingScene::Draw()
{
    // 2D描画のためZバッファを一時無効化
    SetUseZBufferFlag(FALSE);
    SetWriteZBufferFlag(FALSE);

    DrawGraph(0, 0, mnBagHandle, TRUE);

    int frameWidth = 1020;
    int frameHeight = 520;

    // UI背景（影）
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

    // UI本体
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

    // 進捗バーの背景（未完了部分）
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

    // 進捗ゲージ本体
    if (fillWidth > 0)
    {
        if (fillWidth > barWidth) fillWidth = barWidth;

        // 進行度に応じて色が変化する演出
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

    // アンチエイリアスによる縁取り描画
    BeginAADraw();

    DrawRoundRectAA(
        (float)startX,
        (float)startY,
        (float)(startX + barWidth),
        (float)(startY + barHeight),
        (float)radius,
        (float)radius,
        16, // 分割数：円の滑らかさ
        GetColor(255, 255, 255),
        FALSE, // 枠線のみ描画
        1.5f   // 線の太さ
    );

    // 3D描画用設定へ復帰
    SetUseZBufferFlag(TRUE);
    SetWriteZBufferFlag(TRUE);
}

void LoadingScene::Finalize()
{
}