# typed: true
# enable-suggest-unsafe: true

class Parent < T::Struct
end

class Child < Parent # error: Subclassing `Parent` is not allowed
end

