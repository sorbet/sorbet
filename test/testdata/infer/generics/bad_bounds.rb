# typed: true

class BadBounds1
  extend T::Generic

  TypeMember = type_member

  # TODO(jez) Report an error here
  TypeTemplate = type_template { {fixed: TypeMember} }
end

class BadBounds2
  extend T::Generic

  Out = type_member(:out)

  # TODO(jez) Report an error here
  # class BadBounds[+X]
  # class BadBoundsChild[-Y] extends BadBounds[Y]
  #     contravariant type Y occurs in covariant position in type Playground.BadBounds[Y] {...} of class BadBoundsChild
  In = type_member(:in) { {fixed: Out} }
end

# TODO(jez) flesh out this test too
# class BadBounds1[+X, -Y <: X] {
#   def foo(y: Y): X =
#   	val x: X = y
#   	x
# }
# class BadBounds2[+X, -Y >: X] {
#   def foo(y: Y): X =
#   	val x: X = y
#   	// Found:    (y : Y)
#   	// Required: X
#   	//
#   	// here:    X is a type in class BadBounds2 with bounds <: Y
#   	//          Y is a type in class BadBounds2 with bounds >: X
#   	x
# }
# class BadBounds3[-X, +Y <: X] {
#   def foo(x: X): Y =
#   	val y: Y = x
#   	// Found:    (x : X)
#   	// Required: Y
#   	// 
#   	// where:    X is a type in class BadBounds3 with bounds >: Y
#   	//           Y is a type in class BadBounds3 with bounds <: X
#   	y
# }
# class BadBounds4[-X, +Y >: X] {
#   def foo(x: X): Y =
#   	val y: Y = x
#   	y
# }


