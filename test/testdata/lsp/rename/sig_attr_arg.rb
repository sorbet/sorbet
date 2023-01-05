# typed: true

# The behavior on this test is terrible, but that's because
# find-all-references is terrible on attr_reader:
#
# https://github.com/sorbet/sorbet/issues/4093
#
# When we fix that bug, this test should magically start
# working. In the mean time, I'm not too worried about
# blocking on fixing that to support this for normal methods.

class A
  extend T::Sig

  sig {params(x: T.nilable(Integer)).returns(T.nilable(Integer))}
  #           ^ apply-rename: [A] newName: target placeholderText: x
  attr_writer :x
  #            ^ apply-rename: [B] newName: target placeholderText: @x

  sig {returns(T.nilable(Integer))}
  attr_accessor :y
  #              ^ apply-rename: [C] newName: target placeholderText: @y
end
