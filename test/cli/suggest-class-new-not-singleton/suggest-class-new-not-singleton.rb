# typed: true

T.class_of(String).new
T.class_of(Integer).new
T.class_of(T::Array).new
T.class_of(T::Array[String]).new
x = T.class_of(String)
x.new
