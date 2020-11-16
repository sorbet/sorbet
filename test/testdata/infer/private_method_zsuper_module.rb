# typed: true

module DefinesFoo
  def foo; Kernel.puts 'DefinesFoo#foo'; end
end

module MakesFooPrivate
  # Ruby errors right here!
  private :foo # error: No method called `foo` exists to be made `private` in `MakesFooPrivate`
end

# -- everything that happens after doesn't matter to me --

class A
  include DefinesFoo
  include MakesFooPrivate
end

A.new.foo # error: TODO(jez) figure out whether there should be an error here

class B
  include MakesFooPrivate
end

B.new.foo # error: TODO(jez) figure out whether there should be an error here
