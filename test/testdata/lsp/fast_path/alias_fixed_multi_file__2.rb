# typed: true

class A
  alias_method :from, :to_bad # error: Can't make method alias from `from` to non existing method `to_bad`
end
