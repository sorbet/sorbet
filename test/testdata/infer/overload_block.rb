# @typed
def main
    a = [1,2,3].map do |x|
        x
    end
    T.assert_type!(a, Array)
end
