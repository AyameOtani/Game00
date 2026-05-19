#include "Stage.h"
#include "Model.h"
#include "Master.h"

Stage::Stage(std::string stageModelName, std::string stageCollisionModelName)
	: Object3D(VGet(0.0f, 0.0f, 0.0f)) // 座標は原点としておく
	, mnModelHandle(-1)
	, mnCollisionHandle(-1)

{
	// タグ設定
	SetTag(Object3D::T_Stage3D);
	
	// ステージモデルの読み込み
	mnModelHandle = MV1LoadModel(stageModelName.c_str());

	// コリジョンモデル（当たり判定用のモデル）の読みこみ
	mnCollisionHandle = MV1LoadModel(stageCollisionModelName.c_str());

	// 当たり判定情報の作成 コリジョンので作ってくれている
	// 自動的にデータが作成される便利な関数
	MV1SetupCollInfo(mnCollisionHandle, -1);
}



//Stage::Stage(int modelhandel, int collisionHandle)
//	: Object3D(VGet(0.0f, 0.0f, 0.0f)) // 座標は原点としておく
//	, mnModelHandle(modelhandel)
//	, mnCollisionHandle(collisionHandle)
//{
//	// タグ設定
//	SetTag(Object3D::T_Stage3D);
//	// 当たり判定情報の作成 ハンドルごとに必要
//	if (mnCollisionHandle != -1)
//	{
//		MV1SetupCollInfo(mnCollisionHandle, -1);
//	}
//	// 当たり判定情報の作成 コリジョンので作ってくれている
//	// 自動的にデータが作成される便利な関数
//	// MV1SetupCollInfo(mnCollisionHandle, -1);
//}

Stage::~Stage()
{
	// 読み込んだモデルの破棄
	// ぉーディングでやるからいらぬ
	// MV1DeleteModel(mnModelHandle);
	// MV1DeleteModel(mnCollisionHandle);
}

void Stage::Update()
{
	TitleRotate();

	MV1SetPosition(mnModelHandle, mvPosition);
	MV1SetPosition(mnCollisionHandle, mvPosition);

	MV1SetRotationXYZ(mnModelHandle, mvRotation);
	MV1SetRotationXYZ(mnCollisionHandle, mvRotation);

}

void Stage::Draw()
{
	// 描画
	MV1DrawModel(mnModelHandle);

	// デバッグ表示
	// MV1DrawModelDebug(mnCollisionHandle, GetColor(255,255,255), 1, 10, 1, 0);
}


// これを、bool,じゃなくて、boolとVECTORを返せるような構造体にする。
//  boolで当たっていたら、TRUEで、帰ってきたVECTORで何かするとか
// 今の状況でも、ポジションは取れているから、そこをどうにかする
//そして、当たっているポジションを引っぱり出せる用にするとか

// 出来なかったら、Rayを４本

bool Stage::CheckHit_Capsule(VECTOR pos1, VECTOR pos2, float r)
{
	// 生成していた当たり判定をもとにカプセルとの当たり判定を行う
	// コリジョン結果代入用ポリゴン配列
	MV1_COLL_RESULT_POLY_DIM result = MV1CollCheck_Capsule(mnCollisionHandle, -1, pos1, pos2, r);

	// ポリゴンに一つ以上当たっている場合
	if (result.HitNum >= 1)
	{
		// 回数を当たった回数を回す
		for (int i = 0; i < result.HitNum; i++)
		{
			//// ３Dの三角形を描画する
			//DrawTriangle3D(
			//	result.Dim[i].Position[0],
			//	result.Dim[i].Position[1],
			//	result.Dim[i].Position[2],
			//	GetColor(255, 0, 0),
			//	0
			//);
		}

	}
	// 当たり判定情報の後片付け
	MV1CollResultPolyDimTerminate(result);
	return result.HitNum >= 1;
}

VECTOR Stage::CheckHit_Line(VECTOR pos1, VECTOR pos2)
{
	VECTOR ret = VGet(0.0f, 0.0f, 0.0f);

	// あたり判定情報と線分とのあたり判定を行う
	//MV1_COLL_RESULT_POLY_DIM result = MV1CollCheck_LineDim(mnCollisionHandle, -1, pos1, pos2); // ★★
	auto result = MV1CollCheck_Line(mnCollisionHandle, -1, pos1, pos2);

	//// 当たっていた場合   // ★★
	//if (result.HitNum >= 1)
	//{
	//	// 当たった個数のポジションをreturnするように取得する
	//	// 壁のときみたいにHitPositionをとる必要がない
	//	ret = result.Dim[0].HitPosition;
	//}

    // 当たっていた場合
	if (result.HitFlag >= 1)
	{
		// 当たった個数のポジションをreturnするように取得する
		// 壁のときみたいにHitPositionをとる必要がない
		ret = result.HitPosition;
	}
    // MV1CollCheck_Line の戻り値は明示的な解放関数を必要としないため後片付けは不要
	//ret = pos2;

	return ret;
}

