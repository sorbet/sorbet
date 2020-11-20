# typed: true

class A
  # TODO(jez) See if you can reproduce this with the Mutator DSL too
  define_method(:foo=) do |arg0|
  end
  private :foo=
end
