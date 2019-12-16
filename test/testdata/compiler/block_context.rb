# typed: true
class Foo
end

Foo.new.instance_exec {puts self.class}
