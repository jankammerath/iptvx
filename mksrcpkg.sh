#!/bin/bash
tar --transform "s,./,$1/," -zcvf $1.tar.gz ./app/ ./cfg/ ./src/ ./data/db ./Makefile ./iptvx.service