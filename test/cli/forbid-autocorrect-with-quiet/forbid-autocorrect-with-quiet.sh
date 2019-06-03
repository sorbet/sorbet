#!/bin/sh
exec main/sorbet --silence-dev-message --quiet --autocorrect input.rb 2>&1
