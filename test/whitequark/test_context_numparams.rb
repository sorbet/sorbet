# typed: strict

->(a) { module M; _1; end } # Method _1 does not exist on T.class_of(M)
-> { class C; _1; end } # Method _1 does not exist on T.class_of(C)
-> { def foo; _1; end } # Method _1 does not exist on T.class_of(C)
