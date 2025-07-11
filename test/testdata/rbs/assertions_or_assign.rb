# typed: strict
# enable-experimental-rbs-comments: true

# let

let1 ||= "foo" #: String?
T.reveal_type(let1) # error: Revealed type: `T.nilable(String)`

let2 ||= ("foo") #: Integer
                 #  ^^^^^^^ error: Argument does not have asserted type `Integer`
T.reveal_type(let2) # error: Revealed type: `Integer`

let3 = "" #: String?
(let3 ||= "foo") #: Integer
                 #  ^^^^^^^ error: Argument does not have asserted type `Integer`
T.reveal_type(let3) # error: Revealed type: `String`

class DesugaredLetNilable
  #: -> Integer
  def foo
    @x ||= 42 #: Integer?
    T.reveal_type(@x) # error: Revealed type: `Integer`
  end

  #: -> void
  def bar
    @y ||= nil #: Integer?
    @y ||= 42
    T.reveal_type(@y) # error: Revealed type: `Integer`
  end

  #: -> void
  def baz
    T.reveal_type(@x) # error: Revealed type: `T.nilable(Integer)`
    T.reveal_type(@y) # error: Revealed type: `T.nilable(Integer)`
  end
end
