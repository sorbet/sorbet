# typed: true

# This test prevents against a bug where Sorbet needs to know the types of
# fields (instance variables) defined on a parent class before resolving the
# identifiers in a subclass.
#
# See also the respective *__02.rb file, which will show up lower in the file
# list when this test runs.

class Child < Parent
  def as_of
    T.reveal_type(@as_of) # error: Revealed type: `Integer`
  end
end
