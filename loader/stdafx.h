// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#pragma once

#ifndef _WIN32_WINNT		// Windows XP 以降のバージョンに固有の機能の使用を許可します。                   
#define _WIN32_WINNT 0x0501	// これを Windows の他のバージョン向けに適切な値に変更してください。
#endif						

#include <stdio.h>
#include <tchar.h>
#include <windows.h>

// TODO: プログラムに必要な追加ヘッダーをここで参照してください。
// コマンドラインオプション
#define OPT_FILENAME	0
#define OPT_CONFIGNAME	1
#define OPT_NAMESPACE	2
#define OPT_TIMER		3
#define OPT_AUTH		4

// エラーステータス
#define STS_SUCCESS			0
#define STS_INVBINDIRERR		1
#define STS_INVOPTION		2
#define STS_IOERR			3
#define STS_INVIRISSTATUS	4
#define STS_RTNCALLERR		5
#define STS_INVNAMESPACEERR	6
#define STS_BUSY			7

#define STS_EXITCODEBASE	16	

// 実行ルーチン
#define ExecRoutine	L"%ZLOADER"

