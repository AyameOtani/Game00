#pragma once
#include "Scene.h"
#include <vector>
#include "DxLib.h"

class Enemy3D;


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


private:
    int m_selectedId = 0; // 今操作対象にしている敵のID
    std::vector<Enemy3D*> m_enemyList; // 敵の管理用
private:
    std::vector<EnemyData> m_savedEnemyList; // ファイル保存用のリスト
};






