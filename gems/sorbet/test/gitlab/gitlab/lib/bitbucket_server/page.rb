# frozen_string_literal: true

module BitbucketServer
  class Page
    attr_reader :attrs, :items

    def initialize(raw, type)
      @attrs = parse_attrs(raw)
      @items = parse_values(raw, representation_class(type))
    end

    def next?
      !attrs.fetch(:isLastPage, true)
    end

    def next
      attrs.fetch(:nextPageStart)
    end

    private

    def parse_attrs(raw)
      raw.slice('size', 'nextPageStart', 'isLastPage').symbolize_keys
    end

    def parse_values(raw, bitbucket_rep_class)
      return [] unless raw['values'] && raw['values'].is_a?(Array)

      bitbucket_rep_class.decorate(raw['values'])
    end

    def representation_class(type)
      BitbucketServer::Representation.const_get(type.to_s.camelize)
    end
  end
end
