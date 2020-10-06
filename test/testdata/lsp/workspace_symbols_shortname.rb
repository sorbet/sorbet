# typed: true

def bar
  # ^^^ symbol-search: "bar"
end

class Foo
  #   ^^^ symbol-search: "Foo"
  #   ^^^ symbol-search: "foo"
  def bar
    # ^^^ symbol-search: "bar"
  end
end

module Attrs
  class Reader
    attr_reader :bar
    #            ^^^ symbol-search: "bar", name="Attrs::Reader#bar"
  end

  class Writer
    attr_writer :bar
    #            ^^^ symbol-search: "bar", name="Attrs::Writer#bar="
  end
end

module Inner
  def bar
    # ^^^ symbol-search: "bar"
  end

  module Foo
    #    ^^^ symbol-search: "Foo"
    #    ^^^ symbol-search: "foo"
    def bar
      # ^^^ symbol-search: "bar"
    end
  end
end
