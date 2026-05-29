#include "SoundManager.h"
#include "DxLib.h"
#include "Master.h"

SoundManager::SoundManager()
	: mnNowPlayingBgm((SOUND_BGM)-1)  // 初期は何も再生されていない状態
	, mnNowPlayingSe((SOUND_SE)-1)    // 初期は何も再生されていない状態
{


}

SoundManager::~SoundManager()
{

}

void SoundManager::Initialize()
{
	// 音楽の音量を設定できるものs
	//int SHandle;
	//SHandle = LoadSoundMem("Resource/BGM/3D/maou_bgm_cyber01.mp3");
	//PlaySoundMem(SHandle, DX_PLAYTYPE_BACK);
	//// 音量の設定
	//ChangeVolumeSoundMem(255 * 80 / 100, SHandle);

	// BGM の読み込み
	LoadBGM(SOUND_BGM::BGM_TITLE, "Resource/BGM/Title1.mp3");	// タイトル用のBGM

	// SE の読み込み
	LoadSE(SOUND_SE::SE_ENTER, "Resource/SE/Enter.mp3");		// 決定音
	LoadSE(SOUND_SE::SE_SELECT, "Resource/SE/Select.mp3");		// 選択音
	LoadSE(SOUND_SE::SE_JUMP, "Resource/SE/Jump.mp3");			// ジャンプ音
	LoadSE(SOUND_SE::SE_SHOT, "Resource/SE/Shot.mp3");			// ジャンプ音
}


void SoundManager::Finalize()
{
	// BGMの破棄
	for (auto it = mnBgmHandleList.begin(); it != mnBgmHandleList.end(); it++)
	{
		DeleteSoundMem(it->second);
	}

	// SEの破棄
	for (auto it = mnSeHandleList.begin(); it != mnSeHandleList.end(); it++)
	{
		DeleteSoundMem(it->second);
	}
}


void SoundManager::PlayBGM(SOUND_BGM bgm, bool isTop, int volume)
{
	// 現在再生されているBGMと同じかつ
	// 最初からの再生ではないのであれば return する
	if (mnNowPlayingBgm == bgm && !isTop)
	{
		return;
	}

	//  現在再生中のBGMを止める
	// 今流れているBGMを探して停メル
	for (auto it = mnBgmHandleList.begin(); it != mnBgmHandleList.end(); it++)
	{
		if (it->first == mnNowPlayingBgm)
		{
			if (CheckSoundMem(it->second))
			{
				StopSoundMem(it->second);
			}
			break;
		}
	}

	// 新しいBGMのハンドルを取得して再生
	for (auto it = mnBgmHandleList.begin(); it != mnBgmHandleList.end(); it++)
	{
		if (it->first == bgm)
		{
			// 再生する前に音量を設定する
			ChangeVolumeSoundMem(volume, it->second);
			// BGMをループ再生
			PlaySoundMem(it->second, DX_PLAYTYPE_LOOP, TRUE); // ループ再生
			// 現在の再生種類を更新
			mnNowPlayingBgm = bgm;
		}
	}
}


// 音量変えたい
void SoundManager::ChangeVolume(int volume)
{
	// 何も再生していないなら何もしない
	if (mnNowPlayingBgm == BGM_NONE) return;

	// すでに再生中のBGMがあるか確認
	int currentHandle = -1;
	for (auto it = mnBgmHandleList.begin(); it != mnBgmHandleList.end(); it++)
	{
		if (it->first == mnNowPlayingBgm)
		{
			currentHandle = it->second;
			break;
		}
	}

	// 見つかったら音量を変更
	if (currentHandle != -1)
	{
		ChangeVolumeSoundMem(volume, currentHandle);
	}
}

// 音量変えたいSE追加した
void SoundManager::Change_SE_Volume(SOUND_SE se, int volume)
{
	// 指定されたSE探し
	int targetHandle = -1;
	for (auto it = mnSeHandleList.begin(); it != mnSeHandleList.end(); it++)
	{
		if (it->first == se)
		{
			targetHandle = it->second;
			break;
		}
	}

	// 見つかったらその SE の音量だけを変更
	if (targetHandle != -1)
	{
		ChangeVolumeSoundMem(volume, targetHandle);
	}
}


// 指定したSEを個別に止める関数
void SoundManager::StopSE(SOUND_SE se)
{
	// SEのリストから 止めたいSEを探す
	for (auto it = mnSeHandleList.begin(); it != mnSeHandleList.end(); it++)
	{
		if (it->first == se)
		{
			// 再生中かどうか確認して止める
			if (CheckSoundMem(it->second))
			{
				StopSoundMem(it->second);
			}
			break;
		}
	}
}


void SoundManager::PlaySE(SOUND_SE se, int volume)
{
	for (auto it = mnSeHandleList.begin(); it != mnSeHandleList.end(); it++)
	{
		// 一致した種類のSEがあれば
		if (it->first == se)
		{
			// 再生前に音量を設定する
			ChangeVolumeSoundMem(volume, it->second);
			// SEをループ再生
			PlaySoundMem(it->second, DX_PLAYTYPE_BACK); // バックグラウンド再生する
			// 現在の再生種類を更新
			mnNowPlayingSe = se;
		}
	}
}

