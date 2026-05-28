#pragma once
#include "Scene.h"
#include <vector>
#include "DxLib.h"
#include "Enemy3D.h"

struct EnemyData
{
    int id;
    VECTOR pos;
};

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

    void SaveEnemyDataToFile();

    //int GetEnemyTypeFromID(int id);
    Enemy3D::EnemyType GetEnemyTypeFromID(int id);

private:
    int m_selectedId = 0; // 今操作対象にしている敵のID
    std::vector<Enemy3D*> m_enemyList; // 敵の管理用

    std::vector<VECTOR> m_recordedEnemyPositions; // 記録用リスト

    std::vector<EnemyData> m_savedEnemyList; // ファイル保存用のリスト

private:
	int mnGoalHandle = -1; // 画像ハンドル ゴールの画像
	float mfGoalRadius = 150.0f; // ゴールの当たり判定用半径
	VECTOR mvGoalPosition; // ゴールの座標
};






