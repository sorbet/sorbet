module B
end

module A
  # In real Ruby this will be `B::C` but we make it `A::B::C`
  class B::C
  end
end
