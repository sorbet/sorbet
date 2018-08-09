# typed: strict

class A
  def self.baz(a)
  end
end

def foo; end

begin
rescue
ensure
  A.baz(
    foo&.bar
  )
end
