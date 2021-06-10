// loader.cpp : �R���\�[�� �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
//

#include "stdafx.h"
#include "iris-callin.h"
#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>

// �f�B���N�g�����i�[�̈�
char 	myBinaryDir[MAX_PATH];
char 	myMgrDir[MAX_PATH];

// IRIS�\����
IRISWSTR pInput, pOutput;

TCHAR * filename = NULL;
TCHAR * irisnamespace = NULL;

// �I�v�V����������
int optc;
_TCHAR ** optv;

// ���O�t���p�C�v
TCHAR namedpipe[FILENAME_MAX];
HANDLE hPipe = INVALID_HANDLE_VALUE;

// ���O�t�@�C��
HANDLE hOut = INVALID_HANDLE_VALUE;


// �R�}���h�I�v�V�����̉��
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
					_ftprintf(stdout, _T("�I�v�V����������Ă��܂� :%s\n"),argv[i]);
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
			// �t�@�C�����ɒl������ΏI��
			if( filename != NULL ) break;
			opt = OPT_FILENAME;
		}
	}

	// �t�@�C�����w��`�F�b�N
	if( filename == NULL ) {
		fprintf(stdout, "�t�@�C�������w�肳��Ă��܂���\n");
		return STS_INVOPTION;
	}

	return STS_SUCCESS;
}
// IRIS�G���[���b�Z�[�W�擾
wchar_t * getiriserror(int rc)
{
	wchar_t * sMes;
	switch (rc)
	{
	case IRIS_ACCESSDENIED:
		sMes = L"�F�؂Ɏ��s���܂����B�č����O�ɂĎ��ۂ̔F�؃G���[���m�F���Ă�������";
		break;
	case IRIS_ALREADYCON:
		sMes = L"���ɐڑ�����Ă��܂�";
		break;
	case IRIS_CHANGEPASSWORD:
		sMes = L"�p�X���[�h�̕ύX���K�v�ł�";
		break;
	case IRIS_CONBROKEN:
		sMes = L"�R�l�N�V�������ؒf����Ă��܂�";
		break;
	case IRIS_STRTOOLONG:
		sMes = L"���o�̓f�o�C�X�̕����񂪒������܂�";
		break;
	case IRIS_SUCCESS:
		sMes = L"����";
		break;
	case IRIS_FAILURE:
	default:
		sMes = L"���������s���܂���";
	}
	return sMes;
}

