#!/bin/bash

if [ -f "../../quagmire/libss2x.so.1.0.0" ]; then
	echo "copying to Quagmire project"
	cp libss2x.so.1.0.0 ../../quagmire/.
	cp *.h ../../quagmire/inc/.
fi
if [ -f "../../little-secrets/libss2x.so.1.0.0" ]; then
	echo "copying to Little Secrets project"
	cp libss2x.so.1.0.0 ../../little-secrets/.
	cp *.h ../../little-secrets/inc/.
fi
if [ -f "../../net/libss2x/libss2x.so.1.0.0" ]; then
	echo "copying to Network project"
	cp libss2x.so.1.0.0 ../../net/libss2x/.
	cp *.h ../../net/libss2x/.
fi

