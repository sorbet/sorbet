# frozen_string_literal: true
# typed: true

class T::MustTypeError < TypeError
  # This approximates what the error_highlight gem does.
  if RubyVM::AbstractSyntaxTree.method(:of).parameters.include?([:key, :keep_script_lines])
    def generate_snippet
      # The line at index 1 in the backtrace is the call to T.must
      backtrace_location = self.backtrace_locations&.[](1)
      return "" unless backtrace_location

      # Newer versions of error_highlight allow passing a backtrace_location
      # directly to ErrorHighlight.spot doign it ourself is not much work at the
      # benefit of working no matter what error_highlight version a project uses.
      node = RubyVM::AbstractSyntaxTree.of(backtrace_location, keep_script_lines: true)
      return "" unless node

      spot = ErrorHighlight.spot(node, point_type: :args)
      return "" unless spot

      return ErrorHighlight.formatter.message_for(spot)
    end
  else
    # ErrorHighlight requires `keep_script_lines: true` to get the text source of the node
    def generate_snippet
      ""
    end
  end

  private :generate_snippet

  if defined?(ErrorHighlight::CoreExt) && self < ErrorHighlight::CoreExt
    # If we're running with a version of error_highlight that has already
    # monkeypatched TypeError, we don't need to define our own detailed_message
    # override.
  elsif Exception.method_defined?(:detailed_message)
    # For Ruby 3.2 but with an older version of error_highlight than is bundled with Ruby 3.2
    def detailed_message(highlight: false, error_highlight: true, **)
      return super unless error_highlight
      snippet = generate_snippet
      if highlight && !snippet.empty?
        snippet = "\e[1m" + snippet + "\e[m"
      end
      super + snippet
    end
  else
    # For Ruby 3.1

    # This is a marker to let `DidYouMean::Correctable#original_message` skip
    # the following method definition of `to_s`.
    # See https://github.com/ruby/did_you_mean/pull/152
    SKIP_TO_S_FOR_SUPER_LOOKUP = true
    private_constant :SKIP_TO_S_FOR_SUPER_LOOKUP

    def to_s
      msg = super
      snippet = generate_snippet
      if snippet != "" && !msg.include?(snippet)
        msg + snippet
      else
        msg
      end
    end
  end
end
