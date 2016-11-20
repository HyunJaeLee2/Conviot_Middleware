#ifndef CAPIOT_COMMON_H_
#define CAPIOT_COMMON_H_


#include <capcommon/cap_common.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum _ETopicLevel {
    TOPIC_LEVEL_FIRST,
    TOPIC_LEVEL_SECOND,
    TOPIC_LEVEL_THIRD,
} ETopicLevel;


typedef enum _EIoTHandleId {
	HANDLEID_SCENARIO               = 1,
	HANDLEID_THING_MANAGER          = 3,
	HANDLEID_JOB_HANDLER            = 5,
	HANDLEID_INFO_MANAGER          = 6,
    HANDLEID_CENTRAL_MANAGER        = 11,
	HANDLEID_JOB_MANAGER            = 12,
} EIoTHandleId;

#define ERROR_TYPE_INFORMATION 		0x01000000
#define ERROR_TYPE_ERROR		0x02000000
#define ERROR_TYPE_CRITICAL		0x03000000

#define MODULE_COMMON			0x00000000
#define MODULE_SCENARIO_HANDLER		0x00100000
#define MODULE_THING_HANDLER		0x00200000
#define MODULE_JOB_MANAGER		0x00300000
#define MODULE_THING_MANAGER		0x00400000
#define MODULE_SCENARIO_MANAGER		0x00500000

//TODO
//author : hyunjae
//should change level1 & leve2 to 1, 2. Setted as 0 for testing purpose at the moment
#define QOS_LEVEL_0 (0)
#define QOS_LEVEL_1 (1)
#define QOS_LEVEL_2 (2)

#define SAFEJSONFREE(mem) if((mem) != NULL){json_object_put((mem));mem=NULL;}

extern cap_bool g_bExit;
extern cap_handle g_hLogger;

#ifdef __cplusplus
}
#endif

#endif /* CAPIOT_COMMON_H_ */
