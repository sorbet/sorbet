# typed: strict

extend T::Sig

sig { returns(String) }
def my_method
  ""
end

my_me # error: Method `my_me` does not exist on `T.class_of(<root>)`
#    ^ apply-completion: [A] item: 0
