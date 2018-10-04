# typed: true
class A
end
def main
    A.a # error: Method `a` does not exist on `T.class_of(A)`
end
main
