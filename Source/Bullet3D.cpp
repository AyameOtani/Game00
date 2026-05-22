#include "Bullet3D.h"
#include "Model.h"
#include "Master.h"
#include "SceneManager.h"
#include "Scene.h"
#include "ObjectManager.h"
#include "Stage.h"
#include "Character3D.h"

/**
 * @brief コンストラクタ（安全化）
 */
Bullet3D::Bullet3D(VECTOR initPos, std::string filename, VECTOR Direction, Team shooterTeam)
    : Object3D(initPos)
    , m_shooterTeam(shooterTeam)
    , mfMoveSpeed(0.0f)
    , mEffectDistance(0.0f)
{
    // 初期座標を明示的に設定（Object3D の実装差異に依存しないように）
    mvPosition = initPos;

    // 方向ベクトルがゼロの場合はフォワード(0,0,1)を使う
    if (VSize(Direction) > 0.0001f)
    {
        mvDirection = VNorm(Direction);
    }
    else
    {
        mvDirection = VGet(0.0f, 0.0f, 1.0f);
    }

    mpModel = new Model(filename, initPos);

    // モデルの位置を初期位置に反映
    if (mpModel)
    {
        mpModel->SetPosition(mvPosition);
    }

    // 互換タグ（存在するなら）
    SetTag(Object3D::T_Bullet3D);
}

/**
 * @brief デストラクタ
 */
Bullet3D::~Bullet3D()
{
    if (mpModel) delete mpModel;
}

/**
 * @brief 更新処理
 */
void Bullet3D::Update()
{
    // 方向が不正なら補正（万が一の保険）
    if (VSize(mvDirection) <= 0.0001f)
    {
        mvDirection = VGet(0.0f, 0.0f, 1.0f);
    }

    Move();
    HitStage();
    HitCharacter();

    if (mpModel)
    {
        mpModel->SetPosition(mvPosition);
        mpModel->Update();
    }

    Object3D::Update();
}

void Bullet3D::Draw()
{
    if (mpModel)
    {
        mpModel->SetPosition(mvPosition);
        mpModel->Draw();
    }
    Object3D::Draw();
}

/**
 * @brief 移動処理（簡潔・安全）
 */
void Bullet3D::Move()
{
    // フレーム毎の移動。必要ならフレームレート依存を外す（dt を掛ける）方向に改良してください。
    mvPosition = VAdd(mvPosition, VScale(mvDirection, mfSpeed));

    // 移動距離を積算
    mfMoveSpeed += mfSpeed;
    if (mfMoveSpeed >= 2000.0f)
    {
        SetDeleteFlag(true);
        return;
    }

    if (mpModel) mpModel->SetPosition(mvPosition);
}

/**
 * @brief 壁判定
 */
void Bullet3D::HitStage()
{
    auto stageList = Master::mpSceneManager->GetCurrentScene()
        ->GetObjectManager()->GetObject3DListByTag(Object3D::T_Stage3D);

    for (auto obj : stageList)
    {
        Stage* pStage = dynamic_cast<Stage*>(obj);
        if (!pStage) continue;

        VECTOR hitPos = VGet(0, 0, 0);
        VECTOR hitNormal = VGet(0, 0, 0);

        if (pStage->CheckHit_Capsule_Wall(mvPosition, mvPosition, m_radius, hitPos, hitNormal))
        {
            // 衝突処理（SEやエフェクト）
            SetDeleteFlag(true);
            return;
        }
    }
}

/**
 * @brief キャラクター当たり判定（簡素化）
 */
void Bullet3D::HitCharacter()
{
    // プレイヤーと敵を走査（チーム判定でスキップ）
    std::vector<Object3D::Tag3D> targets = { Object3D::T_Player3D, Object3D::T_Enemy3D };
    for (auto tag : targets)
    {
        auto list = Master::mpSceneManager->GetCurrentScene()->GetObjectManager()->GetObject3DListByTag(tag);
        for (auto obj : list)
        {
            Character3D* pChar = dynamic_cast<Character3D*>(obj);
            if (!pChar) continue;
            if (pChar->GetTeam() == m_shooterTeam) continue;

            if (HitCheck_Sphere_Sphere(mvPosition, m_radius, pChar->GetPosition(), pChar->GetRadius()))
            {
                pChar->TakeDamage(10);
                SetDeleteFlag(true);
                return;
            }
        }
    }
}