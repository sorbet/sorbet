# typed: true
# enable-suggest-unsafe: true

class Parent < T::Struct
end

class Child < Parent # error: Subclassing `Parent` is not allowed
end

# autocorrect not implemented
Child2 = Class.new(Parent) do # error: Subclassing `Parent` is not allowed
end
