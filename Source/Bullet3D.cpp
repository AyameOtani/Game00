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
    // 方向が不正なら補正
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

    DrawSphere3D(mvPosition, m_radius, 8, GetColor(255, 255, 0), GetColor(255, 255, 0), false);

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
    // 弾の判定用半径（Drawのデバッグ球とサイズを合わせてね）
    float bulletRadius = 10.0f;

    // ゲーム内のステージオブジェクトのリストを取得する
    auto stageList = Master::mpSceneManager->GetCurrentScene()
        ->GetObjectManager()->GetObject3DListByTag(Object3D::T_Stage3D);

    // ステージのリストを一つずつループして、弾と当たっているか調べる
    for (auto obj : stageList)
    {
        Stage* pStage = dynamic_cast<Stage*>(obj);
        if (!pStage) continue;

        // 衝突点と法線を受け取るための変数（関数に渡すために必要）
        VECTOR hitPos = VGet(0.0f, 0.0f, 0.0f);
        VECTOR hitNormal = VGet(0.0f, 0.0f, 0.0f);

        // -------------------------------------------------------------------------
        // 【ポイント】始点と終点にどちらも「mvPosition」を渡す！
        // これでカプセル判定が、半径 bulletRadius の「球体判定」として動きます！
        // -------------------------------------------------------------------------
        if (pStage->CheckHit_Capsule_Wall(mvPosition, mvPosition, bulletRadius, hitPos, hitNormal))
        {
            // ステージに当たったら、削除フラグを true にして弾を消去！
            SetDeleteFlag(true);
            break; // ループを抜ける
        }
    }
}

/**
 * @brief キャラクター当たり判定　弾と敵orプレイヤー
 */
void Bullet3D::HitCharacter()
{
    // プレイヤーと敵を走査
    std::vector<Object3D::Tag3D> targets =
    {
        Object3D::T_Player3D,
        Object3D::T_Enemy3D
    };

    for (auto tag : targets)
    {
        auto list = Master::mpSceneManager->GetCurrentScene()
            ->GetObjectManager()
            ->GetObject3DListByTag(tag);

        for (auto obj : list)
        {
            Character3D* pChar = dynamic_cast<Character3D*>(obj);
            if (!pChar) continue;
            if (pChar->GetTeam() == m_shooterTeam) continue;

            bool hit = false;

            //// ==============================
            ////  デバッグ表示（追加）
            //// ==============================
            //if (pChar->GetTeam() == Team::Enemy)
            //{
            //    DrawSphere3D(
            //        pChar->GetHitCenter(),
            //        pChar->GetRadius(),
            //        8,
            //        GetColor(255, 0, 255),
            //        GetColor(255, 0, 255),
            //        false
            //    );
            //}
            //else
            //{
            //    DrawCapsule3D(
            //        pChar->GetCapsuleBottom(),
            //        pChar->GetCapsuleTop(),
            //        pChar->GetRadius(),
            //        8,
            //        GetColor(255, 0, 255),
            //        GetColor(255, 0, 255),
            //        false
            //    );
            //}



            // =====================================================
            // 弾 vs キャラクター当たり判定
            // =====================================================

            // チームによって当たり判定の形状を切り替える
            // Enemy  → 球（シンプルな当たり判定）
            // Player → カプセル（体の形状に近い判定）
            if (pChar->GetTeam() == Team::Enemy)
            {
                // ==============================
                // 敵：球 vs 球
                // ==============================
                // 敵は単純な球体判定で処理（軽量・高速）
                hit = HitCheck_Sphere_Sphere(
                    mvPosition,                  // 弾の中心位置
                    m_radius,                    // 弾の半径
                    pChar->GetHitCenter(),       // 敵の中心位置
                    pChar->GetRadius()           // 敵の半径
                );
            }
            else
            {
                // ==============================
                // プレイヤー：球 vs カプセル
                // ==============================
                // プレイヤーは人体形状を想定してカプセル判定
                // 足元～頭部までの縦長判定にすることで違和感を減らす
                hit = HitCheck_Sphere_Capsule(
                    mvPosition,  // 弾の中心位置
                    m_radius,    // 弾の半径

                    // カプセル下端（足元）
                    pChar->GetCapsuleBottom(),

                    // カプセル上端（頭付近）
                    pChar->GetCapsuleTop(),
                    pChar->GetRadius() // カプセル半径（体の太さ）
                );
            }

            // ==============================
            // ヒット処理共通
            // ==============================
            if (hit)
            {
                pChar->TakeDamage(1);
                SetDeleteFlag(true);
                return;
            }
        }
    }
}