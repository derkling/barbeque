#!/bin/bash

if [ $# -lt 2 ]; then
	echo -e "Usage: $0 <configuration_file> <plugins_dir>\n\n"
	exit -1
fi

# Clean-up the tmp directory
rm -rf /tmp/bbque/*

# Update required plugins
cp plugins/rpc/libplugin_rpc_fifo.so $2/
cp plugins/syncpol/libplugin_syncpol_sasb.so $2/
cp plugins/schedpol/libplugin_schedpol_yamca.so $2/
cp plugins/schedpol/libplugin_schedpol_random.so $2/

clear; strace -xfe open,close,read,write -e read=3 -e write=5 ./bbque/barbeque  -c $1 -p $2
#clear; strace -fe open,close ./bbque/barbeque  -c $1 -p $2

