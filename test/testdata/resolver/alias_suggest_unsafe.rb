# typed: true
# enable-suggest-unsafe: true

class Child
  alias_method :bar, :foo
  #                  ^^^^ error: Can't make method alias from `bar` to non existing method `foo`
end
