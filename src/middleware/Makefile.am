bin_PROGRAMS=conviot_middleware
conviot_middleware_SOURCES=CentralManager.c \
						   ThingManager.c \
						   MQTTMessageHandler.c \
                           ConviotMiddleware.c 

conviot_middleware_CFLAGS= $(PTHREAD_CFLAGS) $(JSONC_CFLAGS) -I../include -I../include/capcommon
conviot_middleware_LDADD= $(PTHREAD_LIBS) $(SQLITE3_LDFLAGS) $(JSONC_LIBS) -lconfig ../common/libcommon.a  ../common/capcommon/libcapcommon.a