/*
// ���O�o�̓X���b�h����
DWORD WINAPI writelogtoconsole(LPVOID p)
{
	char stsbuf[1024];
	int		sc;

	TCHAR errmsg[2048];

	DWORD len, nBytesRead, nCharsWritten;

	// ���O�t���p�C�v�I�[�v��
	hPipe = CreateNamedPipe(L"\\\\.\\pipe\\mypipe",
		PIPE_ACCESS_INBOUND, PIPE_TYPE_BYTE | PIPE_WAIT, 1, 0, 0, 100, NULL);
	if (hPipe == INVALID_HANDLE_VALUE) {
		fwprintf(stdout, L"���O�t���p�C�v���I�[�v���ł��܂���ł���\n");
		CloseHandle(hOut);
		return STS_IOERR;
	}
	if (!ConnectNamedPipe(hPipe, NULL)) {
		fwprintf(stdout, L"���O�t���p�C�v�ɐڑ��ł��܂���ł���");
		CloseHandle(hPipe);
		CloseHandle(hOut);
		return STS_IOERR;
	}

	while ((sc = ReadFile(hPipe, stsbuf, (DWORD)(sizeof(stsbuf) - 1), &nBytesRead, 0)) && nBytesRead != 0) {
		stsbuf[nBytesRead] = 0;
		WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), stsbuf, nBytesRead, &nCharsWritten, NULL);
		if (hOut != INVALID_HANDLE_VALUE) {
			WriteFile(hOut, stsbuf, nBytesRead, &nCharsWritten, NULL);
		}
	}

	CloseHandle(hPipe);
	if (hOut != INVALID_HANDLE_VALUE) {
		CloseHandle(hOut);
	}
	if (!sc) {
		if (GetLastError() == ERROR_BROKEN_PIPE)
			return STS_SUCCESS;

		len = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, errmsg, sizeof(errmsg), 0);
		errmsg[len] = 0;
		fwprintf(stdout, L"IRIS�̏o�̓f�[�^���ǂ߂܂���\n%s", errmsg);
		return STS_IOERR;
	}

	return STS_SUCCESS;
}
*/
// ���C������
int _tmain(int argc, _TCHAR* argv[])
{
	// �t�@�C�����i�[�̈�
	wchar_t loadfile[32768];
	wchar_t curdir[32768];
	//wchar_t logfile[2048];
	wchar_t errmsg[2048];

	int len;
	//TCHAR buf[2048];

	int termflag = IRIS_PROGMODE | IRIS_TTALL;
	int	rc, timeout = 10;
	int retval;

	// Unicode���P�[��
	setlocale(LC_CTYPE, "");

	// ���݂̃f�B���N�g�������߂�
	GetCurrentDirectory(sizeof(curdir), curdir);

	// �R�}���h�I�v�V�����̉��
	if ((rc = parseoption(argc, argv)) != 0) {
		_ftprintf(stdout, _T("�g�p���@: loader [ -U �l�[���X�y�[�X ] �C���X�g�[���t�@�C���� [ �I�v�V����... ]\n"));
		return rc;
	}
	/*
	// ���o�̓f�o�C�X���̐ݒ�(�󕶎�)
	pInput.len = 0;
	wcscpy_s((wchar_t *) pOutput.str,32767, L"\\\\.\\pipe\\mypipe");
	pOutput.len = wcslen(L"\\\\.\\pipe\\mypipe");
	*/
	// ��d�N���̋֎~
	HANDLE hMutex;

	hMutex = CreateMutex(NULL, TRUE, _T("ISRTNLOADER"));
	if (hMutex == NULL) {
		len = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, errmsg, sizeof(errmsg), 0);
		errmsg[len] = 0;
		fwprintf(stdout, _T("�~���[�e�b�N�X���I�[�v���ł��܂���\n%s\n"), errmsg);
		return STS_IOERR;
	}
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		CloseHandle(hMutex);
		_ftprintf(stdout, _T("���݃C���X�g�[�������s���ł�\n"));
		return STS_BUSY;
	}

	/*
	// �o�̓t�@�C���I�[�v��
	SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
	wsprintf(logfile, L"%s\\rtnload.log", curdir);
	if (logfile != NULL) {
		if ((hOut = CreateFile(logfile, GENERIC_WRITE, FILE_SHARE_READ, &sa, CREATE_ALWAYS, 0, 0)) == INVALID_HANDLE_VALUE) {
			int len = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, errmsg, sizeof(errmsg), 0);
			errmsg[len] = 0;
			fwprintf(stdout, L"���O�t�@�C�����I�[�v���ł��܂���ł���\n%s\n", errmsg);
			// ���O�t�@�C�����I�[�v���ł��Ȃ������ꍇ�ł����s
			logfile[0] = '\0';
		}
	}

	// ���O�o�̓X���b�h�̍쐬
	HANDLE hThread = CreateThread(NULL, 0, writelogtoconsole, NULL, 0, 0);
	if (hThread == NULL) {
		len = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, errmsg, sizeof(errmsg), 0);
		errmsg[len] = 0;
		fwprintf(stdout, L"���O�o�̓X���b�h���I�[�v���ł��܂���ł���\n%s\n", errmsg);
		CloseHandle(hPipe);
		CloseHandle(hOut);
		return STS_IOERR;
	}
	*/
	// ���s�t�@�C���̃p�X�����߂�
	if (!GetModuleFileNameA(NULL, myBinaryDir, MAX_PATH))
	{
		_ftprintf(stdout, _T("���s�t�@�C���̃f�B���N�g�����擾�ł��܂���\n"));
		//swprintf(buf, 2047, _T("���s�t�@�C���̃f�B���N�g�����擾�ł��܂���\n"));
		//WriteFile(hOut, buf, wcslen(buf), NULL, NULL);
		//CloseHandle(hOut);
		return STS_INVBINDIRERR;
	}

	// bin�f�B���N�g����mgr�f�B���N�g�������߂�
	char *pTmp = strrchr(myBinaryDir, (int)'\\');
	if (pTmp == NULL) {
		_ftprintf(stdout, _T("Mgr�f�B���N�g�����擾�ł��܂���\n"));
		//swprintf(buf, 2047,_T("Mgr�f�B���N�g�����擾�ł��܂���\n"));
		//WriteFile(hOut, buf, wcslen(buf), NULL, NULL);
	//	CloseHandle(hOut);
		return STS_INVBINDIRERR;
	}
	*(pTmp+1) = '\0';
	// bin�f�B���N�g����mgr�f�B���N�g���ɃR�s�[
	strcpy_s(myMgrDir, MAX_PATH, myBinaryDir);
	// �I�[�ʒu�����߂�
	pTmp = myMgrDir + (pTmp - myBinaryDir);

	while (pTmp > myMgrDir)
	{
		pTmp--;
		if (*pTmp == '\\') {
			//bin�f�B���N�g����mgr�f�B���N�g���ɒu��
			pTmp++;
			strcpy_s(pTmp, 5, "Mgr\\");
			break;
		}
	}

	// Mgr�f�B���N�g���̐ݒ�
	rc = IRISSETDIR(myMgrDir);
	if (rc != IRIS_SUCCESS) {
		_ftprintf(stdout, _T("Mgr�f�B���N�g�����ݒ�ł��܂���\n"));
		//swprintf(buf, 2047, _T("Mgr�f�B���N�g�����ݒ�ł��܂���\n"));
		//WriteFile(hOut, buf, wcslen(buf), NULL, NULL);
		//CloseHandle(hOut);
		return STS_INVBINDIRERR;
	}

	// ���[�h�t�@�C���̃t���p�X�����߂�
	GetFullPathName(filename, sizeof(loadfile), loadfile, 0);

	// Call-in�̊J�n
	rc = IRISSTARTW(termflag, timeout, &pInput, &pOutput);
	if (rc != IRIS_SUCCESS) {
		_ftprintf(stdout, _T("IRISStart�G���[:%s\n"),getiriserror(rc));
		//swprintf(buf, 2047, _T("IRISStart�G���[:%s\n"), getiriserror(rc));
		//WriteFile(hOut, buf, wcslen(buf), NULL, NULL);
		//CloseHandle(hOut);
		return STS_INVBINDIRERR;
	}

	// �l�[���X�y�[�X�ύX
	if (irisnamespace != NULL)
	{
		IRISWSTR exec;
		wsprintf((wchar_t *)exec.str, L"zn \"%s\"", irisnamespace);
		exec.len = (unsigned short)wcslen((wchar_t *)exec.str);
		rc = IRISEXECUTEW(&exec);
		if (rc != IRIS_SUCCESS) {
			wprintf( _T("IRIS�l�[���X�y�[�X�G���[:%s(%d)\n"), getiriserror(rc),rc);
		//	swprintf(buf, 2047, _T("IRIS�l�[���X�y�[�X�G���[:%s\n"), getiriserror(rc));
		//	WriteFile(hOut, buf, wcslen(buf), NULL, NULL);
		//	CloseHandle(hOut);
			IRISEND();
			return STS_INVNAMESPACEERR;
		}
	}
	// %ZLOADER�̎��s
	
	// ��ƃf�B���N�g���A�t�@�C�����A�p�����[�^�������ɓo�^
	unsigned int rflags;

	rc = IRISPUSHFUNCW(&rflags, 0, NULL, (int)wcslen(ExecRoutine), (unsigned short int *)ExecRoutine);
	if (rc != IRIS_SUCCESS) {
		_ftprintf(stdout, _T("IRISRoutine�ݒ�G���[:%s\n"), getiriserror(rc));
	//	swprintf(buf, 2047, _T("IRISRoutine�ݒ�G���[:%s\n"), getiriserror(rc));
	//	WriteFile(hOut, buf, wcslen(buf), NULL, NULL);
	//	CloseHandle(hOut);
		IRISEND();
		return STS_RTNCALLERR;
	}

	rc = IRISPUSHSTRW((int)wcslen(curdir), (unsigned short int *)curdir);
	if (rc != IRIS_SUCCESS) {
		_ftprintf(stdout, _T("IRIS���Dir�ݒ�G���[:%s\n"), getiriserror(rc));
	//	swprintf(buf, 2047, _T("IRIS���Dir�ݒ�G���[:%s\n"), getiriserror(rc));
	//	WriteFile(hOut, buf, wcslen(buf), NULL, NULL);
	//	CloseHandle(hOut);
		IRISEND();
		return STS_RTNCALLERR;
	}

	rc = IRISPUSHSTRW((int)wcslen(loadfile), (unsigned short int *)loadfile);
	if (rc != IRIS_SUCCESS) {
		_ftprintf(stdout, _T("IRIS�Ǎ��t�@�C���ݒ�G���[:%s\n"), getiriserror(rc));
	//	swprintf(buf, 2047, _T("IRIS�Ǎ��t�@�C���ݒ�G���[:%s\n"), getiriserror(rc));
	//	WriteFile(hOut, buf, wcslen(buf), NULL, NULL);
	//	CloseHandle(hOut);
		IRISEND();
		return STS_RTNCALLERR;
	}

	for (int i = 0; i < optc; i++)
	{
		rc = IRISPUSHSTRW((int)wcslen(optv[i]), (unsigned short int *)optv[i]);
		if (rc != IRIS_SUCCESS) {
			_ftprintf(stdout, _T("IRIS�p�����[�^�ݒ�G���[:%s\n"), getiriserror(rc));
		//	swprintf(buf, 2047, _T("IRIS�p�����[�^�ݒ�G���[:%s\n"), getiriserror(rc));
		//	WriteFile(hOut, buf, wcslen(buf), NULL, NULL);
		//	CloseHandle(hOut);
			IRISEND();
			return STS_RTNCALLERR;
		}
	}

	// ���[�`�����s
	rc = IRISEXTFUN(rflags, optc+2);
	if (rc != IRIS_SUCCESS) {
		_ftprintf(stdout, _T("IRISRoutine���s�G���[:%s\n"), getiriserror(rc));
	//	swprintf(buf, 2047, _T("RISRoutine���s�G���[:%s\n"), getiriserror(rc));
	//	WriteFile(hOut, buf, wcslen(buf), NULL, NULL);
	//	CloseHandle(hOut);
		IRISEND();
		return STS_RTNCALLERR;
	}

	// �߂�l�̎擾
	rc = IRISPOPINT(&retval);
	if (rc != IRIS_SUCCESS) {
		_ftprintf(stdout, _T("IRIS�߂�l�擾�G���[:%s\n"), getiriserror(rc));
	//	swprintf(buf, 2047, _T("IRIS�߂�l�擾�G���[:%s\n"), getiriserror(rc));
	//	WriteFile(hOut, buf, wcslen(buf), NULL, NULL);
	//	CloseHandle(hOut);
		IRISEND();
		return STS_RTNCALLERR;
	}

	// IRIS�̏I��
	IRISEND();
	//CloseHandle(hOut);

	return retval;
}

