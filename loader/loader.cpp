// loader.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "iris-callin.h"
#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>

// ディレクトリ情報格納領域
char 	myBinaryDir[MAX_PATH];
char 	myMgrDir[MAX_PATH];

// IRIS構造体
IRISWSTR pInput, pOutput;

TCHAR * filename = NULL;
TCHAR * irisnamespace = NULL;

// オプション文字列
int optc;
_TCHAR ** optv;

// 名前付きパイプ
TCHAR namedpipe[FILENAME_MAX];
HANDLE hPipe = INVALID_HANDLE_VALUE;

// ログファイル
HANDLE hOut = INVALID_HANDLE_VALUE;


// コマンドオプションの解析
int parseoption(int argc, _TCHAR * argv[])
{
	int opt = OPT_FILENAME;

	for( int i = 1; i < argc; i++ ) {
		if( opt == OPT_FILENAME && (* argv[i] == '-' || * argv[i] == '/') ) {
			switch(*(argv[i]+1)) {
				case 'U':
				case 'u':
					if( *( argv[i]+2) == '\0') {
						opt = OPT_NAMESPACE;
					} else {
						irisnamespace = argv[i] + 2;
					}
					break;
				default:
					_ftprintf(stdout, _T("オプションが誤っています :%s\n"),argv[i]);
					return 1;
			}
		} else {
			switch (opt) {
				case OPT_NAMESPACE:
					irisnamespace = argv[i];
					break;
				default:
					filename = argv[i];
					optc = argc - i -1;
					optv = &argv[i+1];
					break;
			}
			// ファイル名に値が入れば終了
			if( filename != NULL ) break;
			opt = OPT_FILENAME;
		}
	}

	// ファイル名指定チェック
	if( filename == NULL ) {
		fprintf(stdout, "ファイル名が指定されていません\n");
		return STS_INVOPTION;
	}

	return STS_SUCCESS;
}
// IRISエラーメッセージ取得
wchar_t * getiriserror(int rc)
{
	wchar_t * sMes;
	switch (rc)
	{
	case IRIS_ACCESSDENIED:
		sMes = L"認証に失敗しました。監査ログにて実際の認証エラーを確認してください";
		break;
	case IRIS_ALREADYCON:
		sMes = L"既に接続されています";
		break;
	case IRIS_CHANGEPASSWORD:
		sMes = L"パスワードの変更が必要です";
		break;
	case IRIS_CONBROKEN:
		sMes = L"コネクションが切断されています";
		break;
	case IRIS_STRTOOLONG:
		sMes = L"入出力デバイスの文字列が長すぎます";
		break;
	case IRIS_SUCCESS:
		sMes = L"成功";
		break;
	case IRIS_FAILURE:
	default:
		sMes = L"処理が失敗しました";
	}
	return sMes;
}

