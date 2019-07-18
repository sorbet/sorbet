#!/bin/bash
main/sorbet --silence-dev-message --censor-raw-locs-within-payload -e 'class Foo; end' -p symbol-table 2>&1
main/sorbet --silence-dev-message --censor-raw-locs-within-payload -e $'# typed: strong\ndef foo; end' 2>&1
