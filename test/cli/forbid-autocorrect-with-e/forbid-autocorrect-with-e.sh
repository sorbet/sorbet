#!/bin/sh
exec main/sorbet --silence-dev-message --suggest-typed --autocorrect --typed=strict --error-white-list=7022 -e puts 2>&1
