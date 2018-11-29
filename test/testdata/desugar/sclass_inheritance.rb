# typed: true
module MM; end

class A
    class << self
        include MM

        def newer
            new
        end
    end
end

class B
    extend MM

    def self.newer
        new
    end
end

def main
    puts A.newer
    puts B.newer
end
main
