# typed: true

A = 1
A ||= 2 # error: Constant reassignment is not supported

B = 1
B &&= 2 # error: Constant reassignment is not supported

C = 1
C += 2 # error: Constant reassignment is not supported
