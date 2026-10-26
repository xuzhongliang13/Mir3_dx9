

#ifndef __ORZ_SCREEN_CAPTURE__
#define __ORZ_SCREEN_CAPTURE__


#include <ddraw.h>

class CDX9SoftSurface;

/*
	scScreenCapture7
	- ���� DirectDraw ���ǽ��� BMP�� �����ϴ� �Լ�

	IDirectDrawSurface7 *surface
	- DirectDrawSurface7 �������̽�

	char *dir
	- ������ ���丮 (������ SCREEN0001.BMP ������ ����ȴ�.)
	��1) �ݵ�� �����ϰ� �ִ� ���丮���� �Ѵ�.
	��2) �⺻ ���� NULL�� ����ϸ� ���� ���丮�� ����ȴ�. (����õ)
	     �� ��� C:\ ����̺꿡�� �ٷ� �����ߴٸ� 
		 C:\SCREEN0001.BMP ���� ������ ��������Ƿ� �����ؾ� �Ѵ�.
*/
void scScreenCapture7( CDX9SoftSurface *surface, char *dir = NULL );

/*
	scIsValid
	- ���۵� �������� �ƴ��� �Ǻ��ϴ� �Լ�

	char *filename
	- �˻��� ��Ʈ�� ���ϸ�
*/
bool scIsValid( char *filename );


#endif