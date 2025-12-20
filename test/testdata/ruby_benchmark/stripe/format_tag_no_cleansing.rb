# frozen_string_literal: true
# typed: strict
# compiled: true

# ::Boolean copied from extn/boolean.rb

module ::Boolean
  extend T::Helpers
  sealed!
end

class ::FalseClass
  include ::Boolean
end

class ::TrueClass
  include ::Boolean
end

# End code copied from extn/boolean.rb

# Begin code adapted from lib/server/lib/stripe-server/metrics.rb

module M
  extend T::Sig

  TagKey = T.type_alias {T.any(String, Symbol)}
  TagValue = T.type_alias {T.any(Boolean, Numeric, String, Symbol, NilClass)}

  sig do
    params(
      key: TagKey,
      value: TagValue,
      allow_periods: Boolean,
    )
    .returns(String)
    .checked(:never)
  end
  private_class_method def self.format_tag(key, value, allow_periods)
    key = key.to_s unless key.is_a?(String)
    key = cleanse_tag(key, allow_periods)

    case value
    when nil
      return "#{key}:nil"
    when true
      return "#{key}:true"
    when false
      return "#{key}:false"
    when String
      value = cleanse_tag(value, allow_periods)
    else
      value = cleanse_tag(value.to_s, allow_periods)
    end

    if value.empty?
      "#{key}:nil"
    else
      "#{key}:#{value}"
    end
  end

  # Per Datadog's Docs:
  # Tags must start with a letter, and after that may contain alphanumerics, underscores, minuses, colons, periods and slashes.
  # Tags can be up to 200 characters long and support unicode. Tags will be converted to lowercase as well.
  # We're being a bit more strict and only allowing alphanumerics and dashes.
  # If allow_periods is true, the . character will be permitted as well except as the leading character
  sig do
    params(
      value: String,
      allow_periods: Boolean,
    )
    .returns(String)
    .checked(:never)
  end
  def self.cleanse_tag(value, allow_periods)
    value = value.downcase if value.match?(/[A-Z]/) # We only care about ASCII uppercase since anything else will be stripped below

    # NB: In Ruby 2.6, the "in-place" versions of these methods (e.g. `tr!`)
    # still allocate, and don't seem to be smart enough to skip doing so
    # even if there's no change to be made. So we don't bother with them
    # even in cases where we know it'd be safe to mutate the string.
    if allow_periods
      value = value.tr("^a-z0-9\-_.", "_") if value.match?(/[^a-z0-9\-_.]/)
      value = value.sub('.', '_') if value.start_with?('.')
    else
      value = value.tr("^a-z0-9\-_", "_") if value.match?(/[^a-z0-9\-_]/)
    end

    value
  end
end

# End code adapted from lib/server/lib/stripe-server/metrics.rb

# The k/v here are chosen to make "cleanse_tag" do as little work as possible. See also format_tag_cleansing.rb.
module M
  sig{void}
  def self.do_test
    k = 'hello'
    v = 42

    i = 0

    while i < 1_000_000
      format_tag(k, v, false)
    
      i += 1
    end

    puts i
  end
end

M.do_test
