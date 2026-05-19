#include "OBjectManager.h"
#include "Master.h"

// コンストラクタ
ObjectManager::ObjectManager()
{

}

// デストラクタ
ObjectManager::~ObjectManager()
{

}

// 更新
void ObjectManager::Update() // 
{
	// 2Dオブジェクトの更新
	// リストで繰り返しでつかわれる
	// mPbject2Dlist.begin    リストの先頭の要素へのイテレーターを取得する
	// mPbject2Dlist.end      リストの末尾の要素へのイテレーターを取得する


	// イテレーターとは？... std::list の中にあるひとつの要素のことを指す
	// リストの利点...要素と要素の間に新しい要素を差し込むことが容易に出来る
	// std::list...双方向連結リスト（要素の前後の要素にアクセスすることが簡単）
	for (std::list<Object2D*>::iterator itr = mObject2DList.begin(); itr != mObject2DList.end(); itr++)
	{
		(*itr)->Update();
	}

	// 3D用の Update
	for (std::list<Object3D*>::iterator itr = mObject3DList.begin(); itr != mObject3DList.end(); itr++)
	{
		(*itr)->Update();
	}


	// Lateいれる
	// ソートしたら、アタッチがずれるので、UnityみたいにLateUpdateをつくる
	for (std::list<Object3D*>::iterator itr = mObject3DList.begin(); itr != mObject3DList.end(); itr++)
	{
		(*itr)->LateUpdate();
	}



	// mObject2DList[0]   今まではこのように配列にアクセスしていたがリストでは出来ない。
	// これがVECTORとの相違点

	  // ★ New!! ★
	// 3Dの更新が終わった時点でカメラとの距離を計算する
	for (std::list<Object3D*>::iterator itr = mObject3DList.begin(); itr != mObject3DList.end(); itr++)
	{
		VECTOR cameraPos = Master::mpCamera->GetPosition();
		VECTOR objPos = (*itr)->GetPosition();
		(*itr)->SetCameraDistance(VSize(VSub(objPos, cameraPos)));
	}

}


// 描画
void ObjectManager::Draw()
{
	// enumをObject3D.hに追加したから書き換えた 1218
	// 透明のは最後に描画するようにしているが透明だけでソートしたほうが良いかも？

	// カメラ距離でソート
	mObject3DList.sort(Object3D::CompareZOrder());

	// 不透明オブジェクト
	for (auto obj : mObject3DList)
	{
		if (obj->GetDrawType() == Object3D::HutoumeiModel)
		{
			obj->Draw();
		}
	}

	// 透明オブジェクト（必ず最後）
	for (auto obj : mObject3DList)
	{
		if (obj->GetDrawType() == Object3D::ToumeiModel)
		{
			obj->Draw();
		}
	}



	//// ★ New!! ★ // もともとの
	//// 3DのZソート
	//// ただし、描画するオブジェクトが多くなるほどソート処理の負荷がかかるので、
	//// 極端に動作が重くなったりするようであれば、描画機構を最適化していく必要がある。
	//mObject3DList.sort(Object3D::CompareZOrder());
	//// 追記　1217 itrのdrawより前に持ってきた。2Dがそうだから一応

	//// 3D用の Draw
	//for (auto itr = mObject3DList.begin(); itr != mObject3DList.end(); itr++)
	//{
	//	(*itr)->Draw();
	//}


	// ★ New!! ★
	// 2DのZソートをしておく
	mObject2DList.sort(Object2D::CompareZOrder());

	// std::list<Object2D*>::iterator　これは長いので auto で書き替えも出来る。
	// auto... 型推論。　＝より右側の型を勝手に推測してくれる便利なやつ
	for (auto itr = mObject2DList.begin(); itr != mObject2DList.end(); itr++)
	{
		(*itr)->Draw();
	}

	// auto i;　だけだと使えない
	// auto i = 0; 初期化してあげると使える
}


// 2Dオブジェクトの追加
void ObjectManager::AddObject(Object2D* object2D)
{
	mObject2DList.push_back(object2D);
}

// 2Dオブジェクトの全削除
void ObjectManager::DeleteAll2D()
{
	for (auto itr = mObject2DList.begin(); itr != mObject2DList.end(); /*ここは空っぽなので注意*/)
	{// 一時的な変数に入れてあげてる
		Object2D* temp = *itr;

		// リストから削除  erase ＝消すという意味（これだけだとオブジェクトは削除されない）
		itr = mObject2DList.erase(itr);

		// オブジェクトそのものを削除
		delete temp;
		temp = nullptr;
	}
}

// 削除する必要のあるオブジェクトがあれば削除する
void ObjectManager::DeleteAll2DIfNeeded()
{
	for (auto itr = mObject2DList.begin(); itr != mObject2DList.end(); /*ここは空っぽなので注意*/)
	{
		// 破棄フラグが立っていれば削除する
		if ((*itr)->IsDeleteFlag() /*== true  これを省略した形*/)
		{
			Object2D* temp = *itr;

			// リストから削除  erase ＝消すという意味（これだけだとオブジェクトは削除されない）
			// ここで itr++; っをやってくれている
			//　erase は、削除したitr の次のitr を返却してくれている
			itr = mObject2DList.erase(itr);

			// オブジェクトそのものを削除
			delete temp;
			temp = nullptr;
		}
		else
		{
			// 次の要素へ進める
			// これをしないと for文 が進まない
			itr++;
		}
	}
}

