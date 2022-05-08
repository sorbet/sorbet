#!/usr/bin/env bash

main/sorbet --silence-dev-message --stripe-packages --max-threads=0 . 2>&1
