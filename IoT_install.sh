#!/bin/sh 
LIST_OF_APPS="mosquitto mosquitto-clients libtool automake autoconf sqlite3 libsqlite3-dev libssl-dev flex byacc bison libconfig-dev"

#install apps
sudo apt-get install $LIST_OF_APPS -y
if [ "$?" != "0" ]; then
    echo "[Error] applications install failed!" 1>&2
    exit 1
fi

# install json-c
git clone https://github.com/json-c/json-c.git
if [ "$?" != "0" ]; then
    echo "[Error] json-c install failed!" 1>&2
    exit 1
fi

sudo ln -sf /usr/share/libtool/config/ltmain.sh .
 
cd json-c
sh autogen.sh
./configure
if [ "$?" != "0" ]; then
    echo "[Error] json-c configure failed!" 1>&2
    exit 1
fi


make -j`nproc`
if [ "$?" != "0" ]; then
    echo "[Error] json-c make failed!" 1>&2
    exit 1
fi

sudo make install
if [ "$?" != "0" ]; then
    echo "[Error] json-c make install failed!" 1>&2
    exit 1
fi

make check
if [ "$?" != "0" ]; then
    echo "[Error] json-c make check failed!" 1>&2
    exit 1
fi

cd ..

# install mqtt library
git clone https://github.com/eclipse/paho.mqtt.c.git
if [ "$?" != "0" ]; then
    echo "[Error] paho install failed!" 1>&2
    exit 1
fi

cd paho.mqtt.c
make -j`nproc`
if [ "$?" != "0" ]; then
    echo "[Error] paho make failed!" 1>&2
    exit 1
fi

sudo make install
if [ "$?" != "0" ]; then
    echo "[Error] paho make install failed!" 1>&2
    exit 1
fi