void SoundManager::LoadBGM(SOUND_BGM bgm, std::string filename)
{
	bool check = false; // 重複して読み込んでいるかどうか
	for (auto it = mnBgmHandleList.begin(); it != mnBgmHandleList.end(); it++)
	{
		if (it->first == bgm)
		{
			check = true;
			break;
		}
	}

	// 重複して読み込まれていたら何もしない
	if (check)
	{
		return;
	}

	// ファイル読み込み
	int handle = LoadSoundMem(filename.c_str());
	if (handle == -1) // -1だと失敗
	{
		return;   // 読み込み失敗したら何もしない
	}

	// 読み込んだハンドルをリストに追加
	// 読み込ん札ハンドルと、タイプ（種類）を一緒に追加
	// std::pair の一個めの変数（種類）がfirst とされている
	// pair の一つ目の要素を表していた
	// strust
	// {
	//  SOUND_BGM bgm;
	//  int handle;
	// }
	// が、std::pair<SOUND_BGM, int>

	mnBgmHandleList.push_back(std::pair<SOUND_BGM, int>(bgm, handle));
}


void SoundManager::LoadSE(SOUND_SE se, std::string filename)
{
	bool check = false; // 重複して読み込んでいるかどうか
	for (auto it = mnSeHandleList.begin(); it != mnSeHandleList.end(); it++)
	{
		if (it->first == se)
		{
			check = true;
			break;
		}
	}

	// 重複して読み込まれていたら何もしない
	if (check)
	{
		return;
	}

	// ファイル読み込み
	int handle = LoadSoundMem(filename.c_str());
	if (handle == -1) // -1だと失敗
	{
		return;   // 読み込み失敗したら何もしない
	}

	mnSeHandleList.push_back(std::pair<SOUND_SE, int>(se, handle));
}


void SoundManager::StopBGM()
{
	for (auto it = mnBgmHandleList.begin(); it != mnBgmHandleList.end(); it++)
	{
		// 今現在再生しているBGMと比較させる
		if (it->first == mnNowPlayingBgm)
		{
			// BGMが再生されているかのif文
			//          もし再生されていたら
			if (CheckSoundMem(it->second))
			{
				StopSoundMem(it->second); // BGMの停止
				break;
			}
		}
	}

	// チュートリアルから戻ったときにBGMが再生されないから
	mnNowPlayingBgm = BGM_NONE;
}

void SoundManager::PlaySE_Loop(SOUND_SE se, int volume)
{
	for (auto it = mnSeHandleList.begin(); it != mnSeHandleList.end(); it++)
	{
		if (it->first == se)
		{
			// すでに鳴っていたら二重に鳴らさない
			if (CheckSoundMem(it->second))
			{
				return;
			}

			// ボリューム
			ChangeVolumeSoundMem(volume, it->second);
			PlaySoundMem(it->second, DX_PLAYTYPE_LOOP);
			break;
		}
	}
}



// 没たち
//void SoundManager::PlayBGM(SOUND_BGM bgm, bool isTop, int volume)
//{
//	// すでに再生中のBGMがあるか確認
//	int currentHandle = -1;
//	for (auto it = mnBgmHandleList.begin(); it != mnBgmHandleList.end(); it++)
//	{
//		if (it->first == mnNowPlayingBgm)
//		{
//			currentHandle = it->second;
//			break;
//		}
//	}
//	// 同じBGMを再生したい場合
//	if (mnNowPlayingBgm == bgm)
//	{
//		if (isTop) // trueだったら  最初からの再生なら
//		{
//			// 最初から再生したいので一旦停止
//			if (CheckSoundMem(currentHandle))
//			{
//				StopSoundMem(currentHandle);
//			}
//		}
//		else
//		{
//			// 途中再生でOKなら何もしない
//			return;
//		}
//	}
//	// ここ追加した
//	// 選択からタイトルに戻っても自然に消えるから神
//	else
//	{
//		// 違うBGMなら現在再生中のBGMを停止
//		if (CheckSoundMem(currentHandle))
//		{
//			StopSoundMem(currentHandle);
//		}
//	}
//
//	// 新しいBGMのハンドルを取得して再生
//	for (auto it = mnBgmHandleList.begin(); it != mnBgmHandleList.end(); it++)
//	{
//		if (it->first == bgm)
//		{
//			// 再生する前に音量を設定する
//			ChangeVolumeSoundMem(volume, it->second);
//			// BGMをループ再生
//			PlaySoundMem(it->second, DX_PLAYTYPE_LOOP, TRUE); // ループ再生
//			// 現在の再生種類を更新
//			mnNowPlayingBgm = bgm;
//			//break;
//		}
//	}
//}



//void SoundManager::PlayBGM(SOUND_BGM bgm, bool isTop)
//{
//	// 現在再生されているBGMと同じかつ
//	// 最初からの再生ではないのであれば return する
//	if (mnNowPlayingBgm == bgm && !isTop)
//	{
//		return;
//	}
//
//	for (auto it = mnBgmHandleList.begin(); it != mnBgmHandleList.end(); it++)
//	{
//		// 一致した種類のBGMがあれば
//		if (it->first == bgm)
//		{
//			// BGMをループ再生
//			PlaySoundMem(it->second, DX_PLAYTYPE_LOOP, isTop); // ループしつつバックグラウンド再生する
//			// 現在の再生種類を更新
//			mnNowPlayingBgm = bgm;
//		}
//	}
//}

