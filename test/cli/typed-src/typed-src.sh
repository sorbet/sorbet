#!/bin/bash

main/sorbet --silence-dev-message --typed-source=a.rb test/cli/typed-src/*.rb
