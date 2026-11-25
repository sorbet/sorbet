#!/bin/bash
sed -n 's/#define SORBET_VERSION_MAJOR_MINOR "\([^"]*\)"/\1/p' sorbet_version/sorbet_version.h
