# typed: strict
# stratum: 1
module Consumer
  Producer::Public
# ^^^^^^^^ importusage: producer-pkg
  #         ^^^^^^ usage: public
  Producer::Alias::Nested
# ^^^^^^^^ importusage: producer-pkg
  Producer::Value
# ^^^^^^^^ importusage: producer-pkg
  T.let(1, Producer::Type)
  #        ^^^^^^^^ importusage: producer-pkg
end
