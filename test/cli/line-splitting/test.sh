#!/bin/bash
main/sorbet --silence-dev-message -e '
 foo(1 +
  2 +
  3 +
  4 +
  5 +
  6 +
  7 + 
  8 +
  9 +
  10 +
  11 +
  12 +
  13 + 
  14)

 foo(1 +
  2 +
  3 +
  4 +
  5 +
  6 +
  7 + 
  8 +
  9)

 foo(1 +
  2 +
  3 +
  4 +
  5 +
  6 +
  7 + 
  8 +
  9 +
  10)

 foo(1 +
  2 +
  3 +
  4 +
  5 +
  6 +
  7 + 
  8 +
  9 +
  10 +
  11)
' 2>&1

main/sorbet --silence-dev-message -e '
# typed: true
extend T::Sig

sig {returns(String)}
def returns_string; ""; end

sig {params(x: Integer).void}
def takes_integer(x); end

takes_integer(
  returns_string( # 1
                  # 2
                  # 3
                  # 4
                  # 5
                  # 6
                  # 7
                  # 8
                  # 9
                  # 10
                  # 11
                  # 12
                  # 13
  )               # 14
)

takes_integer(
  returns_string( # 1
                  # 2
                  # 3
                  # 4
                  # 5
                  # 6
                  # 7
                  # 8
  )               # 9
)

takes_integer(
  returns_string( # 1
                  # 2
                  # 3
                  # 4
                  # 5
                  # 6
                  # 7
                  # 8
                  # 9
  )               # 10
)

takes_integer(
  returns_string( # 1
                  # 2
                  # 3
                  # 4
                  # 5
                  # 6
                  # 7
                  # 8
                  # 9
                  # 10
  )               # 11
)
' 2>&1
