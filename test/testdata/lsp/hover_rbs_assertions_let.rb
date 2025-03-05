# typed: true
# enable-experimental-rbs-signatures: true
# enable-experimental-rbs-assertions: true

  x = 42 #: Integer
# ^ hover: Integer
#        ^ hover: null
#         ^ hover: null
#          ^ hover: null
#           ^ hover: T.class_of(Integer)
#            ^ hover: T.class_of(Integer)
#             ^ hover: T.class_of(Integer)
#              ^ hover: T.class_of(Integer)
#               ^ hover: T.class_of(Integer)
#                ^ hover: T.class_of(Integer)
#                 ^ hover: T.class_of(Integer)

class Foo
  def foo
    @x ||= 42 #: Integer?
  end

  def bar
    puts @x
       # ^ hover: T.nilable(Integer)
  end
end
