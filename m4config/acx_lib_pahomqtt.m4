AU_ALIAS([LIB_PAHO_MQTT], [ACX_LIB_PAHO_MQTT])
AC_DEFUN([ACX_LIB_PAHO_MQTT],
[
	PAHO_LIBS=-lpaho-mqtt3c
	AC_CHECK_LIB([paho-mqtt3c], [MQTTClient_create], 
		AC_MSG_RESULT([MQTT Paho C Client Library is found])
		LIBS="$PAHO_LIBS $LIBS", AC_MSG_ERROR([MQTT Paho C Client Library is not installed!]))
])
