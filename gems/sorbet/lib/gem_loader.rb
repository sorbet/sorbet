# frozen_string_literal: true
# typed: ignore

class Sorbet::Private::GemLoader
  NO_GEM = "_unknown"

  # A map defining the code to load a gem. By default any gem mentioned by
  # Gemfile is loaded by its name, here are either overrides or things that
  # Bundler misses.
  GEM_LOADER = {
    'Ascii85' => proc do
      my_require 'ascii85'
    end,
    'aws-sdk-v1' => proc do
      my_require 'aws-sdk-v1'
      AWS.eager_autoload!
      [
        AWS::DynamoDB::Errors::InternalServerError,
        AWS::DynamoDB::Errors::ProvisionedThroughputExceededException,
        AWS::DynamoDB::Errors::ResourceInUseException,
        AWS::DynamoDB::Errors::ResourceNotFoundException,
        AWS::EC2::Errors::RequestLimitExceeded,
        AWS::S3::Errors::AccessDenied,
        AWS::S3::Errors::NoSuchBucket,
        AWS::S3::Errors::NotFound,
      ]
    end,
    'aws-sdk-core' => proc do
      my_require 'aws-sdk'
      [
        Aws::AssumeRoleCredentials,
        Aws::Athena,
        Aws::AutoScaling::Client,
        Aws::AutoScaling::Errors::AlreadyExists,
        Aws::AutoScaling::Errors::Throttling,
        Aws::AutoScaling::Errors::ValidationError,
        Aws::CloudFormation::Client,
        Aws::CloudFormation::Errors::ValidationError,
        Aws::CloudWatchLogs::Client,
        Aws::Credentials,
        Aws::DynamoDB::Client,
        Aws::DynamoDB::Errors::ProvisionedThroughputExceededException,
        Aws::EC2::Errors::RequestLimitExceeded,
        Aws::ElasticLoadBalancing::Client,
        Aws::Errors::ServiceError,
        Aws::IAM::Client,
        Aws::IAM::Errors::NoSuchEntity,
        Aws::IAM::Resource,
        Aws::InstanceProfileCredentials,
        Aws::Lambda::Client,
        Aws::Query::ParamList,
        Aws::S3::Bucket,
        Aws::S3::Client,
        Aws::S3::Encryption::Client,
        Aws::S3::Errors::InvalidRange,
        Aws::S3::Errors::NoSuchKey,
        Aws::S3::Errors::NotFound,
        Aws::S3::Object,
        Aws::S3::Resource,
        Aws::SES::Client,
        Aws::SES::Errors,
        Aws::SES::Errors::AccessDenied,
        Aws::SES::Errors::MessageRejected,
        Aws::SES::Errors::ServiceError,
        Aws::SES::Types,
        Aws::SES::Types::SendEmailResponse,
        Aws::SNS::Client,
        Aws::SNS::MessageVerifier,
        Aws::SQS::QueuePoller,
        Aws::STS::Client,
        Aws::STS::Errors::AccessDenied,
        Seahorse::Client::NetworkingError,
        Seahorse::Client::Response,
      ]
    end,
    'bloomfilter-rb' => proc do
      my_require 'bloomfilter-rb'
    end,
    'hashie' => proc do
      my_require 'hashie'
      [
        Hashie::Mash,
      ]
    end,
    'hike' => proc do
      my_require 'hike'
      [
        Hike::Trail,
      ]
    end,
    'http' => proc do
      my_require 'http'
    end,
    'http-form_data' => proc do
      my_require 'http/form_data'
    end,
    'http_parser.rb' => proc do
      my_require 'http/parser'
    end,
    'minitest' => proc do
      my_require 'minitest'
      my_require 'minitest/mock'
      my_require 'minitest/spec'
      my_require 'minitest/reporters'
    end,
    'rack-test' => proc do
      my_require 'rack/test'
    end,
    'pagerduty-full' => proc do
      my_require 'pagerduty/full'
    end,
    'puma' => proc do
      my_require 'puma'
      my_require 'puma/configuration'
      my_require 'puma/launcher'
      my_require 'puma/server'
    end,
    'term-ansicolor' => proc do
      my_require 'term/ansicolor'
    end,
    'rexml' => proc do
      my_require "rexml/document"
      my_require "rexml/streamlistener"
    end,
    'rubyzip' => proc do
      my_require "zip"
      my_require 'zip/filesystem'
    end,
    'nsq-ruby' => proc do
      my_require 'nsq'
    end,
    'mongo-ruby-driver' => proc do
      my_require 'mongo'
    end,
    'presto-client-ruby' => proc do
      my_require 'presto-client'
    end,
    'bcrypt-ruby' => proc do
      my_require 'bcrypt'
    end,
    'xml-simple' => proc do
      my_require 'xmlsimple'
    end,
    'sinatra-contrib' => proc do
      # We can't my_require all of 'sinatra/contrib' since we put `raise` in them
      my_require 'sinatra/content_for'
      my_require 'sinatra/capture'
      my_require 'sinatra/multi_route'
      my_require 'sinatra/contrib/version'
    end,
    'sqreen' => proc do
      ENV['SQREEN_DISABLE'] = 'true'
      my_require 'sqreen'
    end,
    'thin-attach_socket' => proc do
      my_require 'thin'
      my_require 'thin/attach_socket'
    end,
    'sinatra-partial' => proc do
      my_require 'sinatra/partial'
    end,
    'rack_csrf' => proc do
      my_require 'rack/csrf'
    end,
    'rack-flash3' => proc do
      my_require 'rack-flash'
    end,
    'google-api-client' => proc do
      # There are lots more but this is all we use for now
      my_require 'google/apis/calendar_v3'
      my_require 'google/apis/drive_v3'
      my_require 'google/apis/gmail_v1'
    end,
    'concurrent-ruby' => proc do
      my_require 'concurrent'
    end,
    'cld2' => proc do
      my_require 'cld'
    end,
    'twitter_cldr' => proc do
      my_require 'twitter_cldr'
      [
        TwitterCldr::Shared::Territories,
      ]
    end,
    'stackprof' => proc do
      my_require 'stackprof'
      [
        StackProf::Report,
      ]
    end,
    'sprockets' => proc do
      my_require 'sprockets'
      [
        Sprockets::Cache::FileStore,
        Sprockets::Environment,
        Sprockets::Manifest,
      ]
    end,
    'signet' => proc do
      my_require 'signet'
      my_require 'signet/oauth_2/client'
    end,
    'roo' => proc do
      my_require 'roo'
      [
        Roo::Spreadsheet,
      ]
      version = Bundler.load.specs['roo'][0].stub.version
      if Gem::Requirement.create('<2.0.0').satisfied_by?(version)
        [
          Roo::Excel,
        ]
      end
    end,
    'rack-protection' => proc do
      my_require 'rack-protection'
      [
        Rack::Protection::FrameOptions,
      ]
    end,
    'rack' => proc do
      my_require 'rack'
      [
        Rack::BodyProxy,
        Rack::Cascade,
        Rack::Chunked,
        Rack::CommonLogger,
        Rack::ConditionalGet,
        Rack::Config,
        Rack::ContentLength,
        Rack::ContentType,
        Rack::ETag,
        Rack::Events,
        Rack::File,
        Rack::Files,
        Rack::Deflater,
        Rack::Directory,
        Rack::ForwardRequest,
        Rack::Handler,
        Rack::Head,
        Rack::Lint,
        Rack::Lock,
        Rack::Logger,
        Rack::MediaType,
        Rack::MethodOverride,
        Rack::Mime,
        Rack::NullLogger,
        Rack::Recursive,
        Rack::Reloader,
        Rack::RewindableInput,
        Rack::Runtime,
        Rack::Sendfile,
        Rack::Server,
        Rack::ShowExceptions,
        Rack::ShowStatus,
        Rack::Static,
        Rack::TempfileReaper,
        Rack::URLMap,
        Rack::Utils,
        Rack::Multipart,
        Rack::MockResponse,
        Rack::Response,
        Rack::Auth::Basic,
        Rack::Auth::AbstractRequest,
        Rack::Auth::AbstractHandler,
        Rack::Auth::Digest::MD5,
        Rack::Auth::Digest::Nonce,
        Rack::Auth::Digest::Params,
        Rack::Auth::Digest::Request,
        Rack::Session::Cookie,
        Rack::Session::Pool,
        Rack::Session::Memcache,
        Rack::Handler::CGI,
        Rack::Handler::FastCGI,
        Rack::Handler::WEBrick,
        Rack::Handler::LSWS,
        Rack::Handler::SCGI,
        Rack::Handler::Thin,
        Rack::Multipart::Generator,
        Rack::Multipart::UploadedFile,
      ]
    end,
    'poncho' => proc do
      [
        Poncho::ClientError,
        Poncho::JSONMethod,
        Poncho::Resource,
        Poncho::ValidationError,
      ]
    end,
    'parser' => proc do
      my_require 'parser'
      my_require 'parser/ruby24'
    end,
    'net' => proc do
      my_require 'net/dns'
      my_require 'net/ftp'
      my_require 'net/http'
      my_require 'net/http/digest_auth'
      my_require 'net/http/persistent'
      my_require 'net/imap'
      my_require 'net/protocol'
      my_require 'net/sftp'
      my_require 'net/smtp'
      my_require 'net/ssh'
      my_require 'net/ssh/proxy/http'
      my_require 'rubyntlm'
    end,
    'openssl' => proc do
      my_require 'openssl'
      [
        OpenSSL::X509::Store,
      ]
    end,
    'mail' => proc do
      my_require 'mail'
      [
        Mail::Address,
        Mail::AddressList,
        Mail::Parsers::AddressListsParser,
        Mail::Parsers::ContentDispositionParser,
        Mail::Parsers::ContentLocationParser,
        Mail::Parsers::ContentTransferEncodingParser,
        Mail::Parsers::ContentTypeParser,
        Mail::Parsers::DateTimeParser,
        Mail::Parsers::EnvelopeFromParser,
        Mail::Parsers::MessageIdsParser,
        Mail::Parsers::MimeVersionParser,
        Mail::Parsers::PhraseListsParser,
        Mail::Parsers::ReceivedParser,
      ]
    end,
    'kramdown' => proc do
      my_require 'kramdown'
      [
        Kramdown::Converter::Html,
      ]
    end,
    'ice_cube' => proc do
      my_require 'ice_cube'
      [
        IceCube::DailyRule,
        IceCube::MonthlyRule,
        IceCube::WeeklyRule,
        IceCube::YearlyRule,
        IceCube::Schedule,
      ]
    end,
    'i18n' => proc do
      my_require 'i18n'
      [
        I18n::Backend,
        I18n::Backend::Base,
        I18n::Backend::InterpolationCompiler,
        I18n::Backend::Cache,
        I18n::Backend::CacheFile,
        I18n::Backend::Cascade,
        I18n::Backend::Chain,
        I18n::Backend::Fallbacks,
        I18n::Backend::Flatten,
        I18n::Backend::Gettext,
        I18n::Backend::KeyValue,
        I18n::Backend::Memoize,
        I18n::Backend::Metadata,
        I18n::Backend::Pluralization,
        I18n::Backend::Simple,
        I18n::Backend::Transliterator,
        I18n::Config,
        I18n::Gettext,
        I18n::Gettext::Helpers,
        I18n::Locale,
        I18n::Locale::Fallbacks,
        I18n::Locale::Tag,
        I18n::Locale::Tag::Parents,
        I18n::Locale::Tag::Rfc4646,
        I18n::Locale::Tag::Simple,
        I18n::Tests,
        I18n::Middleware,
      ]
    end,
    'http-cookie' => proc do
      my_require 'http-cookie'
      my_require 'http/cookie_jar'
      my_require 'mechanize'
      [
        HTTP::CookieJar::AbstractSaver,
      ]
    end,
    'faraday' => proc do
      my_require 'faraday'
      [
        Faraday::Request::Multipart,
        Faraday::Request::UrlEncoded,
        Faraday::Response::RaiseError,
      ]
    end,
    'escort' => proc do
      my_require 'escort'
      [
        Escort::App,
        Escort::Setup::Dsl::Options,
        Escort::Trollop::Parser,
      ]
    end,
    'digest' => proc do
      my_require 'digest'
      [
        Digest::SHA2,
      ]
    end,
    'coderay' => proc do
      my_require 'coderay'
      [
        CodeRay::VERSION,
        CodeRay::FileType,
        CodeRay::Tokens,
        CodeRay::TokensProxy,
        CodeRay::TokenKinds,
        CodeRay::PluginHost,
        CodeRay::Plugin,
        CodeRay::Scanners,
        CodeRay::Scanners::Scanner,
        CodeRay::Scanners::Java::BuiltinTypes,
        CodeRay::Scanners::Ruby::Patterns,
        CodeRay::Scanners::Ruby::StringState,
        CodeRay::Encoders,
        CodeRay::Encoders::Encoder,
        CodeRay::Encoders::HTML::Output,
        CodeRay::Encoders::HTML::CSS,
        CodeRay::Encoders::HTML::Numbering,
        CodeRay::Styles,
        CodeRay::Styles::Style,
        CodeRay::Duo,
        CodeRay::WordList,
      ]
    end,
    'byebug' => proc do
      my_require 'byebug'
      my_require 'byebug/core'
    end,
    'racc' => proc do
      my_require 'parser'
    end,
    'rbnacl' => proc do
      my_require 'rbnacl/libsodium'
    end,
    'double-bag-ftps' => proc do
      my_require 'double_bag_ftps'
    end,
    'livechat_client' => proc do
      my_require 'livechat'
    end,
    'nio4r' => proc do
      my_require 'nio'
    end,
    'rgl' => proc do
      my_require 'rgl/adjacency'
      my_require 'rgl/implicit'
      my_require 'rgl/traversal'
      my_require 'rgl/graph_iterator'
      my_require 'rgl/edmonds_karp'
    end,
    'redcarpet' => proc do
      my_require 'redcarpet'
      my_require 'redcarpet/render_strip'
    end,
    'sequel' => proc do
      my_require 'sequel'
      my_require 'sequel/sql'
    end,
    'sequel_pg' => proc do
      # sequel_pg assumes that it was required by the adapter class in sequel
      # (i.e., it's not mean to be required manually). But also, sequel lazily
      # loads the adapter class depending on the scheme of the database being
      # connected to. Since 'srb init' never only requires and never connects,
      # we need to manually load the adapter class ourself, which will then
      # transitively load sequel_pg
      my_require 'sequel/adapters/postgres'
    end,
    'will_paginate' => proc do
      my_require 'will_paginate'
      my_require 'will_paginate/collection'
    end,
    'yard' => proc do
      my_require 'yard'
      [
        YARD::CodeObjects::MethodObject,
        YARD::Docstring,
        YARD::Handlers::Ruby::Base,
        YARD::Registry,
        YARD::Tags::Library,
        YARD::Tags::Tag,
      ]
    end,
    'mocha' => proc do
      my_require 'minitest/spec' # mocha forces you to do this first
      my_require 'mocha/setup'
    end,
    'bundler-audit' => proc do
      my_require 'bundler/audit'
    end,
    'google-protobuf' => proc do
      my_require 'google/protobuf'
    end,
    'multipart-post' => proc do
      my_require 'net/http/post/multipart'
    end,
    'rdl' => proc do
      # needed because this isn't required by default in the Gemfile
      my_require 'rdl_disable'
    end,
    'rss' => proc do
      # needed because this isn't required our Gemfile but some of our gems use it
      my_require 'rss'
    end,
    'ruby-ole' => proc do
      my_require 'ole/storage'
    end,
    'ruby-rc4' => proc do
      my_require 'rc4'
    end,
    'ruby-prof' => proc do
      my_require 'ruby-prof'
      [
        RubyProf::AbstractPrinter,
      ]
    end,
    'stylus-source' => proc do
      my_require 'stylus'
    end,
    'time-utils' => proc do
      my_require 'time/utils'
      my_require 'date/utils'
    end,
    'thor' => proc do
      my_require 'thor'
      [
        Thor::Actions,
        Thor::Group,
      ]
    end,
    'unicode-display_width' => proc do
      my_require 'unicode/display_width'
    end,
    'simplecov-html' => proc do
      my_require 'simplecov'
    end,
    'thwait' => proc do
      my_require 'thwait'
    end,
    'matrix' => proc do
      my_require 'matrix'
    end,
    'zxcvbn-ruby' => proc do
      my_require 'zxcvbn'
    end,
    'elasticsearch-transport' => proc do
      my_require 'elasticsearch'
    end,
    'tzinfo' => proc do
      my_require 'tzinfo'
      my_require 'tzinfo/data'
      TZInfo::Timezone.all.map(&:canonical_zone)
    end,
    'pry-doc' => proc do
      my_require 'pry'
      my_require 'pry-doc'
    end,
    'taxjar-ruby' => proc do
      my_require 'taxjar'
    end,
    'nokogiri' => proc do
      my_require 'nokogiri'
      my_require 'webrobots'
      my_require 'html_truncator'
    end,
    'actionpack' => proc do
      my_require 'action_pack'
      my_require 'action_controller/railtie'
      [
        ActionController::Base,
        ActionController::API,
        ActionController::Base,
        ActionController::Metal,
        ActionController::Middleware,
        ActionController::Renderer,
        ActionController::FormBuilder,
        ActionController::TestCase,
        ActionController::TemplateAssertions,
        ActionController::ConditionalGet,
        ActionController::ContentSecurityPolicy,
        ActionController::Cookies,
        ActionController::DataStreaming,
        ActionController::DefaultHeaders,
        ActionController::EtagWithTemplateDigest,
        ActionController::EtagWithFlash,
        ActionController::FeaturePolicy,
        ActionController::Flash,
        ActionController::ForceSSL,
        ActionController::Head,
        ActionController::Helpers,
        ActionController::HttpAuthentication,
        ActionController::BasicImplicitRender,
        ActionController::ImplicitRender,
        ActionController::Instrumentation,
        ActionController::Logging,
        ActionController::MimeResponds,
        ActionController::ParamsWrapper,
        ActionController::Redirecting,
        ActionController::Renderers,
        ActionController::Rendering,
        ActionController::RequestForgeryProtection,
        ActionController::Rescue,
        ActionController::Streaming,
        ActionController::StrongParameters,
        ActionController::ParameterEncoding,
        ActionController::Testing,
        ActionController::UrlFor,
        ActionDispatch::SystemTestCase
      ]
    end,
    'actionview' => proc do
      my_require 'action_view'
      my_require 'action_view/railtie'
      [
        ActionView::Base,
        ActionView::Context,
        ActionView::Digestor,
        ActionView::Helpers,
        ActionView::LookupContext,
        ActionView::Layouts,
        ActionView::PathSet,
        ActionView::RecordIdentifier,
        ActionView::Rendering,
        ActionView::RoutingUrlFor,
        ActionView::Template,
        ActionView::Template::Error,
        ActionView::Template::RawFile,
        ActionView::Template::Handlers,
        ActionView::Template::HTML,
        ActionView::Template::Inline,
        ActionView::Template::Sources,
        ActionView::Template::Text,
        ActionView::Template::Types,
        ActionView::UnboundTemplate,
        ActionView::ViewPaths,
        ActionView::TestCase,
        ActionView::CacheExpiry,
      ]
    end,
    'actiontext' => proc do
      my_require 'action_text'
      my_require 'action_text/engine'
      [
        ActionText::Attachable,
        ActionText::Attachables::ContentAttachment,
        ActionText::Attachables::MissingAttachable,
        ActionText::Attachables::RemoteImage,
        ActionText::AttachmentGallery,
        ActionText::Attachment,
        ActionText::Attachments::Caching,
        ActionText::Attachments::Minification,
        ActionText::Attachments::TrixConversion,
        ActionText::Attribute,
        ActionText::Content,
        ActionText::Fragment,
        ActionText::HtmlConversion,
        ActionText::PlainTextConversion,
        ActionText::Serialization,
        ActionText::TrixAttachment,
      ]
    end,
    'actionmailbox' => proc do
      my_require 'action_mailbox'
      my_require 'action_mailbox/engine'
      [
        ActionMailbox::Base,
        ActionMailbox::Router,
        ActionMailbox::TestCase,
      ]
    end,
    'actioncable' => proc do
      my_require 'action_cable'
      my_require 'action_cable/engine'
      [
        ActionCable::Server,
        ActionCable::Server::Base,
        ActionCable::Server::Broadcasting,
        ActionCable::Server::Connections,
        ActionCable::Server::Configuration,
        ActionCable::Server::Worker,
        ActionCable::Connection,
        ActionCable::Connection::Authorization,
        ActionCable::Connection::Base,
        ActionCable::Connection::ClientSocket,
        ActionCable::Connection::Identification,
        ActionCable::Connection::InternalChannel,
        ActionCable::Connection::MessageBuffer,
        ActionCable::Connection::Stream,
        ActionCable::Connection::StreamEventLoop,
        ActionCable::Connection::Subscriptions,
        ActionCable::Connection::TaggedLoggerProxy,
        ActionCable::Connection::TestCase,
        ActionCable::Connection::WebSocket,
        ActionCable::Channel,
        ActionCable::Channel::Base,
        ActionCable::Channel::Broadcasting,
        ActionCable::Channel::Callbacks,
        ActionCable::Channel::Naming,
        ActionCable::Channel::PeriodicTimers,
        ActionCable::Channel::Streams,
        ActionCable::Channel::TestCase,
        ActionCable::RemoteConnections,
        ActionCable::SubscriptionAdapter,
        ActionCable::SubscriptionAdapter::Base,
        ActionCable::SubscriptionAdapter::Test,
        ActionCable::SubscriptionAdapter::SubscriberMap,
        ActionCable::SubscriptionAdapter::ChannelPrefix,
        ActionCable::TestHelper,
        ActionCable::TestCase,
      ]
    end,
    'actionmailer' => proc do
      my_require 'action_mailer'
      my_require 'action_mailer/railtie'
      [
        ActionMailer::Base,
        ActionMailer::MessageDelivery,
      ]
    end,
    'activejob' => proc do
      my_require 'active_job'
      my_require 'active_job/railtie'
      [
        ActiveJob::Base,
      ]
    end,
    'activemodel' => proc do
      my_require 'active_model'
      my_require 'active_model/railtie'
    end,
    'activesupport' => proc do
      my_require 'active_support'
      [
        ActiveSupport::Concern,
        ActiveSupport::ActionableError,
        ActiveSupport::CurrentAttributes,
        ActiveSupport::Dependencies,
        ActiveSupport::DescendantsTracker,
        ActiveSupport::ExecutionWrapper,
        ActiveSupport::Executor,
        ActiveSupport::FileUpdateChecker,
        ActiveSupport::EventedFileUpdateChecker,
        ActiveSupport::LogSubscriber,
        ActiveSupport::Notifications,
        ActiveSupport::Reloader,
        ActiveSupport::BacktraceCleaner,
        ActiveSupport::ProxyObject,
        ActiveSupport::Benchmarkable,
        ActiveSupport::Cache,
        ActiveSupport::Cache::FileStore,
        ActiveSupport::Cache::MemoryStore,
        ActiveSupport::Cache::NullStore,
        ActiveSupport::Cache::Strategy::LocalCache,
        ActiveSupport::Cache::Strategy::LocalCache::Middleware,
        ActiveSupport::Callbacks,
        ActiveSupport::Configurable,
        ActiveSupport::Deprecation,
        ActiveSupport::Digest,
        ActiveSupport::Gzip,
        ActiveSupport::Inflector,
        ActiveSupport::JSON,
        ActiveSupport::KeyGenerator,
        ActiveSupport::MessageEncryptor,
        ActiveSupport::MessageVerifier,
        ActiveSupport::Multibyte,
        ActiveSupport::Multibyte::Chars,
        ActiveSupport::Multibyte::Unicode,
        ActiveSupport::NumberHelper,
        ActiveSupport::NumberHelper::NumberConverter,
        ActiveSupport::NumberHelper::RoundingHelper,
        ActiveSupport::NumberHelper::NumberToRoundedConverter,
        ActiveSupport::NumberHelper::NumberToDelimitedConverter,
        ActiveSupport::NumberHelper::NumberToHumanConverter,
        ActiveSupport::NumberHelper::NumberToHumanSizeConverter,
        ActiveSupport::NumberHelper::NumberToPhoneConverter,
        ActiveSupport::NumberHelper::NumberToCurrencyConverter,
        ActiveSupport::NumberHelper::NumberToPercentageConverter,
        ActiveSupport::OptionMerger,
        ActiveSupport::OrderedHash,
        ActiveSupport::OrderedOptions,
        ActiveSupport::StringInquirer,
        ActiveSupport::TaggedLogging,
        ActiveSupport::XmlMini,
        ActiveSupport::ArrayInquirer,
        ActiveSupport::Duration,
        ActiveSupport::Duration::ISO8601Parser,
        ActiveSupport::Duration::ISO8601Serializer,
        ActiveSupport::TimeWithZone,
        ActiveSupport::TimeZone,
        ActiveSupport::Rescuable,
        ActiveSupport::SafeBuffer,
        ActiveSupport::TestCase,
      ]
    end,
    'activerecord' => proc do
      my_require 'active_record'
      my_require 'active_record/railtie'
      [
        ActiveRecord::Schema,
        ActiveRecord::Migration::Current,
      ]
    end,
    'activestorage' => proc do
      my_require 'active_storage'
      my_require 'active_storage/engine'
      [
        ActiveStorage::Attached,
        ActiveStorage::Attached::Changes::CreateOne,
        ActiveStorage::Attached::Changes::CreateMany,
        ActiveStorage::Attached::Changes::CreateOneOfMany,
        ActiveStorage::Attached::Changes::DeleteOne,
        ActiveStorage::Attached::Changes::DeleteMany,
        ActiveStorage::Service,
        ActiveStorage::Service::Configurator,
        ActiveStorage::Previewer,
        ActiveStorage::Analyzer,
        ActiveStorage::Transformers::Transformer,
        ActiveStorage::Transformers::ImageProcessingTransformer,
        ActiveStorage::Transformers::MiniMagickTransformer,
      ]
    end,
    'rdoc' => proc do
      my_require 'rdoc'
      [
        RDoc::Options,
      ]
    end,
    'paul_revere' => proc do
      my_require 'paul_revere'
      [
        Announcement,
      ]
    end,
    'clearance' => proc do
      my_require 'clearance'
      [
        ClearanceMailer,
      ]
    end,
    'webmock' => proc do
      my_require 'webmock'
      WebMock.singleton_class.send(:define_method, :enable!) do
        puts "\nWebMock.enable! is incompatible with Sorbet. Please don't unconditionally do it on requiring this file."
      end
    end,
    'codecov' => proc do
      my_require 'simplecov'
      my_require 'codecov'
    end,
    'sparql' => proc do
      my_require 'sparql'
      [
        SPARQL::Algebra,
        SPARQL::Algebra::Aggregate,
        SPARQL::Algebra::Evaluatable,
        SPARQL::Algebra::Expression,
        SPARQL::Algebra::Operator,
        SPARQL::Algebra::Query,
        SPARQL::Algebra::Update,
        SPARQL::Client,
        SPARQL::Grammar,
        SPARQL::Grammar::Parser,
        SPARQL::Grammar::Terminals,
        SPARQL::Results,
        SPARQL::VERSION,
      ]
    end,
    'sparql-client' => proc do
      my_require 'sparql/client'
      [
        SPARQL::Client::Query,
        SPARQL::Client::Repository,
        SPARQL::Client::Update,
        SPARQL::Client::VERSION,
      ]
    end,
    'selenium-webdriver' => proc do
      my_require 'selenium/webdriver'
      [
        Selenium::WebDriver::Chrome,
        Selenium::WebDriver::Chrome::Bridge,
        Selenium::WebDriver::Chrome::Driver,
        Selenium::WebDriver::Chrome::Profile,
        Selenium::WebDriver::Chrome::Options,
        Selenium::WebDriver::Chrome::Service,
        Selenium::WebDriver::Edge,
        Selenium::WebDriver::Firefox,
        Selenium::WebDriver::Firefox::Extension,
        Selenium::WebDriver::Firefox::ProfilesIni,
        Selenium::WebDriver::Firefox::Profile,
        Selenium::WebDriver::Firefox::Driver,
        Selenium::WebDriver::Firefox::Options,
        Selenium::WebDriver::Firefox::Service,
        Selenium::WebDriver::IE,
        Selenium::WebDriver::IE::Driver,
        Selenium::WebDriver::IE::Options,
        Selenium::WebDriver::IE::Service,
        Selenium::WebDriver::Remote,
        Selenium::WebDriver::Remote::Bridge,
        Selenium::WebDriver::Remote::Driver,
        Selenium::WebDriver::Remote::Response,
        Selenium::WebDriver::Remote::Capabilities,
        Selenium::WebDriver::Remote::Http::Common,
        Selenium::WebDriver::Remote::Http::Default,
        Selenium::WebDriver::Safari,
        Selenium::WebDriver::Safari::Bridge,
        Selenium::WebDriver::Safari::Driver,
        Selenium::WebDriver::Safari::Options,
        Selenium::WebDriver::Safari::Service,
        Selenium::WebDriver::Support,
      ]
    end,
    'friendly_id' => proc do
      my_require 'friendly_id'
      [
        FriendlyId::History,
        FriendlyId::Slug,
        FriendlyId::SimpleI18n,
        FriendlyId::Reserved,
        FriendlyId::Scoped,
        FriendlyId::Slugged,
        FriendlyId::Finders,
        FriendlyId::SequentiallySlugged
      ]
    end,
    'rdf' => proc do
      my_require 'rdf'
      my_require 'rdf/ntriples'
      [
        RDF::Countable,
        RDF::Durable,
        RDF::Enumerable,
        RDF::Indexable,
        RDF::Mutable,
        RDF::Queryable,
        RDF::Readable,
        RDF::TypeCheck,
        RDF::Transactable,
        RDF::Writable,
        RDF::Graph,
        RDF::IRI,
        RDF::Literal,
        RDF::Node,
        RDF::Resource,
        RDF::Statement,
        RDF::URI,
        RDF::Value,
        RDF::Term,
        RDF::List,
        RDF::Format,
        RDF::Reader,
        RDF::ReaderError,
        RDF::Writer,
        RDF::WriterError,
        RDF::NTriples,
        RDF::NQuads,
        RDF::Changeset,
        RDF::Dataset,
        RDF::Repository,
        RDF::Transaction,
        RDF::Query,
        RDF::Query::Pattern,
        RDF::Query::Solution,
        RDF::Query::Solutions,
        RDF::Query::Variable,
        RDF::Query::HashPatternNormalizer,
        RDF::Vocabulary,
        RDF::StrictVocabulary,
        RDF::Util,
        RDF::Util::Aliasing,
        RDF::Util::Cache,
        RDF::Util::File,
        RDF::Util::Logger,
        RDF::Util::UUID,
        RDF::Util::Coercions,
      ]
    end,
    'rspec-core' => proc do
      my_require 'rspec/core'
      [
        RSpec::SharedContext,
        RSpec::Core::ExampleStatusPersister,
        RSpec::Core::Profiler,
        RSpec::Core::DidYouMean,
        RSpec::Core::Formatters::DocumentationFormatter,
        RSpec::Core::Formatters::HtmlFormatter,
        RSpec::Core::Formatters::FallbackMessageFormatter,
        RSpec::Core::Formatters::ProgressFormatter,
        RSpec::Core::Formatters::ProfileFormatter,
        RSpec::Core::Formatters::JsonFormatter,
        RSpec::Core::Formatters::BisectDRbFormatter,
        RSpec::Core::Formatters::ExceptionPresenter,
        RSpec::Core::Formatters::FailureListFormatter,
      ]
    end,
    'rspec-mocks' => proc do
      my_require 'rspec/mocks'
      [
        RSpec::Mocks::AnyInstance,
        RSpec::Mocks::ExpectChain,
        RSpec::Mocks::StubChain,
        RSpec::Mocks::MarshalExtension,
        RSpec::Mocks::Matchers::HaveReceived,
        RSpec::Mocks::Matchers::Receive,
        RSpec::Mocks::Matchers::ReceiveMessageChain,
        RSpec::Mocks::Matchers::ReceiveMessages,
      ]
    end,
    'rspec-support' => proc do
      my_require 'rspec/support'
      [
        RSpec::Support::Differ,
      ]
    end,
    'rspec-expectations' => proc do
      my_require 'rspec/expectations'
      [
        RSpec::Expectations::BlockSnippetExtractor,
        RSpec::Expectations::FailureAggregator,
        RSpec::Matchers::BuiltIn::BeAKindOf,
        RSpec::Matchers::BuiltIn::BeAnInstanceOf,
        RSpec::Matchers::BuiltIn::BeBetween,
        RSpec::Matchers::BuiltIn::Be,
        RSpec::Matchers::BuiltIn::BeComparedTo,
        RSpec::Matchers::BuiltIn::BeFalsey,
        RSpec::Matchers::BuiltIn::BeNil,
        RSpec::Matchers::BuiltIn::BePredicate,
        RSpec::Matchers::BuiltIn::BeTruthy,
        RSpec::Matchers::BuiltIn::BeWithin,
        RSpec::Matchers::BuiltIn::Change,
        RSpec::Matchers::BuiltIn::Compound,
        RSpec::Matchers::BuiltIn::ContainExactly,
        RSpec::Matchers::BuiltIn::Cover,
        RSpec::Matchers::BuiltIn::EndWith,
        RSpec::Matchers::BuiltIn::Eq,
        RSpec::Matchers::BuiltIn::Eql,
        RSpec::Matchers::BuiltIn::Equal,
        RSpec::Matchers::BuiltIn::Exist,
        RSpec::Matchers::BuiltIn::Has,
        RSpec::Matchers::BuiltIn::HaveAttributes,
        RSpec::Matchers::BuiltIn::Include,
        RSpec::Matchers::BuiltIn::All,
        RSpec::Matchers::BuiltIn::Match,
        RSpec::Matchers::BuiltIn::NegativeOperatorMatcher,
        RSpec::Matchers::BuiltIn::OperatorMatcher,
        RSpec::Matchers::BuiltIn::Output,
        RSpec::Matchers::BuiltIn::PositiveOperatorMatcher,
        RSpec::Matchers::BuiltIn::RaiseError,
        RSpec::Matchers::BuiltIn::RespondTo,
        RSpec::Matchers::BuiltIn::Satisfy,
        RSpec::Matchers::BuiltIn::StartWith,
        RSpec::Matchers::BuiltIn::ThrowSymbol,
        RSpec::Matchers::BuiltIn::YieldControl,
        RSpec::Matchers::BuiltIn::YieldSuccessiveArgs,
        RSpec::Matchers::BuiltIn::YieldWithArgs,
        RSpec::Matchers::BuiltIn::YieldWithNoArgs,
      ]
    end,
    'kaminari-core' => proc do
      my_require 'kaminari/core'
    end,
    'kaminari-activerecord' => proc do
      my_require 'kaminari/activerecord'
    end,
    'kaminari-actionview' => proc do
      my_require 'kaminari/actionview'
    end,
    'sxp' => proc do
      my_require 'sxp'
      [
        SXP::Pair,
        SXP::List,
        SXP::Generator,
        SXP::Reader,
        SXP::Reader::Basic,
        SXP::Reader::Extended,
        SXP::Reader::Scheme,
        SXP::Reader::CommonLisp,
        SXP::Reader::SPARQL,
      ]
    end,
    'ebnf' => proc do
      my_require 'ebnf'
      [
        EBNF::Base,
        EBNF::BNF,
        EBNF::LL1,
        EBNF::LL1::Lexer,
        EBNF::LL1::Parser,
        EBNF::LL1::Scanner,
        EBNF::Parser,
        EBNF::Rule,
        EBNF::Writer,
        EBNF::VERSION,
      ]
    end,
    'doorkeeper' => proc do
      my_require 'doorkeeper'
      version = Bundler.load.specs['doorkeeper'][0].stub.version
      if Gem::Requirement.create('>=5.4.0').satisfied_by?(version)
        [
          Doorkeeper::Errors,
          Doorkeeper::OAuth,
          Doorkeeper::Rake,
          Doorkeeper::Request,
          Doorkeeper::Server,
          Doorkeeper::StaleRecordsCleaner,
          Doorkeeper::Validations,
          Doorkeeper::VERSION,
          Doorkeeper::AccessGrantMixin,
          Doorkeeper::AccessTokenMixin,
          Doorkeeper::ApplicationMixin,
          Doorkeeper::Helpers::Controller,
          Doorkeeper::Request::Strategy,
          Doorkeeper::Request::AuthorizationCode,
          Doorkeeper::Request::ClientCredentials,
          Doorkeeper::Request::Code,
          Doorkeeper::Request::Password,
          Doorkeeper::Request::RefreshToken,
          Doorkeeper::Request::Token,
          Doorkeeper::OAuth::BaseRequest,
          Doorkeeper::OAuth::AuthorizationCodeRequest,
          Doorkeeper::OAuth::BaseResponse,
          Doorkeeper::OAuth::CodeResponse,
          Doorkeeper::OAuth::Client,
          Doorkeeper::OAuth::ClientCredentialsRequest,
          Doorkeeper::OAuth::CodeRequest,
          Doorkeeper::OAuth::ErrorResponse,
          Doorkeeper::OAuth::Error,
          Doorkeeper::OAuth::InvalidTokenResponse,
          Doorkeeper::OAuth::InvalidRequestResponse,
          Doorkeeper::OAuth::ForbiddenTokenResponse,
          Doorkeeper::OAuth::NonStandard,
          Doorkeeper::OAuth::PasswordAccessTokenRequest,
          Doorkeeper::OAuth::PreAuthorization,
          Doorkeeper::OAuth::RefreshTokenRequest,
          Doorkeeper::OAuth::Scopes,
          Doorkeeper::OAuth::Token,
          Doorkeeper::OAuth::TokenIntrospection,
          Doorkeeper::OAuth::TokenRequest,
          Doorkeeper::OAuth::TokenResponse,
          Doorkeeper::OAuth::Authorization::Code,
          Doorkeeper::OAuth::Authorization::Context,
          Doorkeeper::OAuth::Authorization::Token,
          Doorkeeper::OAuth::Authorization::URIBuilder,
          Doorkeeper::OAuth::Client::Credentials,
          Doorkeeper::OAuth::ClientCredentials::Validator,
          Doorkeeper::OAuth::ClientCredentials::Creator,
          Doorkeeper::OAuth::ClientCredentials::Issuer,
          Doorkeeper::OAuth::Helpers::ScopeChecker,
          Doorkeeper::OAuth::Helpers::URIChecker,
          Doorkeeper::OAuth::Helpers::UniqueToken,
          Doorkeeper::OAuth::Hooks::Context,
          Doorkeeper::Models::Accessible,
          Doorkeeper::Models::Expirable,
          Doorkeeper::Models::Orderable,
          Doorkeeper::Models::Scopes,
          Doorkeeper::Models::Reusable,
          Doorkeeper::Models::ResourceOwnerable,
          Doorkeeper::Models::Revocable,
          Doorkeeper::Models::SecretStorable,
          Doorkeeper::Orm::ActiveRecord,
          Doorkeeper::Rails::Helpers,
          Doorkeeper::Rails::Routes,
          Doorkeeper::SecretStoring::Base,
          Doorkeeper::SecretStoring::Plain,
          Doorkeeper::SecretStoring::Sha256Hash,
          Doorkeeper::SecretStoring::BCrypt,
        ]
      end
    end,
  }

  # This is so that the autoloader doesn't treat these as mandatory requires
  # before loading this file
  def self.my_require(gem)
    require gem # rubocop:disable PrisonGuard/NoDynamicRequire
  end

  def self.require_gem(gem)
    if gem == NO_GEM
      require_all_gems
      return
    end
    loader = GEM_LOADER[gem]
    if loader
      begin
        loader.call
      rescue NameError => e
        puts "NameError: #{e}"
      end
    else
      begin
        require gem # rubocop:disable PrisonGuard/NoDynamicRequire
      rescue NameError => e
        puts "NameError: #{e}"
      end
    end
  end

  def self.require_all_gems
    require 'bundler/setup'
    require 'bundler/lockfile_parser'

    specs = []
    # Do not load gems in Gemfile where require is false
    gemfile_dependencies = Bundler.load.dependencies.
      reject { |dep| dep.autorequire && dep.autorequire.empty? }
    required_dependency_names = gemfile_dependencies.map(&:name)

    lockfile_parser = Bundler::LockfileParser.new(File.read(Bundler.default_lockfile))
    lockfile_parser.specs.each do |spec|
      # Only include the spec for a gem and it's dependencies if it's autorequired.
      if required_dependency_names.include?(spec.name)
        specs << spec
        specs << spec.dependencies
      end
    end
    specs.flatten!
    specs.uniq! { |spec| spec.name }
    specs = specs.to_set

    specs.sort_by(&:name).each do |gemspec|
      begin
        require_gem(gemspec.name)
      rescue LoadError => e
        puts "LoadError: #{e}"
      rescue NameError => e
        puts "NameError: #{e}"
      end
    end
    begin
      Bundler.require
    rescue NameError => e
      puts "NameError: #{e}"
    end
  end
end
# rubocop:enable PrisonGuard/AutogenLoaderPreamble
