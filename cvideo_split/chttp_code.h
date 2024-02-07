/********************************************************************
created:	2021-11-24

author:		chensong

purpose:	cdecoderProxy

*********************************************************************/

#ifndef  _C_HTTP_CODE_H_
#define  _C_HTTP_CODE_H_



namespace chen {

	enum EWebCode
	{
		EWebSuccess = 0,
		EWebDb = 500, //  ���ݿ����
		EWebNotFindAuthId, // 
		EWebNotOnline, // �ӽڵ㲻����
		EWebAllocMemFailed, // 
		EWebAppRuning, // app �Ѿ�������
		EWebAppNotRun, // app û������
		EWebAppStartFailed, // app ����ʧ��  
		EWebAppInjectorFailed, // app����ʱ��ע��ʧ��
		EWebJsonError = 600,  // json��ʽ����
		EWebJsonParam,  // json�ֶν�������
		EWebRenderServerOffline, //��Ⱦ��������
		EWebRenderServerId, // ��Ⱦ����ID����
		EWebRenderServerMaxApp, //���������Ⱦapp�ĸ���
		EWebNotFindUserId, // û���û�
		EWebSaveRenderApp, // ��ʽ����
		EWebNotFindRenderApp, // app
		EWebRenderAppStaring, // app�Ѿ���������
		EWebRenderAppStop, //app�Ѿ�ֹͣ��
		EWebWait,   // ����һ��
		EWebNotFindCameraId, //û�и������ID
		EWebNotFindVideoSplitId, //û�и���Ƶƴ��ID
		
	};
}


#endif // _C_HTTP_CODE_H_