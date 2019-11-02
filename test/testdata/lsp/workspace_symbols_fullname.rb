# typed: true

def foo
  # ^^^ symbol-search: "foo"
end

class Foo
  #   ^^^ symbol-search: "Foo"
  #   TODO: support case-insensitive matches ^^^ symbol-search: "foo"
  def foo
    # ^^^ symbol-search: "foo"
  end
end

module Bar
  def foo
    # ^^^ symbol-search: "foo"
  end

  module Foo
    #    ^^^ symbol-search: "Foo"
    #    TODO: support case-insensitive matches ^^^ symbol-search: "foo"
    def foo
      # ^^^ symbol-search: "foo"
    end
  end
end
