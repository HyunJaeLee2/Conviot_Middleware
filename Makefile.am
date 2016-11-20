#!/bin/bash
SUBDIRS=src/common/capcommon src/common src/middleware

middleware :
	cd src/common; make 
	cd src/application/middleware; make 

tags :
	find . \( -name "*.c" -o -name "*.h" -o -name "*.y" \) > cscope.files
	cscope

