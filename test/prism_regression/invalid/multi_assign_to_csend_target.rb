# typed: false

self&.target1, self&.target2 = []
#   ^^ error: &. inside multiple assignment
#                  ^^ error: &. inside multiple assignment