// 指定したタグの2Dオブジェクトを取得
Object2D* ObjectManager::GetObject2DByTag(Object2D::Tag tag)
{

	// std::find を利用して対象のオブジェクトを探す
	auto itr = std::find_if(
		mObject2DList.begin(), // begin とend　の間のオブジェクトという意味
		mObject2DList.end(),
		[&](Object2D* obj) {return obj->GetTag() == tag; } // ラムダ式
		// 今回の場合、mObject2DList の要素を [参照]するという意味合い
		// (Object2D *obj) ... 参照したオブジェクトの型と引数名
		// { ... } ... 処理内容（今回は条件式）
		// [] () {...} この形がラムダ式の基本

		// 関数を作って処理を渡しているようなイメージ
	);

	// 見つかったかどうかの判定
	// itr が end と同じ値でなければ
	if (itr != mObject2DList.end())
	{
		return (*itr);  // オブジェクトが見つかった
	}

	return nullptr; // オブジェクトが見つからなかった なかったという意味
}


// 指定したタグの2Dオブジェクトのリストを取得
std::vector<Object2D*> ObjectManager::GetObject2DListByTag(Object2D::Tag tag)
{
	std::vector<Object2D*> ret;

	for (auto itr = mObject2DList.begin(); itr != mObject2DList.end(); itr++)
	{
		// tag と同じタグを持っているオブジェクトがあればvectorに入れる
		if ((*itr)->GetTag() == tag)
		{
			ret.push_back((*itr));
		}
	}

	return ret;
}








//=========================================== 3D用 =======================================//



// 3Dオブジェクトの追加
void ObjectManager::AddObject(Object3D* object3D)
{
	mObject3DList.push_back(object3D);
}

// 3Dオブジェクトの全削除
void ObjectManager::DeleteAll3D()
{
	for (auto itr = mObject3DList.begin(); itr != mObject3DList.end(); /*ここは空っぽなので注意*/)
	{// 一時的な変数に入れてあげてる
		Object3D* temp = *itr;

		// リストから削除  erase ＝消すという意味（これだけだとオブジェクトは削除されない）
		itr = mObject3DList.erase(itr);

		// オブジェクトそのものを削除
		delete temp;
		temp = nullptr;
	}
}

// 削除する必要のあるオブジェクトがあれば削除する
void ObjectManager::DeleteAll3DIfNeeded()
{
	for (auto itr = mObject3DList.begin(); itr != mObject3DList.end(); /*ここは空っぽなので注意*/)
	{
		// 破棄フラグが立っていれば削除する
		if ((*itr)->IsDeleteFlag() /*== true  これを省略した形*/)
		{
			Object3D* temp = *itr;

			// リストから削除  erase ＝消すという意味（これだけだとオブジェクトは削除されない）
			// ここで itr++; っをやってくれている
			//　erase は、削除したitr の次のitr を返却してくれている
			itr = mObject3DList.erase(itr);

			// オブジェクトそのものを削除
			delete temp;
			temp = nullptr;
		}
		else
		{
			// 次の要素へ進める
			// これをしないと for文 が進まない
			itr++;
		}
	}
}

// 指定したタグの3Dオブジェクトを取得
Object3D* ObjectManager::GetObject3DByTag(Object3D::Tag3D tag)
{

	// std::find を利用して対象のオブジェクトを探す
	auto itr = std::find_if(
		mObject3DList.begin(), // begin とend　の間のオブジェクトという意味
		mObject3DList.end(),
		[&](Object3D* obj) {return obj->GetTag() == tag; } // ラムダ式
		// 今回の場合、mObject3DList の要素を [参照]するという意味合い
		// (Object3D *obj) ... 参照したオブジェクトの型と引数名
		// { ... } ... 処理内容（今回は条件式）
		// [] () {...} この形がラムダ式の基本

		// 関数を作って処理を渡しているようなイメージ
	);

	// 見つかったかどうかの判定
	// itr が end と同じ値でなければ
	if (itr != mObject3DList.end())
	{
		return (*itr);  // オブジェクトが見つかった
	}

	return nullptr; // オブジェクトが見つからなかった なかったという意味
}
// 指定したタグの3Dオブジェクトのリストを取得
std::vector<Object3D*> ObjectManager::GetObject3DListByTag(Object3D::Tag3D tag)
{
	std::vector<Object3D*> ret;

	for (auto itr = mObject3DList.begin(); itr != mObject3DList.end(); itr++)
	{
		// tag と同じタグを持っているオブジェクトがあればvectorに入れる
		if ((*itr)->GetTag() == tag)
		{
			ret.push_back((*itr));
		}
	}

	return ret;
}