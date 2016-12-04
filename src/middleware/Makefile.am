bin_PROGRAMS=conviot_middleware
conviot_middleware_SOURCES=CentralManager.c \
						   ThingManager.c \
						   MQTTMessageHandler.c \
						   InfoManager.c \
						   AppManager.c \
						   DBHandler.c \
                           ConviotMiddleware.c 

#TODO
#change MYSQL with automake flag later
conviot_middleware_CFLAGS= $(PTHREAD_CFLAGS) $(JSONC_CFLAGS) -I/usr/include/mysql -I../include -I../include/capcommon
conviot_middleware_LDADD= $(PTHREAD_LIBS) $(JSONC_LIBS) -L/usr/lib/x86_64-linux-gnu -lmysqlclient -lconfig ../common/libcommon.a  ../common/capcommon/libcapcommon.a

