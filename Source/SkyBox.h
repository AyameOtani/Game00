#pragma once

class Model;

#include "Object3D.h"

class SkyBox : public Object3D
{
public:
	// ファイル名版（既存）
	SkyBox(std::string filename);
	// ハンドル版（既に読み込み済みのモデルを使う）
	SkyBox(int modelHandle);
	~SkyBox();

	void Update() override;
	void Draw() override;

	void SetScale(float scale);
	void SetModelTexture(std::string filename, int index = 0);

private:
	Model* mpModel;
	float mfRotation;
	bool m_ownsModel;
};