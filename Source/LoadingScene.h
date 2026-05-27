#pragma once
#include "Scene.h"

// 単純ローディングシーン：必要なモデルを一度だけ読み込み、次シーンをセットする
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
	bool mbLoaded;
	int mnProgressCounter;

	int mMinWaitFrame; // 最低表示フレーム数
};
