#!/bin/bash
main/sorbet --silence-dev-message --censor-for-snapshot-tests \
  --e-rbi $'
    module Foo
      sig { returns(T::Boolean) }
      sig { params(x: Integer).returns(Integer) }
      def self.foo(x); end;
    end
  ' \
  -e $'Foo.foo("abc")' 2>&1
