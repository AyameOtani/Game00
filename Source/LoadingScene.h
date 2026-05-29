#pragma once
#include "Scene.h"

class LoadingScene : public Scene
{
public:
    LoadingScene();
    ~LoadingScene();

    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;

private:
    // ロード進行管理
    int mnProgressCounter = 0;   // どこまでロードしたか

    // 最低表示時間
    int mMinWaitFrame = 0;       // ロード後の待機時間

    int mnBagHandle = -1; // ロゴのハンドル
};