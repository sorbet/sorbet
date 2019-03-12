# typed: true

def gen(_arg)
end

def foo_gen
end

def bar_gen
end

def baz_gen
end

module Hi
  gen :bird
  class << self
    gen :fish
    class Hello
      gen :rabbit
    end
    gen :cat
  end
  gen :penguin
end
