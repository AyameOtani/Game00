#include "AttachmentModel.h"
#include "Master.h"

AttachmentModel::AttachmentModel(std::string filename, int parentModelHandle, int parentFrameIndex)
	: Object3D(VGet(0.0f, 0.0f, 0.0f))
	, mnParentHandle(parentModelHandle)
	, mnParentFrameIndex(parentFrameIndex)
	, mvRotation(VGet(0.0f, 0.0f, 0.0f))
{
	// 3Dモデルの読み込み
	//mnHandle = MV1LoadModel(filename.c_str());
	mnHandle = Master::mpResourceManager->LoadModel(filename.c_str()); // 変更
}

AttachmentModel::~AttachmentModel()
{
	// モデルの削除
	MV1DeleteModel(mnHandle);
}

void AttachmentModel::LateUpdate()
{
	if (!mVisible)// 非表示なら処理しないように 0101
	{
		return;
	}
	// アタッチ先のモデルのフレームの行列情報を取得
	MATRIX matrix = MV1GetFrameLocalWorldMatrix(mnParentHandle, mnParentFrameIndex);


	//取得した行列が原点を指しているかチェック
	// 敵が死んだ後に武器だけも凝る問題を解消するために追加
	if (matrix.m[3][0] == 0.0f &&
		matrix.m[3][1] == 0.0f &&
		matrix.m[3][2] == 0.0f)
	{
		// 0,0,0 なら「親が消えた」と判断して、
		// MV1SetMatrix を呼ばずに終了する（＝前のフレームの正常な位置を維持する）
		mVisible = false;  // 非表示にしたい
		return;
	}


	// 回転のやつ　追加
	// とりあえず取得
	MATRIX rotX = MGetRotX(mvRotation.x);
	MATRIX rotY = MGetRotY(mvRotation.y);
	MATRIX rotZ = MGetRotZ(mvRotation.z);

	// 回転を一つにまとめる 中にいれても問題ない
	MATRIX Local = MMult(rotX, MMult(rotY, rotZ));
	// 親の行列に回転を掛け合わせる
	// 回転してからロードしたい
	MATRIX Rota = MMult(Local, matrix);




	//  がったいさせた行列をセットする
	MV1SetMatrix(mnHandle, Rota);

	// 上で取得した行列情報を自身に設定する
	//MV1SetMatrix(mnHandle, matrix);
}

void AttachmentModel::Draw()
{
	if (!mVisible)// 非表示なら処理しないように 0101
	{
		return; // これがないと変えても武器がたくさんでてる
	}
	// モデルの描画
	MV1DrawModel(mnHandle);
}

// 新年に追加
// 今の武器がどこにあるか
VECTOR AttachmentModel::GetWorldPosition() const
{
	//// 武器モデルの行列を取得
	//MATRIX matrix = MV1GetMatrix(mnHandle);

	//// モデルの今の座標を返している 位置だけを取り出している
	//// 行列の平行移動成分がワールド座標 今のをとっている
	//// DXの行列構造体にあるのが "m" だからm以外はerror
	//// MATRIXの中の位置だけ取り出して座標にして返す処理
	//return VGet(
	//	matrix.m[3][0], // X座標
	//	matrix.m[3][1], // Y座標
	//	matrix.m[3][2]  // Z座標
	//);


	// 現在の計算済み行列ではなく、今この瞬間の親の行列を直接見に行く
	MATRIX parentMatrix = MV1GetFrameLocalWorldMatrix(mnParentHandle, mnParentFrameIndex);
	// RotateWeapon で設定された回転も一応やっとく
	MATRIX rotX = MGetRotX(mvRotation.x);
	MATRIX rotY = MGetRotY(mvRotation.y);
	MATRIX rotZ = MGetRotZ(mvRotation.z);
	MATRIX AllRot = MMult(rotX, MMult(rotY, rotZ));

	// 親の行列と掛け合わせる LateUpdateと同じ計算をここでも行う
	MATRIX matrix = MMult(AllRot, parentMatrix);

	// 計算したばかりの行列から座標を返す
	return VGet(
		matrix.m[3][0],
		matrix.m[3][1],
		matrix.m[3][2]
	);

	//// メモ　MATRIXは[4][4]らしい
	//// m[0] = X軸方向(向き)
	//// m[1] = y軸方向(向き)
	//// m[2] = z軸方向(向き)
	//// m[3] = 位置全部
	//// だから、Xno位置で3と１とかになってる
	//// mat.m[3]だけにすると配列になるから注意
}

// 武器のframeの位置を所得したい
VECTOR AttachmentModel::GetFrame(const std::string& frameName) const
{
	int frameIndex = MV1SearchFrame(mnHandle, frameName.c_str());
	if (frameIndex < 0) // 見つからなかったら
	{
		// デバッグ用 なかったらでるように
	   // printfDx("Weapon frame not found : %s\n", frameName.c_str());

		// ０００だと位置が変だから武器の位置を返すようにしている
		// これでだいたい武器ん尾中心からでるから良さげ
		return GetWorldPosition();
	}

	// 指定のフレームのローカル座標からワールド座標に変換する行列を得る
	MATRIX mat = MV1GetFrameLocalWorldMatrix(mnHandle, frameIndex);
	// 位置返す
	return VGet(
		mat.m[3][0],
		mat.m[3][1],
		mat.m[3][2]);
}
