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
		EWebDb = 500, //  数据库错误
		EWebNotFindAuthId, // 
		EWebNotOnline, // 子节点不在线
		EWebAllocMemFailed, // 
		EWebAppRuning, // app 已经运行了
		EWebAppNotRun, // app 没有运行
		EWebAppStartFailed, // app 启动失败  
		EWebAppInjectorFailed, // app启动时的注入失败
		EWebJsonError = 600,  // json格式错误
		EWebJsonParam,  // json字段解析错误
		EWebRenderServerOffline, //渲染服务不在线
		EWebRenderServerId, // 渲染服务ID错误
		EWebRenderServerMaxApp, //超出最大渲染app的个数
		EWebNotFindUserId, // 没有用户
		EWebSaveRenderApp, // 格式错误
		EWebNotFindRenderApp, // app
		EWebRenderAppStaring, // app已经运行中了
		EWebRenderAppStop, //app已经停止了
		EWebWait,   // 再试一次
		EWebNotFindCameraId, //没有该摄像机ID
		EWebNotFindVideoSplitId, //没有该视频拼接ID
		
	};
}


#endif // _C_HTTP_CODE_H_