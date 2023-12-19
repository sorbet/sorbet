# typed: false

# enable-typed-false-completion-nudges: false

class AnImportantClass
  def important_method(important_parameter)
    important
           # ^ completion: (nothing)
  end
end

An # error: Unable to resolve constant
# ^ completion: AnImportantClass

AnImportantClass.n
#                 ^ completion: (nothing)
