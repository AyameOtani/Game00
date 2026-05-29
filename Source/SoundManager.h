#pragma once
#include <string>
#include <vector>

class SoundManager
{
public:   // enum, struct などの定義

	// enumで番号をふっている
	enum SOUND_BGM  // BGMの種類
	{
		BGM_NONE = -1,		// BGMを消す
		BGM_TITLE = 0,		// タイトル用のBGM

		// 増やしても良い
	};

	enum SOUND_SE  // SEの種類
	{
		SE_ENTER = 0,		 // 決定音
		SE_SELECT = 1,		 // 選択音
		SE_JUMP = 2,		 // ジャンプ音
		SE_SHOT = 3,		 // ショット音
	};

public:  // 公開

	SoundManager(); // コンストラクタ

	~SoundManager(); // デストラクタ

	// 初期化
	void Initialize();

	// 終了処理
	void Finalize();


public:   // サウンド再生系  こっちは関数


	// BGM再生
	// SOUND_BGM... 鳴らしたいBGMの種類
	// voli\ume... 大きさ　追加した
	// isTop... 最初から再生するかどうか
	// bool isTop = true -> デフォルト引数と呼ばれる書き方。館数を呼びだす際に渡さなくても認識される。
	void PlayBGM(SOUND_BGM bgm, bool isTop = true, int volume = 255);

	// 音量変えるやつ	BGM
	void ChangeVolume(int volume);

	// ループするSE
	void PlaySE_Loop(SOUND_SE se, int volume);
	// 音量変えるやつ	SE
	void Change_SE_Volume(SOUND_SE se, int volume);
	void StopSE(SOUND_SE se); // 止める SE SEというかながめのやつ

	// SE再生
	// SEは短い音なので最初から再生のは入れなくて良い
	// 大きさ追した 0306 マスターまでに間に合うとよいがな
	void PlaySE(SOUND_SE se, int volume = 255);

	// BGMの読み込み
	void LoadBGM(SOUND_BGM bgm, std::string filename);

	// SEの読み込み
	void LoadSE(SOUND_SE se, std::string filename);

	// BGMの停止
	void StopBGM();


private:  // メンバ変数

	SOUND_BGM mnNowPlayingBgm;  // 現在再生されているBGMの種類
	SOUND_SE mnNowPlayingSe;  // 現在再生されているSEの種類


	// pair... 構造体みたいな要素？ 二ついれられるやつ
	// 
	// よみこんだBGMハンドルのリスト
	std::vector<std::pair<SOUND_BGM, int>> mnBgmHandleList;
	// よみこんだSEハンドルのリスト
	std::vector<std::pair<SOUND_SE, int>> mnSeHandleList;

};