int Stage::GetModelHandle() const
{
	return mnCollisionHandle; // 3Dモデルのハンドルを保存
}



// 壁沿いベクトルの処理 11/4
// カプセルの両端の座標、半径R、衝突点（戻り値）、衝突法線（戻り値）
bool Stage::CheckHit_Capsule_Wall(VECTOR pos1, VECTOR pos2, float r, VECTOR& hitPos, VECTOR& hitNormal)
{
	// 当たり判定
	// MV1_COLL_RESULT_POLY_DIM...ポリゴンの情報を格納する構造体
	MV1_COLL_RESULT_POLY_DIM result = MV1CollCheck_Capsule(mnCollisionHandle, -1, pos1, pos2, r);

	bool isHit = false; //当たっているかのフラグをfalse

	if (result.HitNum > 0) // ヒットした回数が０以上だったら（当たっていたら）
	{
        int nearIndex = 0; // いちばん近いヒット数を入れる変数
		float nearDistance = 999999.0f; // 最小距離の保持の為の変数

        // 当たった三角形の数だけ調べる
		for (int i = 0; i < result.HitNum; i++)
		{
			// MV1CollCheck_Capsule の結果には各ポリゴンに対する HitPosition が含まれるはず
			// そこを基準に最も近いポリゴンを選ぶ
			VECTOR hitPosPoly = result.Dim[i].HitPosition;
			float distance = VSize(VSub(hitPosPoly, pos1));
			if (distance < nearDistance)
			{
				nearDistance = distance;
				nearIndex = i;
			}

			// デバッグ用の三角形の描画
			DrawTriangle3D(
				result.Dim[i].Position[0],
				result.Dim[i].Position[1],
				result.Dim[i].Position[2],
				GetColor(0, 255, 0), // 緑
				false
			);
		}


		// 衝突点と法線の設定
		// 複数ポリゴンにヒットしている場合、全ヒットの HitPosition と Normal を平均して返す
		VECTOR avgHitPos = VGet(0.0f, 0.0f, 0.0f);
		VECTOR avgNormal = VGet(0.0f, 0.0f, 0.0f);
		for (int i = 0; i < result.HitNum; ++i)
		{
			avgHitPos = VAdd(avgHitPos, result.Dim[i].HitPosition);
			avgNormal = VAdd(avgNormal, result.Dim[i].Normal);
		}
		avgHitPos = VScale(avgHitPos, 1.0f / (float)result.HitNum);
		avgNormal = VNorm(avgNormal);

		// 返す値
		hitPos = avgHitPos;
		hitNormal = avgNormal;

		isHit = true; // フラグをtrueにしている
	}

	MV1CollResultPolyDimTerminate(result); // 当たり判定結果ポリゴン配列の後始末をしている
	return isHit; // 衝突があったかを教えて、＆の所に結果を返している
}

void Stage::TitleRotate()
{
	if (Master::mpSceneManager->GetCurrentSceneType() == SceneManager::SCENE_TYPE::TITLE_3D)
	{
		// 回転するようにした　1210
		mfRotation += 0.0005f; // ここで回転速度
		if (mfRotation > DX_TWO_PI_F) // 360を越したら
		{
			mfRotation -= DX_TWO_PI_F; // 今の回転角から３６０分引く
		}
		mvRotation.y = mfRotation;
		//MV1SetRotationXYZ(mnModelHandle, mvRotation);


		// ------フォグ設定 ------
		SetFogEnable(TRUE);// フォグ有効
		SetFogStartEnd(000.0f, 15000.0f);// フォグが始まる距離と終了する距離を設定する
		SetFogColor(255, 200, 180); // 色
	}
}

// 線分が当たったらtrueを返し、hitNormalに床の傾きを格納する
bool Stage::CheckHit_Line_Normal(VECTOR pos1, VECTOR pos2, VECTOR& hitNormal)
{
	// 一度普通にDxLibの線分判定を呼ぶ
	auto result = MV1CollCheck_Line(mnCollisionHandle, -1, pos1, pos2);

	// 当たっていたら
	if (result.HitFlag >= 1)
	{
		hitNormal = result.Normal; // 床の法線（向き）を代入
		return true;
	}

	return false; // 当たっていなければfalse
}