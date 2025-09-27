# typed: true

extend T::Sig

sig { params(name: String).returns(T.nilable(String)) }
def foo(name)
end


sig { params(other_name: String).returns(T.nilable(String)) }
#            ^^^^^^^^^^ error: Unknown parameter name `other_name`
def foo(name)
#       ^^^^ error: Malformed `sig`. Type not specified for parameter `name`
end

def bar(name)
end

sig { params(other_name: String).returns(T.nilable(String)) }
#            ^^^^^^^^^^ error: Unknown parameter name `other_name`
def bar(name)
#       ^^^^ error: Malformed `sig`. Type not specified for parameter `name`
end

def qux(name)
end

sig { returns(T.nilable(String)) }
def qux(name)
#       ^^^^ error: Malformed `sig`. Type not specified for parameter `name`
end
