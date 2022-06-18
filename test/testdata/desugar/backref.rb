# typed: true

x1 = $~ # is equivalent to ::last_match;
T.reveal_type(x1) # error: Revealed type: `T.untyped`

x2 = $& # contains the complete matched text;
T.reveal_type(x2) # error: Revealed type: `T.nilable(String)`

x3 = $` # contains string before match;
T.reveal_type(x3) # error: Revealed type: `T.nilable(String)`

x4 = $' # contains string after match;
T.reveal_type(x4) # error: Revealed type: `T.nilable(String)`

x5 = $+ # contains last capture group;
T.reveal_type(x5) # error: Revealed type: `T.nilable(String)`

x6 = $1 # contains the first match group;
T.reveal_type(x6) # error: Revealed type: `T.nilable(String)`

x7 = $9 # contains the 9th match group;
T.reveal_type(x7) # error: Revealed type: `T.nilable(String)`

class TestNotPinningBackrefs
  /foo/.match("foo") do |m|
    x = $&
    T.reveal_type(x) # error: Revealed type: `T.nilable(String)`
    x = false
    T.reveal_type(x) # error: Revealed type: `FalseClass`
  end
end
