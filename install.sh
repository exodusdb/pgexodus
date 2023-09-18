#!/bin/bash
set -euxo pipefail

	apt update
	apt install cmake git postgresql postgresql-server-dev-1*
	rm build -rf
	mkdir build
	cd build
	cmake ..
	make
	make install
	make test
