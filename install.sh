#!/bin/bash
set -euxo pipefail

	apt update
	apt install cmake git postgresql
	apt install postgresql-server-dev-all
	rm build -rf
	mkdir build
	cmake . -B build
	cmake --build build
	cmake --install build
