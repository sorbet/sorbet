# typed: strict

module Root::Ignored
  STUFF = T.let(:ignored_stuff, Symbol)
end

# this is fine because we're deliberately ignoring the owned directory
AnotherConstant = T.let(:another_constant, Symbol)
