// stdafx.h : �W���̃V�X�e�� �C���N���[�h �t�@�C���̃C���N���[�h �t�@�C���A�܂���
// �Q�Ɖ񐔂������A�����܂�ύX����Ȃ��A�v���W�F�N�g��p�̃C���N���[�h �t�@�C��
// ���L�q���܂��B
//

#pragma once

#ifndef _WIN32_WINNT		// Windows XP �ȍ~�̃o�[�W�����ɌŗL�̋@�\�̎g�p�������܂��B                   
#define _WIN32_WINNT 0x0501	// ����� Windows �̑��̃o�[�W���������ɓK�؂Ȓl�ɕύX���Ă��������B
#endif						

#include <stdio.h>
#include <tchar.h>
#include <windows.h>

// TODO: �v���O�����ɕK�v�Ȓǉ��w�b�_�[�������ŎQ�Ƃ��Ă��������B
// �R�}���h���C���I�v�V����
#define OPT_FILENAME	0
#define OPT_CONFIGNAME	1
#define OPT_NAMESPACE	2
#define OPT_TIMER		3
#define OPT_AUTH		4

// �G���[�X�e�[�^�X
#define STS_SUCCESS			0
#define STS_INVBINDIRERR		1
#define STS_INVOPTION		2
#define STS_IOERR			3
#define STS_INVIRISSTATUS	4
#define STS_RTNCALLERR		5
#define STS_INVNAMESPACEERR	6
#define STS_BUSY			7

#define STS_EXITCODEBASE	16	

// ���s���[�`��
#define ExecRoutine	L"%ZLOADER"

