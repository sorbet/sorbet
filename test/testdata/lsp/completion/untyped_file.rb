# typed: false

class AnImportantClass
  def important_method(important_parameter)
    important
           # ^ completion: (file is not `# typed: true` or higher)
  end
end

An # error: Unable to resolve constant
# ^ completion: AnImportantClass

AnImportantClass.n
#                 ^ completion: (file is not `# typed: true` or higher)
