# typed: strict
# spacer for exclude-from-file-update

class Foo
end

T.reveal_type(Foo) # error: `T.class_of(Foo)`
