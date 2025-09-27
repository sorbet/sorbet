# typed: true

extend T::Sig

sig { params(name: String).returns(T.nilable(String)) }
#            ^^^^ error: Malformed `sig`. Type not specified for argument `name`
def foo(name)
end


sig { params(other_name: String).returns(T.nilable(String)) }
#            ^^^^^^^^^^ error: Unknown argument name `other_name`
def foo(name)
end

def bar(name)
end

sig { params(other_name: String).returns(T.nilable(String)) }
#            ^^^^^^^^^^ error: Unknown argument name `other_name`
def bar(name)
#       ^^^^ error: Malformed `sig`. Type not specified for argument `name`
end

def qux(name)
end

sig { returns(T.nilable(String)) }
def qux(name)
#       ^^^^ error: Malformed `sig`. Type not specified for argument `name`
end
