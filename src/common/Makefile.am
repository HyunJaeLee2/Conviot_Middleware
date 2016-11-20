noinst_LIBRARIES = libcommon.a
libcommon_a_SOURCES = MQTT_common.c\
                    Json_common.c

libcommon_a_CFLAGS = -I../include/capcommon -I../include
libcommon_a_LIBADD = $(PTHREAD_LIBS) capcommon/libcapcommon.a	
