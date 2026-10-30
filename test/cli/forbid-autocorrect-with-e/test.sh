#!/bin/sh

if main/sorbet --silence-dev-message --suggest-typed --autocorrect --typed=strict --isolate-error-code=7022 -e puts 2>&1; then
  >&2 echo "Expected to fail!"
fi

if main/sorbet --silence-dev-message --suggest-typed --autocorrect --typed=strict --isolate-error-code=7022 --e-rbi puts 2>&1; then
  >&2 echo "Expected to fail!"
fi
