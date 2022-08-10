# typed: true

class Test
  struct # error: does not exist
  #     ^ apply-completion [A] item: 0
end
