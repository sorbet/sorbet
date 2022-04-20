# typed: false

class AnImportantClass
  def important_method(important_parameter)
    important
           # ^ completion: (nothing)
  end
end

An # error: Unable to resolve constant
# ^ completion: (nothing)

AnImportantClass.n
#                 ^ completion: (nothing)

