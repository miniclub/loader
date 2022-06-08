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
		//	_ftprintf(stdout, _T("  %d: %s\n"), i,argv[i]);

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
/*					int loop;
					wchar_t* ptr;
					ptr = argv[i];

					for (loop = 0;loop < 32; loop++, ptr++) {
						_ftprintf(stdout, _T("%02X "), *ptr);
						if (*ptr == '\0') {
							_ftprintf(stdout, _T("\n"));
							break;
						}
					}
*/
					if (*argv[i] == '\"') {
						if (*(argv[i] + _tcslen(argv[i]) - 1) == '\"') {
							*(argv[i] + _tcslen(argv[i]) - 1) = '\0';
						}
						irisnamespace = (argv[i] + 1);
					}
					else {
						irisnamespace = argv[i];
					}
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
		_ftprintf(stdout, _T("�t�@�C�������w�肳��Ă��܂���\n"));
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
		sMes = _T("�F�؂Ɏ��s���܂����B�č����O�ɂĎ��ۂ̔F�؃G���[���m�F���Ă�������");
		break;
	case IRIS_ALREADYCON:
		sMes = _T("���ɐڑ�����Ă��܂�");
		break;
	case IRIS_CHANGEPASSWORD:
		sMes = _T("�p�X���[�h�̕ύX���K�v�ł�");
		break;
	case IRIS_CONBROKEN:
		sMes = _T("�R�l�N�V�������ؒf����Ă��܂�");
		break;
	case IRIS_STRTOOLONG:
		sMes = _T("���o�̓f�o�C�X�̕����񂪒������܂�");
		break;
	case IRIS_SUCCESS:
		sMes = _T("����");
		break;
	case IRIS_FAILURE:
	default:
		sMes = _T("���������s���܂���");
	}
	return sMes;
}
void OutputExitCode(int rc)
{
	_ftprintf(stdout, _T("\n%d\n"), rc);
}
// ���C������
int _tmain(int argc, _TCHAR* argv[])
{
	// �t�@�C�����i�[�̈�
	wchar_t loadfile[32768];
	wchar_t curdir[32768];
	wchar_t errmsg[2048];

	int len;

	int termflag = IRIS_PROGMODE | IRIS_TTALL;
	int	rc, timeout = 10;
	int retval;

	// Unicode���P�[��
	_tsetlocale(LC_ALL, _T("japanese"));
	//setlocale(LC_CTYPE, "");

	// ���݂̃f�B���N�g�������߂�
	GetCurrentDirectory(sizeof(curdir), curdir);

	// �R�}���h�I�v�V�����̉��
	if ((rc = parseoption(argc, argv)) != 0) {
		_ftprintf(stdout, _T("�g�p���@: loader [ -U �l�[���X�y�[�X ] �C���X�g�[���t�@�C���� [ �I�v�V����... ]\n"));
		OutputExitCode(rc);
		return rc;
	}
	// ��d�N���̋֎~
	HANDLE hMutex;

	hMutex = CreateMutex(NULL, TRUE, _T("ISRTNLOADER"));
	if (hMutex == NULL) {
		len = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, errmsg, sizeof(errmsg), 0);
		errmsg[len] = 0;
		_ftprintf(stdout, _T("�~���[�e�b�N�X���I�[�v���ł��܂���\n%s\n"), errmsg);
		OutputExitCode(STS_IOERR);
		return STS_IOERR;
	}
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		CloseHandle(hMutex);
		_ftprintf(stdout, _T("���݃C���X�g�[�������s���ł�\n"));
		OutputExitCode(STS_BUSY);
		return STS_BUSY;
	}

	// ���s�t�@�C���̃p�X�����߂�
	if (!GetModuleFileNameA(NULL, myBinaryDir, MAX_PATH))
	{
		_ftprintf(stdout, _T("���s�t�@�C���̃f�B���N�g�����擾�ł��܂���\n"));
		OutputExitCode(STS_INVBINDIRERR);
		return STS_INVBINDIRERR;
	}

	// bin�f�B���N�g����mgr�f�B���N�g�������߂�
	char *pTmp = strrchr(myBinaryDir, (int)'\\');
	if (pTmp == NULL) {
		_ftprintf(stdout, _T("Mgr�f�B���N�g�����擾�ł��܂���\n"));
		OutputExitCode(STS_INVBINDIRERR);
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
		OutputExitCode(STS_INVBINDIRERR);
		return STS_INVBINDIRERR;
	}

	// ���[�h�t�@�C���̃t���p�X�����߂�
	GetFullPathName(filename, sizeof(loadfile), loadfile, 0);

	// Call-in�̊J�n
	rc = IRISSTARTW(termflag, timeout, &pInput, &pOutput);
	if (rc != IRIS_SUCCESS) {
		_ftprintf(stdout, _T("IRISStart�G���[:%s\n"),getiriserror(rc));
		OutputExitCode(STS_INVBINDIRERR);
		return STS_INVBINDIRERR;
	}

	// �l�[���X�y�[�X�ύX
	if (irisnamespace != NULL)
	{
		IRISWSTR exec;
		//wsprintf((wchar_t*)exec.str, L"zn \"%s\"", irisnamespace);
		wsprintf((wchar_t*)exec.str, _T("set $namespace=\"%s\""), irisnamespace);
		exec.len = (unsigned short)wcslen((wchar_t *)exec.str);
		rc = IRISEXECUTEW(&exec);
		if (rc != IRIS_SUCCESS) {
			wprintf( _T("IRIS�l�[���X�y�[�X�G���[:%s(%d)\n"), getiriserror(rc),rc);
			IRISEND();
			OutputExitCode(STS_INVNAMESPACEERR);
			return STS_INVNAMESPACEERR;
		}
	}
	// %ZLOADER�̎��s
	
	// ��ƃf�B���N�g���A�t�@�C�����A�p�����[�^�������ɓo�^
	unsigned int rflags;

	rc = IRISPUSHFUNCW(&rflags, 0, NULL, (int)wcslen(ExecRoutine), (unsigned short int *)ExecRoutine);
	if (rc != IRIS_SUCCESS) {
		_ftprintf(stdout, _T("IRISRoutine�ݒ�G���[:%s\n"), getiriserror(rc));
		IRISEND();
		OutputExitCode(STS_RTNCALLERR);
		return STS_RTNCALLERR;
	}

	rc = IRISPUSHSTRW((int)wcslen(curdir), (unsigned short int *)curdir);
	if (rc != IRIS_SUCCESS) {
		_ftprintf(stdout, _T("IRIS���Dir�ݒ�G���[:%s\n"), getiriserror(rc));
		IRISEND();
		OutputExitCode(STS_RTNCALLERR);
		return STS_RTNCALLERR;
	}

	rc = IRISPUSHSTRW((int)wcslen(loadfile), (unsigned short int *)loadfile);
	if (rc != IRIS_SUCCESS) {
		_ftprintf(stdout, _T("IRIS�Ǎ��t�@�C���ݒ�G���[:%s\n"), getiriserror(rc));
		IRISEND();
		OutputExitCode(STS_RTNCALLERR);
		return STS_RTNCALLERR;
	}

	for (int i = 0; i < optc; i++)
	{
		rc = IRISPUSHSTRW((int)wcslen(optv[i]), (unsigned short int *)optv[i]);
		if (rc != IRIS_SUCCESS) {
			_ftprintf(stdout, _T("IRIS�p�����[�^�ݒ�G���[:%s\n"), getiriserror(rc));
			IRISEND();
			OutputExitCode(STS_RTNCALLERR);
			return STS_RTNCALLERR;
		}
	}

	// ���[�`�����s
	rc = IRISEXTFUN(rflags, optc+2);
	if (rc != IRIS_SUCCESS) {
		_ftprintf(stdout, _T("IRISRoutine���s�G���[:%s\n"), getiriserror(rc));
		IRISEND();
		OutputExitCode(STS_RTNCALLERR);
		return STS_RTNCALLERR;
	}

	// �߂�l�̎擾
	rc = IRISPOPINT(&retval);
	if (rc != IRIS_SUCCESS) {
		_ftprintf(stdout, _T("IRIS�߂�l�擾�G���[:%s\n"), getiriserror(rc));
		IRISEND();
		OutputExitCode(STS_RTNCALLERR);
		return STS_RTNCALLERR;
	}

	// IRIS�̏I��
	IRISEND();

	OutputExitCode(retval);
	return retval;
}

