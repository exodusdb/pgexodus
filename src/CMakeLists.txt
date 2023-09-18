# Copyright 2020 Mats Kindahl
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License. You may obtain a copy of
# the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.

# Will install the following files:
# See cmake's generated install_manifest.txt after building
# /usr/lib/postgresql/15/lib/pgexodus.so
# /usr/share/postgresql/15/extension/pgexodus.control
# /usr/share/postgresql/15/extension/pgexodus--1.0.sql

add_postgresql_extension(
  pgexodus
  VERSION 1.0
  SOURCES pgexodus.c extract.c
  SCRIPTS pgexodus--1.0.sql
  REGRESS basic
  SUPERUSER false)
