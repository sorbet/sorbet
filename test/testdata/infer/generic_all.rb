# typed: true

module One
  extend T::Sig

  module A; end

  module B; end

  sig {params(x: Integer).returns(T.all(A, B))}
  def return_ab(x)
    return_ab(x)
  end

  def main
    ab_list = [1,2,3].map {|i| return_ab(i)}
    T.reveal_type(ab_list) # error: Revealed type: `T::Array[T.all(One::A, One::B)]`
  end
end

module Two
  extend T::Sig

  class Parent; end
  module Interface; end
  class Child < Parent
    include Interface
  end

  def bar
    x = T.let([[:child, Child.new]], T::Array[[Symbol, T.all(Interface, Parent)]])
    T.reveal_type(x.to_h) # error: Revealed type: `T::Hash[Symbol, T.all(Two::Interface, Two::Parent)]`
  end
end

module Three
  module WhateverInterface; end
  module WhyInterface; end
  module HelpInterface; end

  class Test
    include WhateverInterface
    include WhyInterface
    include HelpInterface
  end

  class A
    extend T::Sig

    Everything = T.type_alias do 
      T.all(
        WhateverInterface,
        WhyInterface,
        HelpInterface,
      )
    end
    sig {returns(T::Array[Everything])}
    def something
      arr = T.let([Test.new], T::Array[Everything])
      results = arr.map do |elem|
        elem
      end
      results
    end
  end
end
