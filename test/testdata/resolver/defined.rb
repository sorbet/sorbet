# typed: true
class Foo
end
T.reveal_type(defined?(Foo)) # error: T.nilable(String)
