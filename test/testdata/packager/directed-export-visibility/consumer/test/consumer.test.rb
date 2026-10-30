# typed: strict
# stratum: 2
module Test::Consumer
  Test::Producer::Helper
# ^^^^^^^^^^^^^^ importusage: producer-pkg
  #               ^^^^^^ usage: helper
end
