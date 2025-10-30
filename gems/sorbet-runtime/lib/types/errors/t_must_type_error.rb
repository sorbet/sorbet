# frozen_string_literal: true
# typed: true

class T::MustTypeError < TypeError
  # This approximates what the error_highlight gem does (ErrorHighlight::CoreExt).
  # We have chosen not to use that so that we can customize like the
  # `point_type` in the call to ErrorHighlight.spot and generally just have
  # more control over the logic as it might change across Ruby and gem versions.
  #
  # You are encouraged to understand this file by way of cross-referencing with
  # https://github.com/ruby/error_highlight/blob/master/lib/error_highlight/core_ext.rb
  # and/or
  # https://github.com/ruby/error_highlight/blob/80ede6b8ca3219310f30cff42cd17177a7e47f25/lib/error_highlight/core_ext.rb
  if RubyVM::AbstractSyntaxTree.method(:of).parameters.include?([:key, :keep_script_lines])
    def generate_snippet
      # The line at index 1 in the backtrace is the call to T.must
      backtrace_location = self.backtrace_locations&.[](1)
      return "" unless backtrace_location

      # Newer versions of error_highlight allow passing a backtrace_location
      # directly to ErrorHighlight.spot doing it ourself is not much work at the
      # benefit of working no matter what error_highlight version a project uses.
      begin
        node = RubyVM::AbstractSyntaxTree.of(backtrace_location, keep_script_lines: true)
      rescue ArgumentError
        # AbstractSyntaxTree.of raises if the backtrace_location is an eval line.
        #
        #     <internal:ast>:67:in `of': cannot get AST for method defined in eval (ArgumentError)
        #
        # It doesn't appear that there's a method exposed to Ruby to determine
        # whether the backtrace line is an eval line (there is a C function),
        # nor does there appear to be a way to request that `.of` return an
        # error value (or nil) instead of raising an exception.
        return ""
      end
      return "" unless node

      spot = ErrorHighlight.spot(node, point_type: :args)
      return "" unless spot

      ErrorHighlight.formatter.message_for(spot)
    end
  else
    # ErrorHighlight requires `keep_script_lines: true` to get the text source of the node
    def generate_snippet
      ""
    end
  end

  private :generate_snippet

  # ----------------------------------------------------------------------------------------------
  # After a few Ruby versions go by, we can probably replace everything below with just
  #
  # include ErrorHighlight::CoreExt
  #
  # Vendoring it below so that sorbet-runtime users get good error messages
  # regardless of how new their error_highlight gem version is.

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
        snippet = "\e[1m#{snippet}\e[m"
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
