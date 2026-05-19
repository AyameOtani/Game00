#pragma once
#include "Scene.h"

// タイトル画面クラス
class GameScene : public Scene
{
public:

    GameScene();
    ~GameScene();

    void Initialize() override;  // 初期化
    void Update() override;      // 更新
    void Draw() override;        // 描画
    void Finalize() override;    // 終了
};






