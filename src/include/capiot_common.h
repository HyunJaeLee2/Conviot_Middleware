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
    TOPIC_LEVEL_FOURTH,
    TOPIC_LEVEL_FIFTH,
} ETopicLevel;

#define HANDLID_GROUP_MIDDLEWARE (0x1000)

typedef enum _EIoTHandleId {
	HANDLEID_SCENARIO               = HANDLID_GROUP_MIDDLEWARE | 1,
	HANDLEID_THING_MANAGER,
	HANDLEID_APP_RUNNER,
	HANDLEID_INFO_MANAGER,
	HANDLEID_CENTRAL_MANAGER,

	HANDLEID_APP_MANAGER,
	HANDLEID_MQTT_MESSAGE_HANDLER,
	HANDLEID_APP_DATA_CACHE,
	HANDLEID_APP_ENGINE,
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
