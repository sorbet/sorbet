# typed: strict

extend T::Sig

#: -> String
def foo
  x = ARGV.first #: String
end
