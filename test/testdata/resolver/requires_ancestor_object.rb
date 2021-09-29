# typed: true
# enable-experimental-requires-ancestor: true

module Helper
  extend T::Helpers

  requires_ancestor { Object }

  def get_my_class_name
    self.class.name
  end
end

class Foo
  include Helper
end

class Bar < BasicObject # error: `Bar` must inherit `Object` (required by `Helper`)
  include Helper
end

class Baz < Bar # error: `Baz` must inherit `Object` (required by `Helper`)
  include Helper
end
