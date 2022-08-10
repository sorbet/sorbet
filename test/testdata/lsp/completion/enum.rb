# typed: true
class Test
  enum # error: does not exist
  #   ^ apply-completion [A] item: 0
end
