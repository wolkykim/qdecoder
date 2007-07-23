#ifndef _QDECODER_COMPAT_H
#define _QDECODER_COMPAT_H

#define	qValue		qGetValue
#define	qiValue		qGetInt
#define	qValueDefault	qGetValueDefault
#define	qValueNotEmpty	qGetValueNotEmpty
#define	qValueReplace	qGetValueReplace
#define	qValueFirst	qGetValueFirst
#define	qValueNext	qGetValueNext
#define	qValueAdd	qAdd
#define	qValueRemove	qRemove
#define	qValueType	qGetType
#define	qCookieValue	qCookieGetValue

#define qfValue		qfGetValue
#define qfiValue	qfGetInt
#define qfValueFirst	qfGetValueFirst
#define qfValueNext	qfGetValueNext

#define qSessionAddInteger	qSessionAddInt
#define qSessionUpdateInteger	qSessionUpdateInt
#define qSessionValue		qSessionGetValue
#define qSessionValueInteger	qSessionGetInt

#define qsValue		qsGetValue
#define qsiValue	qsGetInt
#define qsValueFirst	qsGetValueFirst
#define qsValueNext	qsGetValueNext

#define	qURLencode	qUrlEncode
#define	qURLdecode	qUrlDecode
#define	qMD5Str		qMd5Str
#define	qMD5File	qMd5File

#define	qfopen		qFileOpen
#define	qfclose		qFileClose
#define	qfGets		qFileReadString
#define	qfGetLine	qFileReadLine

#define	qCheckURL	qCheckUrl

#define	qSedArgAdd		qSedAdd
#define	qSedArgAddDirect	qSedAddDirect
#define	qSedArgPrint		qSedPrint
#define	qSedArgFree		qSedFree

#define	qGetGMTime		qGetGmtime

#endif
