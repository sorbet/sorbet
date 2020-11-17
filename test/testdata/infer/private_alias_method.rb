# typed: true

class A
  def public_foo; puts 'in public_foo'; end

  private :private_foo
  alias_method :private_foo, :public_foo

  def private_ok_here
    public_foo
    private_foo
  end

  def inspect
    '#<A:...>'
  end
end

a2 = A.new
puts '-- private ok here --'
a2.private_ok_here
puts '-- public ok --'
a2.public_foo
puts '-- private not ok --'
begin
  a2.private_foo # error: Non-private call to private method `A#private_foo`
rescue NoMethodError => exn
  puts exn.message
end
