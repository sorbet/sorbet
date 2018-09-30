# typed: true
extend T::Helpers

sig {returns(Class)}
def foo
    Class
end

foo.new
