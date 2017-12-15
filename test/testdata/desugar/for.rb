# @typed

class A
    def self.each
        yield 1,2,3,4,5
        yield 6,7,8,9,0
    end
end

class E
    def self.e=(e)
        @e = e; # error: Use of undeclared variable `@e'
    end
    def self.e
        @e
    end
end

class Main
    def self.main

        for a in A do
            puts a.inspect
        end
        A.each do |*forTemp|
            a, = *forTemp
            puts a.inspect
        end

        for a,b in A do
            puts a.inspect
            puts b.inspect
        end
        A.each do |*forTemp|
            a,b = *forTemp
            puts a.inspect
            puts b.inspect
        end
        puts "main"
        # You can put all sorts of things on the left
        for @a,@@b,$c,d,E.e in A do # error: MULTI
            puts @a.inspect
            puts @@b.inspect
            puts $c.inspect
            puts d.inspect
            puts E.e.inspect
        end
        A.each do |*forTemp|
            @a,@@b,$c,d,E.e = *forTemp
            puts @a.inspect
            puts @@b.inspect
            puts $c.inspect
            puts d.inspect
            puts E.e.inspect
        end
    end
end
Main.main
