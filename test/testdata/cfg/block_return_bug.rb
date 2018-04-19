# typed: strict
class Foo
    def bla(arr)
      bla(arr) || (arr.each {|item| return :invalid_expandable_array if item.empty?}; nil)
      # wrong order of blocks here can trigger a crash in inferencer
    end
end
