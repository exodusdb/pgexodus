#!/bin/bash
set -euxo pipefail

	apt update
	apt install cmake git postgresql postgresql-server-dev-all

	rm build -rf
	mkdir build

	cmake . -B build
	cmake --build build
	cmake --install build
:
: The extension is now available to the above postgres installation.
:
: Exodus installation will load the extension into each database using
: "create extension pgexodus;"
: A quick self test will be performed as part of every loading.
:
