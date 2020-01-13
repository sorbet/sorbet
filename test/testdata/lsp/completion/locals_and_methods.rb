# typed: true

class A
  def some_method_on_a; end

  def locals_always_first
    some_local = nil
    some_ # error: does not exist
    #    ^ completion: some_local, some_method_on_a
  end
end
