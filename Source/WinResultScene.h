#pragma once
#include "Scene.h"

// リザルト（結果）画面クラス
// ゲームクリアやゲームオーバーの表示を行います
class WinResultScene : public Scene
{
public:
    WinResultScene();
    ~WinResultScene();

    void Initialize() override;  // 初期化
    void Update() override;      // 更新
    void Draw() override;        // 描画
    void Finalize() override;    // 終了
};






