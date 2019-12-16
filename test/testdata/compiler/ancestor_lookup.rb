# typed: true
module M
  def foo
    1
  end
end

class A
  include M
end

puts A.ancestors.inspect
