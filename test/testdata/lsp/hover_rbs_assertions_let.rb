# typed: true
# enable-experimental-rbs-comments: true

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
