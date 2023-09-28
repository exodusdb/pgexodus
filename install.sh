#!/bin/bash
set -euxo pipefail
:
: --------------------------------
: Build, install and test pgexodus
: --------------------------------
:
: Get dependencies
: ----------------
:
	apt update
	apt install cmake git postgresql postgresql-server-dev-all
:
: Build
: -----
:
	rm build -rf
	mkdir build
	cmake . -B build
	cmake --build build
:
: Install
: -------
:
	cmake --install build
:
: Test
: ----
:
	cd build
	ctest
:
: The extension is now available to the above postgres installation.
:
: Exodus installation will load the extension into each database using
: "create extension pgexodus;"
: A quick self test will be performed as part of every loading.
:
