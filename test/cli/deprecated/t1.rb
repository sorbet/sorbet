# typed: true
# enable-experimental-rbs-comments: true

# Basic deprecated method
class BasicDeprecated
  
  # @deprecated
  #: -> String
  def old_method
    "legacy"
  end
  
  #: -> String
  def new_method
    "current"
  end
end

BasicDeprecated.new.old_method # error: Method `BasicDeprecated#old_method` is deprecated
BasicDeprecated.new.new_method
