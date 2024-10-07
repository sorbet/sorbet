# typed: false

undef m1 # Works with bare references

undef :m2 # Works with Symbols

undef :m3, m4 # Works with multiple arguments
