# typed: strict
class Main
    def yielder
        a = yield 1
        yield a
    end
    def blockpass(&blk)
        a = blk.call 1
        blk.call a
    end
    def mixed(&blk)
        a = yield 1
        blk.call a
    end
    def blockyield
        yielder {|i| yield i}
    end

    def main
        l = lambda {|x| puts x; 3}
        yielder &l
        blockpass &l
        mixed &l
        blockyield &l
    end
end
Main.new.main
