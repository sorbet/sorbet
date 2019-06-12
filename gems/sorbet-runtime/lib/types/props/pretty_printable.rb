# frozen_string_literal: true
# typed: true

module T::Props::PrettyPrintable
  include T::Props::Plugin

  # Return a string representation of this object and all of its props
  def inspect
    T.unsafe(T.cast(self, Object).class).decorator.inspect_instance(self)
  end

  # Override the PP gem with something that's similar, but gives us a hook
  # to do redaction
  def pretty_inspect
    T.unsafe(T.cast(self, Object).class).decorator.inspect_instance(self, multiline: true)
  end

  module DecoratorMethods
    extend T::Sig

    sig {returns(T::Array[Symbol])}
    def valid_props
      super + [:inspect]
    end

    sig do
      params(instance: T::Props::PrettyPrintable, multiline: T::Boolean, indent: String)
      .returns(String)
    end
    def inspect_instance(instance, multiline: false, indent: '  ')
      components =
        inspect_instance_components(
          instance,
          multiline: multiline,
          indent: indent
        )
          .reject(&:empty?)

      # Not using #<> here as that makes pry highlight these objects
      # as if they were all comments, whereas this makes them look
      # like the structured thing they are.
      if multiline
        "#{components[0]}:\n" + T.must(components[1..-1]).join("\n")
      else
        "<#{components.join(' ')}>"
      end
    end

    sig do
      params(instance: T::Props::PrettyPrintable, multiline: T::Boolean, indent: String)
      .returns(T::Array[String])
    end
    private def inspect_instance_components(instance, multiline:, indent:)
      pretty_props = T.unsafe(self).all_props.map do |prop|
        [prop, inspect_prop_value(instance, prop, multiline: multiline, indent: indent)]
      end

      joined_props = join_props_with_pretty_values(
        pretty_props,
        multiline: multiline,
        indent: indent
      )

      [
        T.unsafe(self).decorated_class.to_s,
        joined_props,
      ]
    end

    sig do
      params(instance: T::Props::PrettyPrintable, prop: Symbol, multiline: T::Boolean, indent: String)
      .returns(String)
    end
    private def inspect_prop_value(instance, prop, multiline:, indent:)
      val = T.unsafe(self).get(instance, prop)
      rules = T.unsafe(self).prop_rules(prop)
      if (custom_inspect = rules[:inspect])
        if T::Utils.arity(custom_inspect) == 1
          custom_inspect.call(val)
        else
          custom_inspect.call(val, {multiline: multiline, indent: indent})
        end
      elsif rules[:sensitivity] && !rules[:sensitivity].empty? && !val.nil?
        "<REDACTED #{rules[:sensitivity].join(', ')}>"
      else
        val.inspect
      end
    end

    sig do
      params(pretty_kvs: T::Array[[Symbol, String]], multiline: T::Boolean, indent: String)
      .returns(String)
    end
    private def join_props_with_pretty_values(pretty_kvs, multiline:, indent: '  ')
      pairs = pretty_kvs
        .sort_by {|k, _v| k.to_s}
        .map {|k, v| "#{k}=#{v}"}

      if multiline
        indent + pairs.join("\n#{indent}")
      else
        pairs.join(', ')
      end
    end
  end
end