// メイン処理
int _tmain(int argc, _TCHAR* argv[])
{
	// ファイル名格納領域
	wchar_t loadfile[32768];
	wchar_t curdir[32768];
	wchar_t errmsg[2048];

	int len;

	int termflag = IRIS_PROGMODE | IRIS_TTALL;
	int	rc, timeout = 10;
	int retval;

	// Unicodeロケール
	setlocale(LC_CTYPE, "");

	// 現在のディレクトリを求める
	GetCurrentDirectory(sizeof(curdir), curdir);

	// コマンドオプションの解析
	if ((rc = parseoption(argc, argv)) != 0) {
		_ftprintf(stdout, _T("使用方法: loader [ -U ネームスペース ] インストールファイル名 [ オプション... ]\n"));
		return rc;
	}
	// 二重起動の禁止
	HANDLE hMutex;

	hMutex = CreateMutex(NULL, TRUE, _T("ISRTNLOADER"));
	if (hMutex == NULL) {
		len = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, errmsg, sizeof(errmsg), 0);
		errmsg[len] = 0;
		fwprintf(stdout, _T("ミューテックスがオープンできません\n%s\n"), errmsg);
		return STS_IOERR;
	}
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		CloseHandle(hMutex);
		_ftprintf(stdout, _T("現在インストーラが実行中です\n"));
		return STS_BUSY;
	}

	// 実行ファイルのパスを求める
	if (!GetModuleFileNameA(NULL, myBinaryDir, MAX_PATH))
	{
		_ftprintf(stdout, _T("実行ファイルのディレクトリが取得できません\n"));
		return STS_INVBINDIRERR;
	}

	// binディレクトリとmgrディレクトリを求める
	char *pTmp = strrchr(myBinaryDir, (int)'\\');
	if (pTmp == NULL) {
		_ftprintf(stdout, _T("Mgrディレクトリが取得できません\n"));
		return STS_INVBINDIRERR;
	}
	*(pTmp+1) = '\0';
	// binディレクトリをmgrディレクトリにコピー
	strcpy_s(myMgrDir, MAX_PATH, myBinaryDir);
	// 終端位置を求める
	pTmp = myMgrDir + (pTmp - myBinaryDir);

	while (pTmp > myMgrDir)
	{
		pTmp--;
		if (*pTmp == '\\') {
			//binディレクトリをmgrディレクトリに置換
			pTmp++;
			strcpy_s(pTmp, 5, "Mgr\\");
			break;
		}
	}

	// Mgrディレクトリの設定
	rc = IRISSETDIR(myMgrDir);
	if (rc != IRIS_SUCCESS) {
		_ftprintf(stdout, _T("Mgrディレクトリが設定できません\n"));
		return STS_INVBINDIRERR;
	}

	// ロードファイルのフルパスを求める
	GetFullPathName(filename, sizeof(loadfile), loadfile, 0);

	// Call-inの開始
	rc = IRISSTARTW(termflag, timeout, &pInput, &pOutput);
	if (rc != IRIS_SUCCESS) {
		_ftprintf(stdout, _T("IRISStartエラー:%s\n"),getiriserror(rc));
		return STS_INVBINDIRERR;
	}

	// ネームスペース変更
	if (irisnamespace != NULL)
	{
		IRISWSTR exec;
		wsprintf((wchar_t *)exec.str, L"zn \"%s\"", irisnamespace);
		exec.len = (unsigned short)wcslen((wchar_t *)exec.str);
		rc = IRISEXECUTEW(&exec);
		if (rc != IRIS_SUCCESS) {
			wprintf( _T("IRISネームスペースエラー:%s(%d)\n"), getiriserror(rc),rc);
			IRISEND();
			return STS_INVNAMESPACEERR;
		}
	}
	// %ZLOADERの実行
	
	// 作業ディレクトリ、ファイル名、パラメータを引数に登録
	unsigned int rflags;

	rc = IRISPUSHFUNCW(&rflags, 0, NULL, (int)wcslen(ExecRoutine), (unsigned short int *)ExecRoutine);
	if (rc != IRIS_SUCCESS) {
		_ftprintf(stdout, _T("IRISRoutine設定エラー:%s\n"), getiriserror(rc));
		IRISEND();
		return STS_RTNCALLERR;
	}

	rc = IRISPUSHSTRW((int)wcslen(curdir), (unsigned short int *)curdir);
	if (rc != IRIS_SUCCESS) {
		_ftprintf(stdout, _T("IRIS作業Dir設定エラー:%s\n"), getiriserror(rc));
		IRISEND();
		return STS_RTNCALLERR;
	}

	rc = IRISPUSHSTRW((int)wcslen(loadfile), (unsigned short int *)loadfile);
	if (rc != IRIS_SUCCESS) {
		_ftprintf(stdout, _T("IRIS読込ファイル設定エラー:%s\n"), getiriserror(rc));
		IRISEND();
		return STS_RTNCALLERR;
	}

	for (int i = 0; i < optc; i++)
	{
		rc = IRISPUSHSTRW((int)wcslen(optv[i]), (unsigned short int *)optv[i]);
		if (rc != IRIS_SUCCESS) {
			_ftprintf(stdout, _T("IRISパラメータ設定エラー:%s\n"), getiriserror(rc));
			IRISEND();
			return STS_RTNCALLERR;
		}
	}

	// ルーチン実行
	rc = IRISEXTFUN(rflags, optc+2);
	if (rc != IRIS_SUCCESS) {
		_ftprintf(stdout, _T("IRISRoutine実行エラー:%s\n"), getiriserror(rc));
		IRISEND();
		return STS_RTNCALLERR;
	}

	// 戻り値の取得
	rc = IRISPOPINT(&retval);
	if (rc != IRIS_SUCCESS) {
		_ftprintf(stdout, _T("IRIS戻り値取得エラー:%s\n"), getiriserror(rc));
		IRISEND();
		return STS_RTNCALLERR;
	}

	// IRISの終了
	IRISEND();

	return retval;
}

