#pragma once
#include "Scene.h"

// タイトル画面クラス
// BaseSceneを継承して、タイトル画面専用の処理を書きます
class TitleScene : public Scene
{
public:

    TitleScene();
    ~TitleScene();


    void Initialize() override;  // 初期化
    void Update() override;      // 更新
    void Draw() override;        // 描画
    void Finalize() override;    // 終了

private:
    int mnRogoHandle; // ロゴのハンドル

    // フォント
    int mnTitleFontHandle;
	int mnSmallFontHandle;

	int mBlinkCounter = 0; // 点滅のカウンター
	bool mShowText = true; // テキストを表示するかどうかのフラグ
};






