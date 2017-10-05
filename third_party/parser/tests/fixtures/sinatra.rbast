(begin
  (send nil :require
    (str "rack"))
  (send nil :require
    (str "tilt"))
  (send nil :require
    (str "rack/protection"))
  (send nil :require
    (str "thread"))
  (send nil :require
    (str "time"))
  (send nil :require
    (str "uri"))
  (send nil :require
    (str "sinatra/showexceptions"))
  (send nil :require
    (str "sinatra/version"))
  (module
    (const nil :Sinatra)
    (begin
      (class
        (const nil :Request)
        (const
          (const nil :Rack) :Request)
        (begin
          (def :accept
            (args)
            (or-asgn
              (send
                (ivar :@env) :[]
                (str "sinatra.accept"))
              (kwbegin
                (lvasgn :entries
                  (send
                    (send
                      (send
                        (ivar :@env) :[]
                        (str "HTTP_ACCEPT")) :to_s) :split
                    (str ",")))
                (send
                  (send
                    (block
                      (send
                        (lvar :entries) :map)
                      (args
                        (procarg0 :e))
                      (send nil :accept_entry
                        (lvar :e))) :sort_by
                    (block-pass
                      (sym :last))) :map
                  (block-pass
                    (sym :first))))))
          (def :preferred_type
            (args
              (restarg :types))
            (begin
              (if
                (send
                  (lvar :types) :empty?)
                (return
                  (send
                    (send nil :accept) :first)) nil)
              (send
                (lvar :types) :flatten!)
              (block
                (send
                  (send nil :accept) :detect)
                (args
                  (procarg0 :pattern))
                (begin
                  (lvasgn :type
                    (block
                      (send
                        (lvar :types) :detect)
                      (args
                        (procarg0 :t))
                      (send
                        (const nil :File) :fnmatch
                        (lvar :pattern)
                        (lvar :t))))
                  (if
                    (lvar :type)
                    (return
                      (lvar :type)) nil)))))
          (alias
            (sym :accept?)
            (sym :preferred_type))
          (alias
            (sym :secure?)
            (sym :ssl?))
          (def :forwarded?
            (args)
            (send
              (ivar :@env) :include?
              (str "HTTP_X_FORWARDED_HOST")))
          (def :safe?
            (args)
            (or
              (or
                (or
                  (send nil :get?)
                  (send nil :head?))
                (send nil :options?))
              (send nil :trace?)))
          (def :idempotent?
            (args)
            (or
              (or
                (send nil :safe?)
                (send nil :put?))
              (send nil :delete?)))
          (send nil :private)
          (def :accept_entry
            (args
              (arg :entry))
            (begin
              (masgn
                (mlhs
                  (lvasgn :type)
                  (splat
                    (lvasgn :options)))
                (send
                  (send
                    (lvar :entry) :delete
                    (str " ")) :split
                  (str ";")))
              (lvasgn :quality
                (int 0))
              (block
                (send
                  (lvar :options) :delete_if)
                (args
                  (procarg0 :e))
                (if
                  (send
                    (lvar :e) :start_with?
                    (str "q="))
                  (lvasgn :quality
                    (send
                      (int 1) :-
                      (send
                        (send
                          (lvar :e) :[]
                          (irange
                            (int 2)
                            (int -1))) :to_f))) nil))
              (array
                (lvar :type)
                (array
                  (lvar :quality)
                  (send
                    (lvar :type) :count
                    (str "*"))
                  (send
                    (int 1) :-
                    (send
                      (lvar :options) :size))))))))
      (class
        (const nil :Response)
        (const
          (const nil :Rack) :Response)
        (begin
          (def :body=
            (args
              (arg :value))
            (begin
              (while
                (send
                  (const
                    (const nil :Rack) :Response) :===
                  (lvar :value))
                (lvasgn :value
                  (send
                    (lvar :value) :body)))
              (ivasgn :@body
                (if
                  (send
                    (const nil :String) :===
                    (lvar :value))
                  (array
                    (send
                      (lvar :value) :to_str))
                  (lvar :value)))))
          (def :each
            (args)
            (if
              (send nil :block_given?)
              (zsuper)
              (send nil :enum_for
                (sym :each))))
          (def :finish
            (args)
            (begin
              (if
                (send
                  (send
                    (send
                      (send nil :status) :to_i) :/
                    (int 100)) :==
                  (int 1))
                (begin
                  (send
                    (send nil :headers) :delete
                    (str "Content-Length"))
                  (send
                    (send nil :headers) :delete
                    (str "Content-Type")))
                (if
                  (and
                    (send
                      (const nil :Array) :===
                      (send nil :body))
                    (send
                      (send
                        (array
                          (int 204)
                          (int 304)) :include?
                        (send
                          (send nil :status) :to_i)) :!))
                  (or-asgn
                    (send
                      (send nil :headers) :[]
                      (str "Content-Length"))
                    (send
                      (block
                        (send
                          (send nil :body) :inject
                          (int 0))
                        (args
                          (arg :l)
                          (arg :p))
                        (send
                          (lvar :l) :+
                          (send
                            (const
                              (const nil :Rack) :Utils) :bytesize
                            (lvar :p)))) :to_s)) nil))
              (masgn
                (mlhs
                  (lvasgn :status)
                  (lvasgn :headers)
                  (lvasgn :result))
                (zsuper))
              (if
                (send
                  (lvar :result) :==
                  (self))
                (lvasgn :result
                  (send nil :body)) nil)
              (array
                (lvar :status)
                (lvar :headers)
                (lvar :result))))))
      (class
        (const nil :ExtendedRack)
        (send
          (const nil :Struct) :new
          (sym :app))
        (begin
          (def :call
            (args
              (arg :env))
            (begin
              (masgn
                (mlhs
                  (lvasgn :result)
                  (lvasgn :callback))
                (array
                  (send
                    (send nil :app) :call
                    (lvar :env))
                  (send
                    (lvar :env) :[]
                    (str "async.callback"))))
              (if
                (and
                  (lvar :callback)
                  (send nil :async?
                    (splat
                      (lvar :result)))) nil
                (return
                  (lvar :result)))
              (block
                (send nil :after_response)
                (args)
                (send
                  (lvar :callback) :call
                  (lvar :result)))
              (send nil :setup_close
                (lvar :env)
                (splat
                  (lvar :result)))
              (send nil :throw
                (sym :async))))
          (send nil :private)
          (def :setup_close
            (args
              (arg :env)
              (arg :status)
              (arg :header)
              (arg :body))
            (begin
              (if
                (and
                  (send
                    (lvar :body) :respond_to?
                    (sym :close))
                  (send
                    (lvar :env) :include?
                    (str "async.close"))) nil
                (return))
              (block
                (send
                  (send
                    (lvar :env) :[]
                    (str "async.close")) :callback)
                (args)
                (send
                  (lvar :body) :close))
              (block
                (send
                  (send
                    (lvar :env) :[]
                    (str "async.close")) :errback)
                (args)
                (send
                  (lvar :body) :close))))
          (def :after_response
            (args
              (blockarg :block))
            (begin
              (if
                (defined?
                  (const nil :EventMachine)) nil
                (send nil :raise
                  (const nil :NotImplementedError)
                  (str "only supports EventMachine at the moment")))
              (send
                (const nil :EventMachine) :next_tick
                (block-pass
                  (lvar :block)))))
          (def :async?
            (args
              (arg :status)
              (arg :headers)
              (arg :body))
            (begin
              (if
                (send
                  (lvar :status) :==
                  (int -1))
                (return
                  (true)) nil)
              (and
                (send
                  (lvar :body) :respond_to?
                  (sym :callback))
                (send
                  (lvar :body) :respond_to?
                  (sym :errback)))))))
      (class
        (const nil :CommonLogger)
        (const
          (const nil :Rack) :CommonLogger)
        (begin
          (def :call
            (args
              (arg :env))
            (if
              (send
                (lvar :env) :[]
                (str "sinatra.commonlogger"))
              (send
                (ivar :@app) :call
                (lvar :env))
              (zsuper)))
          (block
            (send
              (send nil :superclass) :class_eval)
            (args)
            (begin
              (if
                (send nil :method_defined?
                  (sym :call_without_check)) nil
                (alias
                  (sym :call_without_check)
                  (sym :call)))
              (def :call
                (args
                  (arg :env))
                (begin
                  (send
                    (lvar :env) :[]=
                    (str "sinatra.commonlogger")
                    (true))
                  (send nil :call_without_check
                    (lvar :env))))))))
      (class
        (const nil :NotFound)
        (const nil :NameError)
        (def :http_status
          (args)
          (int 404)))
      (module
        (const nil :Helpers)
        (begin
          (def :status
            (args
              (optarg :value
                (nil)))
            (begin
              (if
                (lvar :value)
                (send
                  (send nil :response) :status=
                  (lvar :value)) nil)
              (send
                (send nil :response) :status)))
          (def :body
            (args
              (optarg :value
                (nil))
              (blockarg :block))
            (if
              (send nil :block_given?)
              (begin
                (defs
                  (lvar :block) :each
                  (args)
                  (yield
                    (send nil :call)))
                (send
                  (send nil :response) :body=
                  (lvar :block)))
              (if
                (lvar :value)
                (send
                  (send nil :response) :body=
                  (lvar :value))
                (send
                  (send nil :response) :body))))
          (def :redirect
            (args
              (arg :uri)
              (restarg :args))
            (begin
              (if
                (and
                  (send
                    (send
                      (send nil :env) :[]
                      (str "HTTP_VERSION")) :==
                    (str "HTTP/1.1"))
                  (send
                    (send
                      (send nil :env) :[]
                      (str "REQUEST_METHOD")) :!=
                    (str "GET")))
                (send nil :status
                  (int 303))
                (send nil :status
                  (int 302)))
              (send
                (send nil :response) :[]=
                (str "Location")
                (send nil :uri
                  (lvar :uri)
                  (send
                    (send nil :settings) :absolute_redirects?)
                  (send
                    (send nil :settings) :prefixed_redirects?)))
              (send nil :halt
                (splat
                  (lvar :args)))))
          (def :uri
            (args
              (optarg :addr
                (nil))
              (optarg :absolute
                (true))
              (optarg :add_script_name
                (true)))
            (begin
              (if
                (send
                  (lvar :addr) :=~
                  (regexp
                    (str "\\A[A-z][A-z0-9\\+\\.\\-]*:")
                    (regopt)))
                (return
                  (lvar :addr)) nil)
              (lvasgn :uri
                (array
                  (lvasgn :host
                    (str ""))))
              (if
                (lvar :absolute)
                (begin
                  (send
                    (lvar :host) :<<
                    (dstr
                      (str "http")
                      (begin
                        (if
                          (send
                            (send nil :request) :secure?)
                          (str "s") nil))
                      (str "://")))
                  (if
                    (or
                      (send
                        (send nil :request) :forwarded?)
                      (send
                        (send
                          (send nil :request) :port) :!=
                        (begin
                          (if
                            (send
                              (send nil :request) :secure?)
                            (int 443)
                            (int 80)))))
                    (send
                      (lvar :host) :<<
                      (send
                        (send nil :request) :host_with_port))
                    (send
                      (lvar :host) :<<
                      (send
                        (send nil :request) :host)))) nil)
              (if
                (lvar :add_script_name)
                (send
                  (lvar :uri) :<<
                  (send
                    (send
                      (send nil :request) :script_name) :to_s)) nil)
              (send
                (lvar :uri) :<<
                (send
                  (begin
                    (if
                      (lvar :addr)
                      (lvar :addr)
                      (send
                        (send nil :request) :path_info))) :to_s))
              (send
                (const nil :File) :join
                (lvar :uri))))
          (alias
            (sym :url)
            (sym :uri))
          (alias
            (sym :to)
            (sym :uri))
          (def :error
            (args
              (arg :code)
              (optarg :body
                (nil)))
            (begin
              (if
                (send
                  (lvar :code) :respond_to?
                  (sym :to_str))
                (masgn
                  (mlhs
                    (lvasgn :code)
                    (lvasgn :body))
                  (array
                    (int 500)
                    (send
                      (lvar :code) :to_str))) nil)
              (if
                (send
                  (lvar :body) :nil?) nil
                (send
                  (send nil :response) :body=
                  (lvar :body)))
              (send nil :halt
                (lvar :code))))
          (def :not_found
            (args
              (optarg :body
                (nil)))
            (send nil :error
              (int 404)
              (lvar :body)))
          (def :headers
            (args
              (optarg :hash
                (nil)))
            (begin
              (if
                (lvar :hash)
                (send
                  (send
                    (send nil :response) :headers) :merge!
                  (lvar :hash)) nil)
              (send
                (send nil :response) :headers)))
          (def :session
            (args)
            (send
              (send nil :request) :session))
          (def :logger
            (args)
            (send
              (send nil :request) :logger))
          (def :mime_type
            (args
              (arg :type))
            (send
              (const nil :Base) :mime_type
              (lvar :type)))
          (def :content_type
            (args
              (optarg :type
                (nil))
              (optarg :params
                (hash)))
            (begin
              (if
                (lvar :type) nil
                (return
                  (send
                    (send nil :response) :[]
                    (str "Content-Type"))))
              (lvasgn :default
                (send
                  (lvar :params) :delete
                  (sym :default)))
              (lvasgn :mime_type
                (or
                  (send nil :mime_type
                    (lvar :type))
                  (lvar :default)))
              (if
                (send
                  (lvar :mime_type) :nil?)
                (send nil :fail
                  (send
                    (str "Unknown media type: %p") :%
                    (lvar :type))) nil)
              (lvasgn :mime_type
                (send
                  (lvar :mime_type) :dup))
              (if
                (or
                  (send
                    (lvar :params) :include?
                    (sym :charset))
                  (block
                    (send
                      (send
                        (send nil :settings) :add_charset) :all?)
                    (args
                      (procarg0 :p))
                    (send
                      (send
                        (lvar :p) :===
                        (lvar :mime_type)) :!))) nil
                (send
                  (lvar :params) :[]=
                  (sym :charset)
                  (or
                    (send
                      (lvar :params) :delete
                      (str "charset"))
                    (send
                      (send nil :settings) :default_encoding))))
              (if
                (send
                  (lvar :mime_type) :include?
                  (str "charset"))
                (send
                  (lvar :params) :delete
                  (sym :charset)) nil)
              (if
                (send
                  (lvar :params) :empty?) nil
                (begin
                  (send
                    (lvar :mime_type) :<<
                    (begin
                      (if
                        (send
                          (lvar :mime_type) :include?
                          (str ";"))
                        (str ", ")
                        (str ";"))))
                  (send
                    (lvar :mime_type) :<<
                    (send
                      (block
                        (send
                          (lvar :params) :map)
                        (args
                          (procarg0 :kv))
                        (send
                          (lvar :kv) :join
                          (str "="))) :join
                      (str ", ")))))
              (send
                (send nil :response) :[]=
                (str "Content-Type")
                (lvar :mime_type))))
          (def :attachment
            (args
              (optarg :filename
                (nil)))
            (begin
              (send
                (send nil :response) :[]=
                (str "Content-Disposition")
                (str "attachment"))
              (if
                (lvar :filename)
                (begin
                  (lvasgn :params
                    (send
                      (str "; filename=\"%s\"") :%
                      (send
                        (const nil :File) :basename
                        (lvar :filename))))
                  (send
                    (send
                      (send nil :response) :[]
                      (str "Content-Disposition")) :<<
                    (lvar :params))
                  (lvasgn :ext
                    (send
                      (const nil :File) :extname
                      (lvar :filename)))
                  (if
                    (or
                      (send
                        (send nil :response) :[]
                        (str "Content-Type"))
                      (send
                        (lvar :ext) :empty?)) nil
                    (send nil :content_type
                      (lvar :ext)))) nil)))
          (def :send_file
            (args
              (arg :path)
              (optarg :opts
                (hash)))
            (rescue
              (begin
                (if
                  (or
                    (send
                      (lvar :opts) :[]
                      (sym :type))
                    (send
                      (send
                        (send nil :response) :[]
                        (str "Content-Type")) :!))
                  (send nil :content_type
                    (or
                      (send
                        (lvar :opts) :[]
                        (sym :type))
                      (send
                        (const nil :File) :extname
                        (lvar :path)))
                    (hash
                      (pair
                        (sym :default)
                        (str "application/octet-stream")))) nil)
                (if
                  (or
                    (send
                      (send
                        (lvar :opts) :[]
                        (sym :disposition)) :==
                      (str "attachment"))
                    (send
                      (lvar :opts) :[]
                      (sym :filename)))
                  (send nil :attachment
                    (or
                      (send
                        (lvar :opts) :[]
                        (sym :filename))
                      (lvar :path)))
                  (if
                    (send
                      (send
                        (lvar :opts) :[]
                        (sym :disposition)) :==
                      (str "inline"))
                    (send
                      (send nil :response) :[]=
                      (str "Content-Disposition")
                      (str "inline")) nil))
                (if
                  (send
                    (lvar :opts) :[]
                    (sym :last_modified))
                  (send nil :last_modified
                    (send
                      (lvar :opts) :[]
                      (sym :last_modified))) nil)
                (lvasgn :file
                  (send
                    (const
                      (const nil :Rack) :File) :new
                    (nil)))
                (send
                  (lvar :file) :path=
                  (lvar :path))
                (lvasgn :result
                  (send
                    (lvar :file) :serving
                    (send nil :env)))
                (block
                  (send
                    (send
                      (lvar :result) :[]
                      (int 1)) :each)
                  (args
                    (arg :k)
                    (arg :v))
                  (or-asgn
                    (send
                      (send nil :headers) :[]
                      (lvar :k))
                    (lvar :v)))
                (send
                  (send nil :headers) :[]=
                  (str "Content-Length")
                  (send
                    (send
                      (lvar :result) :[]
                      (int 1)) :[]
                    (str "Content-Length")))
                (send nil :halt
                  (or
                    (send
                      (lvar :opts) :[]
                      (sym :status))
                    (send
                      (lvar :result) :[]
                      (int 0)))
                  (send
                    (lvar :result) :[]
                    (int 2))))
              (resbody
                (array
                  (const
                    (const nil :Errno) :ENOENT)) nil
                (send nil :not_found)) nil))
          (class
            (const nil :Stream) nil
            (begin
              (defs
                (self) :schedule
                (args
                  (restarg))
                (yield))
              (defs
                (self) :defer
                (args
                  (restarg))
                (yield))
              (def :initialize
                (args
                  (optarg :scheduler
                    (send
                      (self) :class))
                  (optarg :keep_open
                    (false))
                  (blockarg :back))
                (begin
                  (masgn
                    (mlhs
                      (ivasgn :@back)
                      (ivasgn :@scheduler)
                      (ivasgn :@keep_open))
                    (array
                      (send
                        (lvar :back) :to_proc)
                      (lvar :scheduler)
                      (lvar :keep_open)))
                  (masgn
                    (mlhs
                      (ivasgn :@callbacks)
                      (ivasgn :@closed))
                    (array
                      (array)
                      (false)))))
              (def :close
                (args)
                (begin
                  (if
                    (ivar :@closed)
                    (return) nil)
                  (ivasgn :@closed
                    (true))
                  (block
                    (send
                      (ivar :@scheduler) :schedule)
                    (args)
                    (block
                      (send
                        (ivar :@callbacks) :each)
                      (args
                        (procarg0 :c))
                      (send
                        (lvar :c) :call)))))
              (def :each
                (args
                  (blockarg :front))
                (begin
                  (ivasgn :@front
                    (lvar :front))
                  (block
                    (send
                      (ivar :@scheduler) :defer)
                    (args)
                    (begin
                      (kwbegin
                        (rescue
                          (send
                            (ivar :@back) :call
                            (self))
                          (resbody
                            (array
                              (const nil :Exception))
                            (lvasgn :e)
                            (block
                              (send
                                (ivar :@scheduler) :schedule)
                              (args)
                              (send nil :raise
                                (lvar :e)))) nil))
                      (if
                        (ivar :@keep_open) nil
                        (send nil :close))))))
              (def :<<
                (args
                  (arg :data))
                (begin
                  (block
                    (send
                      (ivar :@scheduler) :schedule)
                    (args)
                    (send
                      (ivar :@front) :call
                      (send
                        (lvar :data) :to_s)))
                  (self)))
              (def :callback
                (args
                  (blockarg :block))
                (begin
                  (if
                    (ivar :@closed)
                    (return
                      (yield)) nil)
                  (send
                    (ivar :@callbacks) :<<
                    (lvar :block))))
              (alias
                (sym :errback)
                (sym :callback))))
          (def :stream
            (args
              (optarg :keep_open
                (false)))
            (begin
              (lvasgn :scheduler
                (if
                  (send
                    (send nil :env) :[]
                    (str "async.callback"))
                  (const nil :EventMachine)
                  (const nil :Stream)))
              (lvasgn :current
                (send
                  (ivar :@params) :dup))
              (send nil :body
                (block
                  (send
                    (const nil :Stream) :new
                    (lvar :scheduler)
                    (lvar :keep_open))
                  (args
                    (procarg0 :out))
                  (block
                    (send nil :with_params
                      (lvar :current))
                    (args)
                    (yield
                      (lvar :out)))))))
          (def :cache_control
            (args
              (restarg :values))
            (begin
              (if
                (send
                  (send
                    (lvar :values) :last) :kind_of?
                  (const nil :Hash))
                (begin
                  (lvasgn :hash
                    (send
                      (lvar :values) :pop))
                  (block
                    (send
                      (lvar :hash) :reject!)
                    (args
                      (arg :k)
                      (arg :v))
                    (send
                      (lvar :v) :==
                      (false)))
                  (block
                    (send
                      (lvar :hash) :reject!)
                    (args
                      (arg :k)
                      (arg :v))
                    (if
                      (send
                        (lvar :v) :==
                        (true))
                      (send
                        (lvar :values) :<<
                        (lvar :k)) nil)))
                (lvasgn :hash
                  (hash)))
              (block
                (send
                  (lvar :values) :map!)
                (args
                  (procarg0 :value))
                (send
                  (send
                    (lvar :value) :to_s) :tr
                  (str "_")
                  (str "-")))
              (block
                (send
                  (lvar :hash) :each)
                (args
                  (arg :key)
                  (arg :value))
                (begin
                  (lvasgn :key
                    (send
                      (send
                        (lvar :key) :to_s) :tr
                      (str "_")
                      (str "-")))
                  (if
                    (send
                      (lvar :key) :==
                      (str "max-age"))
                    (lvasgn :value
                      (send
                        (lvar :value) :to_i)) nil)
                  (send
                    (lvar :values) :<<
                    (send
                      (array
                        (lvar :key)
                        (lvar :value)) :join
                      (str "=")))))
              (if
                (send
                  (lvar :values) :any?)
                (send
                  (send nil :response) :[]=
                  (str "Cache-Control")
                  (send
                    (lvar :values) :join
                    (str ", "))) nil)))
          (def :expires
            (args
              (arg :amount)
              (restarg :values))
            (begin
              (if
                (send
                  (send
                    (lvar :values) :last) :kind_of?
                  (const nil :Hash)) nil
                (send
                  (lvar :values) :<<
                  (hash)))
              (if
                (send
                  (lvar :amount) :is_a?
                  (const nil :Integer))
                (begin
                  (lvasgn :time
                    (send
                      (send
                        (const nil :Time) :now) :+
                      (send
                        (lvar :amount) :to_i)))
                  (lvasgn :max_age
                    (lvar :amount)))
                (begin
                  (lvasgn :time
                    (send nil :time_for
                      (lvar :amount)))
                  (lvasgn :max_age
                    (send
                      (lvar :time) :-
                      (send
                        (const nil :Time) :now)))))
              (send
                (send
                  (lvar :values) :last) :merge!
                (hash
                  (pair
                    (sym :max_age)
                    (lvar :max_age))))
              (send nil :cache_control
                (splat
                  (lvar :values)))
              (send
                (send nil :response) :[]=
                (str "Expires")
                (send
                  (lvar :time) :httpdate))))
          (def :last_modified
            (args
              (arg :time))
            (rescue
              (begin
                (if
                  (lvar :time) nil
                  (return))
                (lvasgn :time
                  (send nil :time_for
                    (lvar :time)))
                (send
                  (send nil :response) :[]=
                  (str "Last-Modified")
                  (send
                    (lvar :time) :httpdate))
                (if
                  (send
                    (send nil :env) :[]
                    (str "HTTP_IF_NONE_MATCH"))
                  (return) nil)
                (if
                  (and
                    (send
                      (send nil :status) :==
                      (int 200))
                    (send
                      (send nil :env) :[]
                      (str "HTTP_IF_MODIFIED_SINCE")))
                  (begin
                    (lvasgn :since
                      (send
                        (send
                          (const nil :Time) :httpdate
                          (send
                            (send nil :env) :[]
                            (str "HTTP_IF_MODIFIED_SINCE"))) :to_i))
                    (if
                      (send
                        (lvar :since) :>=
                        (send
                          (lvar :time) :to_i))
                      (send nil :halt
                        (int 304)) nil)) nil)
                (if
                  (and
                    (begin
                      (or
                        (send nil :success?)
                        (send
                          (send nil :status) :==
                          (int 412))))
                    (send
                      (send nil :env) :[]
                      (str "HTTP_IF_UNMODIFIED_SINCE")))
                  (begin
                    (lvasgn :since
                      (send
                        (send
                          (const nil :Time) :httpdate
                          (send
                            (send nil :env) :[]
                            (str "HTTP_IF_UNMODIFIED_SINCE"))) :to_i))
                    (if
                      (send
                        (lvar :since) :<
                        (send
                          (lvar :time) :to_i))
                      (send nil :halt
                        (int 412)) nil)) nil))
              (resbody
                (array
                  (const nil :ArgumentError)) nil nil) nil))
          (def :etag
            (args
              (arg :value)
              (optarg :options
                (hash)))
            (begin
              (if
                (send
                  (const nil :Hash) :===
                  (lvar :options)) nil
                (lvasgn :options
                  (hash
                    (pair
                      (sym :kind)
                      (lvar :options)))))
              (lvasgn :kind
                (or
                  (send
                    (lvar :options) :[]
                    (sym :kind))
                  (sym :strong)))
              (lvasgn :new_resource
                (block
                  (send
                    (lvar :options) :fetch
                    (sym :new_resource))
                  (args)
                  (send
                    (send nil :request) :post?)))
              (if
                (send
                  (array
                    (sym :strong)
                    (sym :weak)) :include?
                  (lvar :kind)) nil
                (send nil :raise
                  (const nil :ArgumentError)
                  (str ":strong or :weak expected")))
              (lvasgn :value
                (send
                  (str "\"%s\"") :%
                  (lvar :value)))
              (if
                (send
                  (lvar :kind) :==
                  (sym :weak))
                (lvasgn :value
                  (send
                    (str "W/") :+
                    (lvar :value))) nil)
              (send
                (send nil :response) :[]=
                (str "ETag")
                (lvar :value))
              (if
                (or
                  (send nil :success?)
                  (send
                    (send nil :status) :==
                    (int 304)))
                (begin
                  (if
                    (send nil :etag_matches?
                      (send
                        (send nil :env) :[]
                        (str "HTTP_IF_NONE_MATCH"))
                      (lvar :new_resource))
                    (send nil :halt
                      (if
                        (send
                          (send nil :request) :safe?)
                        (int 304)
                        (int 412))) nil)
                  (if
                    (send
                      (send nil :env) :[]
                      (str "HTTP_IF_MATCH"))
                    (if
                      (send nil :etag_matches?
                        (send
                          (send nil :env) :[]
                          (str "HTTP_IF_MATCH"))
                        (lvar :new_resource)) nil
                      (send nil :halt
                        (int 412))) nil)) nil)))
          (def :back
            (args)
            (send
              (send nil :request) :referer))
          (def :informational?
            (args)
            (send
              (send nil :status) :between?
              (int 100)
              (int 199)))
          (def :success?
            (args)
            (send
              (send nil :status) :between?
              (int 200)
              (int 299)))
          (def :redirect?
            (args)
            (send
              (send nil :status) :between?
              (int 300)
              (int 399)))
          (def :client_error?
            (args)
            (send
              (send nil :status) :between?
              (int 400)
              (int 499)))
          (def :server_error?
            (args)
            (send
              (send nil :status) :between?
              (int 500)
              (int 599)))
          (def :not_found?
            (args)
            (send
              (send nil :status) :==
              (int 404)))
          (def :time_for
            (args
              (arg :value))
            (rescue
              (if
                (send
                  (lvar :value) :respond_to?
                  (sym :to_time))
                (send
                  (lvar :value) :to_time)
                (if
                  (send
                    (lvar :value) :is_a?
                    (const nil :Time))
                  (lvar :value)
                  (if
                    (send
                      (lvar :value) :respond_to?
                      (sym :new_offset))
                    (begin
                      (lvasgn :d
                        (send
                          (lvar :value) :new_offset
                          (int 0)))
                      (lvasgn :t
                        (send
                          (const nil :Time) :utc
                          (send
                            (lvar :d) :year)
                          (send
                            (lvar :d) :mon)
                          (send
                            (lvar :d) :mday)
                          (send
                            (lvar :d) :hour)
                          (send
                            (lvar :d) :min)
                          (send
                            (send
                              (lvar :d) :sec) :+
                            (send
                              (lvar :d) :sec_fraction))))
                      (send
                        (lvar :t) :getlocal))
                    (if
                      (send
                        (lvar :value) :respond_to?
                        (sym :mday))
                      (send
                        (const nil :Time) :local
                        (send
                          (lvar :value) :year)
                        (send
                          (lvar :value) :mon)
                        (send
                          (lvar :value) :mday))
                      (if
                        (send
                          (lvar :value) :is_a?
                          (const nil :Numeric))
                        (send
                          (const nil :Time) :at
                          (lvar :value))
                        (send
                          (const nil :Time) :parse
                          (send
                            (lvar :value) :to_s)))))))
              (resbody
                (array
                  (const nil :ArgumentError))
                (lvasgn :boom)
                (send nil :raise
                  (lvar :boom)))
              (resbody
                (array
                  (const nil :Exception)) nil
                (send nil :raise
                  (const nil :ArgumentError)
                  (dstr
                    (str "unable to convert ")
                    (begin
                      (send
                        (lvar :value) :inspect))
                    (str " to a Time object")))) nil))
          (send nil :private)
          (def :etag_matches?
            (args
              (arg :list)
              (optarg :new_resource
                (send
                  (send nil :request) :post?)))
            (begin
              (if
                (send
                  (lvar :list) :==
                  (str "*"))
                (return
                  (send
                    (lvar :new_resource) :!)) nil)
              (send
                (send
                  (send
                    (lvar :list) :to_s) :split
                  (regexp
                    (str "\\s*,\\s*")
                    (regopt))) :include?
                (send
                  (send nil :response) :[]
                  (str "ETag")))))
          (def :with_params
            (args
              (arg :temp_params))
            (ensure
              (begin
                (masgn
                  (mlhs
                    (lvasgn :original)
                    (ivasgn :@params))
                  (array
                    (ivar :@params)
                    (lvar :temp_params)))
                (yield))
              (if
                (lvar :original)
                (ivasgn :@params
                  (lvar :original)) nil)))))
      (send nil :private)
      (module
        (const nil :Templates)
        (begin
          (module
            (const nil :ContentTyped)
            (send nil :attr_accessor
              (sym :content_type)))
          (def :initialize
            (args)
            (begin
              (zsuper)
              (ivasgn :@default_layout
                (sym :layout))))
          (def :erb
            (args
              (arg :template)
              (optarg :options
                (hash))
              (optarg :locals
                (hash)))
            (send nil :render
              (sym :erb)
              (lvar :template)
              (lvar :options)
              (lvar :locals)))
          (def :erubis
            (args
              (arg :template)
              (optarg :options
                (hash))
              (optarg :locals
                (hash)))
            (begin
              (send nil :warn
                (dstr
                  (str "Sinatra::Templates#erubis is deprecated and will be removed, use #erb instead.\n")
                  (str "If you have Erubis installed, it will be used automatically.")))
              (send nil :render
                (sym :erubis)
                (lvar :template)
                (lvar :options)
                (lvar :locals))))
          (def :haml
            (args
              (arg :template)
              (optarg :options
                (hash))
              (optarg :locals
                (hash)))
            (send nil :render
              (sym :haml)
              (lvar :template)
              (lvar :options)
              (lvar :locals)))
          (def :sass
            (args
              (arg :template)
              (optarg :options
                (hash))
              (optarg :locals
                (hash)))
            (begin
              (send
                (lvar :options) :merge!
                (hash
                  (pair
                    (sym :layout)
                    (false))
                  (pair
                    (sym :default_content_type)
                    (sym :css))))
              (send nil :render
                (sym :sass)
                (lvar :template)
                (lvar :options)
                (lvar :locals))))
          (def :scss
            (args
              (arg :template)
              (optarg :options
                (hash))
              (optarg :locals
                (hash)))
            (begin
              (send
                (lvar :options) :merge!
                (hash
                  (pair
                    (sym :layout)
                    (false))
                  (pair
                    (sym :default_content_type)
                    (sym :css))))
              (send nil :render
                (sym :scss)
                (lvar :template)
                (lvar :options)
                (lvar :locals))))
          (def :less
            (args
              (arg :template)
              (optarg :options
                (hash))
              (optarg :locals
                (hash)))
            (begin
              (send
                (lvar :options) :merge!
                (hash
                  (pair
                    (sym :layout)
                    (false))
                  (pair
                    (sym :default_content_type)
                    (sym :css))))
              (send nil :render
                (sym :less)
                (lvar :template)
                (lvar :options)
                (lvar :locals))))
          (def :builder
            (args
              (optarg :template
                (nil))
              (optarg :options
                (hash))
              (optarg :locals
                (hash))
              (blockarg :block))
            (begin
              (send
                (lvar :options) :[]=
                (sym :default_content_type)
                (sym :xml))
              (send nil :render_ruby
                (sym :builder)
                (lvar :template)
                (lvar :options)
                (lvar :locals)
                (block-pass
                  (lvar :block)))))
          (def :liquid
            (args
              (arg :template)
              (optarg :options
                (hash))
              (optarg :locals
                (hash)))
            (send nil :render
              (sym :liquid)
              (lvar :template)
              (lvar :options)
              (lvar :locals)))
          (def :markdown
            (args
              (arg :template)
              (optarg :options
                (hash))
              (optarg :locals
                (hash)))
            (send nil :render
              (sym :markdown)
              (lvar :template)
              (lvar :options)
              (lvar :locals)))
          (def :textile
            (args
              (arg :template)
              (optarg :options
                (hash))
              (optarg :locals
                (hash)))
            (send nil :render
              (sym :textile)
              (lvar :template)
              (lvar :options)
              (lvar :locals)))
          (def :rdoc
            (args
              (arg :template)
              (optarg :options
                (hash))
              (optarg :locals
                (hash)))
            (send nil :render
              (sym :rdoc)
              (lvar :template)
              (lvar :options)
              (lvar :locals)))
          (def :radius
            (args
              (arg :template)
              (optarg :options
                (hash))
              (optarg :locals
                (hash)))
            (send nil :render
              (sym :radius)
              (lvar :template)
              (lvar :options)
              (lvar :locals)))
          (def :markaby
            (args
              (optarg :template
                (nil))
              (optarg :options
                (hash))
              (optarg :locals
                (hash))
              (blockarg :block))
            (send nil :render_ruby
              (sym :mab)
              (lvar :template)
              (lvar :options)
              (lvar :locals)
              (block-pass
                (lvar :block))))
          (def :coffee
            (args
              (arg :template)
              (optarg :options
                (hash))
              (optarg :locals
                (hash)))
            (begin
              (send
                (lvar :options) :merge!
                (hash
                  (pair
                    (sym :layout)
                    (false))
                  (pair
                    (sym :default_content_type)
                    (sym :js))))
              (send nil :render
                (sym :coffee)
                (lvar :template)
                (lvar :options)
                (lvar :locals))))
          (def :nokogiri
            (args
              (optarg :template
                (nil))
              (optarg :options
                (hash))
              (optarg :locals
                (hash))
              (blockarg :block))
            (begin
              (send
                (lvar :options) :[]=
                (sym :default_content_type)
                (sym :xml))
              (send nil :render_ruby
                (sym :nokogiri)
                (lvar :template)
                (lvar :options)
                (lvar :locals)
                (block-pass
                  (lvar :block)))))
          (def :slim
            (args
              (arg :template)
              (optarg :options
                (hash))
              (optarg :locals
                (hash)))
            (send nil :render
              (sym :slim)
              (lvar :template)
              (lvar :options)
              (lvar :locals)))
          (def :creole
            (args
              (arg :template)
              (optarg :options
                (hash))
              (optarg :locals
                (hash)))
            (send nil :render
              (sym :creole)
              (lvar :template)
              (lvar :options)
              (lvar :locals)))
          (def :yajl
            (args
              (arg :template)
              (optarg :options
                (hash))
              (optarg :locals
                (hash)))
            (begin
              (send
                (lvar :options) :[]=
                (sym :default_content_type)
                (sym :json))
              (send nil :render
                (sym :yajl)
                (lvar :template)
                (lvar :options)
                (lvar :locals))))
          (def :find_template
            (args
              (arg :views)
              (arg :name)
              (arg :engine))
            (begin
              (yield
                (send
                  (const
                    (cbase) :File) :join
                  (lvar :views)
                  (dstr
                    (begin
                      (lvar :name))
                    (str ".")
                    (begin
                      (ivar :@preferred_extension)))))
              (block
                (send
                  (send
                    (const nil :Tilt) :mappings) :each)
                (args
                  (arg :ext)
                  (arg :engines))
                (begin
                  (if
                    (and
                      (send
                        (lvar :ext) :!=
                        (ivar :@preferred_extension))
                      (send
                        (lvar :engines) :include?
                        (lvar :engine))) nil
                    (next))
                  (yield
                    (send
                      (const
                        (cbase) :File) :join
                      (lvar :views)
                      (dstr
                        (begin
                          (lvar :name))
                        (str ".")
                        (begin
                          (lvar :ext)))))))))
          (send nil :private)
          (def :render_ruby
            (args
              (arg :engine)
              (arg :template)
              (optarg :options
                (hash))
              (optarg :locals
                (hash))
              (blockarg :block))
            (begin
              (if
                (send
                  (lvar :template) :is_a?
                  (const nil :Hash))
                (masgn
                  (mlhs
                    (lvasgn :options)
                    (lvasgn :template))
                  (array
                    (lvar :template)
                    (nil))) nil)
              (if
                (send
                  (lvar :template) :nil?)
                (lvasgn :template
                  (block
                    (send
                      (const nil :Proc) :new)
                    (args)
                    (lvar :block))) nil)
              (send nil :render
                (lvar :engine)
                (lvar :template)
                (lvar :options)
                (lvar :locals))))
          (def :render
            (args
              (arg :engine)
              (arg :data)
              (optarg :options
                (hash))
              (optarg :locals
                (hash))
              (blockarg :block))
            (begin
              (if
                (send
                  (send nil :settings) :respond_to?
                  (lvar :engine))
                (lvasgn :options
                  (send
                    (send
                      (send nil :settings) :send
                      (lvar :engine)) :merge
                    (lvar :options))) nil)
              (or-asgn
                (send
                  (lvar :options) :[]
                  (sym :outvar))
                (str "@_out_buf"))
              (or-asgn
                (send
                  (lvar :options) :[]
                  (sym :default_encoding))
                (send
                  (send nil :settings) :default_encoding))
              (lvasgn :locals
                (or
                  (or
                    (send
                      (lvar :options) :delete
                      (sym :locals))
                    (lvar :locals))
                  (hash)))
              (lvasgn :views
                (or
                  (or
                    (send
                      (lvar :options) :delete
                      (sym :views))
                    (send
                      (send nil :settings) :views))
                  (str "./views")))
              (lvasgn :layout
                (send
                  (lvar :options) :delete
                  (sym :layout)))
              (lvasgn :eat_errors
                (send
                  (lvar :layout) :nil?))
              (if
                (or
                  (send
                    (lvar :layout) :nil?)
                  (send
                    (lvar :layout) :==
                    (true)))
                (lvasgn :layout
                  (ivar :@default_layout)) nil)
              (lvasgn :content_type
                (or
                  (send
                    (lvar :options) :delete
                    (sym :content_type))
                  (send
                    (lvar :options) :delete
                    (sym :default_content_type))))
              (lvasgn :layout_engine
                (or
                  (send
                    (lvar :options) :delete
                    (sym :layout_engine))
                  (lvar :engine)))
              (lvasgn :scope
                (or
                  (send
                    (lvar :options) :delete
                    (sym :scope))
                  (self)))
              (kwbegin
                (ensure
                  (begin
                    (lvasgn :layout_was
                      (ivar :@default_layout))
                    (ivasgn :@default_layout
                      (false))
                    (lvasgn :template
                      (send nil :compile_template
                        (lvar :engine)
                        (lvar :data)
                        (lvar :options)
                        (lvar :views)))
                    (lvasgn :output
                      (send
                        (lvar :template) :render
                        (lvar :scope)
                        (lvar :locals)
                        (block-pass
                          (lvar :block)))))
                  (ivasgn :@default_layout
                    (lvar :layout_was))))
              (if
                (lvar :layout)
                (begin
                  (lvasgn :options
                    (send
                      (lvar :options) :merge
                      (hash
                        (pair
                          (sym :views)
                          (lvar :views))
                        (pair
                          (sym :layout)
                          (false))
                        (pair
                          (sym :eat_errors)
                          (lvar :eat_errors))
                        (pair
                          (sym :scope)
                          (lvar :scope)))))
                  (block
                    (send nil :catch
                      (sym :layout_missing))
                    (args)
                    (return
                      (block
                        (send nil :render
                          (lvar :layout_engine)
                          (lvar :layout)
                          (lvar :options)
                          (lvar :locals))
                        (args)
                        (lvar :output))))) nil)
              (if
                (lvar :content_type)
                (send
                  (send
                    (lvar :output) :extend
                    (const nil :ContentTyped)) :content_type=
                  (lvar :content_type)) nil)
              (lvar :output)))
          (def :compile_template
            (args
              (arg :engine)
              (arg :data)
              (arg :options)
              (arg :views))
            (begin
              (lvasgn :eat_errors
                (send
                  (lvar :options) :delete
                  (sym :eat_errors)))
              (block
                (send
                  (send nil :template_cache) :fetch
                  (lvar :engine)
                  (lvar :data)
                  (lvar :options))
                (args)
                (begin
                  (lvasgn :template
                    (send
                      (const nil :Tilt) :[]
                      (lvar :engine)))
                  (if
                    (send
                      (lvar :template) :nil?)
                    (send nil :raise
                      (dstr
                        (str "Template engine not found: ")
                        (begin
                          (lvar :engine)))) nil)
                  (case
                    (lvar :data)
                    (when
                      (const nil :Symbol)
                      (begin
                        (masgn
                          (mlhs
                            (lvasgn :body)
                            (lvasgn :path)
                            (lvasgn :line))
                          (send
                            (send
                              (send nil :settings) :templates) :[]
                            (lvar :data)))
                        (if
                          (lvar :body)
                          (begin
                            (if
                              (send
                                (lvar :body) :respond_to?
                                (sym :call))
                              (lvasgn :body
                                (send
                                  (lvar :body) :call)) nil)
                            (block
                              (send
                                (lvar :template) :new
                                (lvar :path)
                                (send
                                  (lvar :line) :to_i)
                                (lvar :options))
                              (args)
                              (lvar :body)))
                          (begin
                            (lvasgn :found
                              (false))
                            (ivasgn :@preferred_extension
                              (send
                                (lvar :engine) :to_s))
                            (block
                              (send nil :find_template
                                (lvar :views)
                                (lvar :data)
                                (lvar :template))
                              (args
                                (procarg0 :file))
                              (begin
                                (or-asgn
                                  (lvasgn :path)
                                  (lvar :file))
                                (if
                                  (lvasgn :found
                                    (send
                                      (const nil :File) :exists?
                                      (lvar :file)))
                                  (begin
                                    (lvasgn :path
                                      (lvar :file))
                                    (break)) nil)))
                            (if
                              (and
                                (lvar :eat_errors)
                                (send
                                  (lvar :found) :!))
                              (send nil :throw
                                (sym :layout_missing)) nil)
                            (send
                              (lvar :template) :new
                              (lvar :path)
                              (int 1)
                              (lvar :options))))))
                    (when
                      (const nil :Proc)
                      (const nil :String)
                      (begin
                        (lvasgn :body
                          (if
                            (send
                              (lvar :data) :is_a?
                              (const nil :String))
                            (block
                              (send
                                (const nil :Proc) :new)
                              (args)
                              (lvar :data))
                            (lvar :data)))
                        (masgn
                          (mlhs
                            (lvasgn :path)
                            (lvasgn :line))
                          (send
                            (send
                              (send nil :settings) :caller_locations) :first))
                        (send
                          (lvar :template) :new
                          (lvar :path)
                          (send
                            (lvar :line) :to_i)
                          (lvar :options)
                          (block-pass
                            (lvar :body)))))
                    (send nil :raise
                      (const nil :ArgumentError)
                      (dstr
                        (str "Sorry, don\'t know how to render ")
                        (begin
                          (send
                            (lvar :data) :inspect))
                        (str "."))))))))))
      (class
        (const nil :Base) nil
        (begin
          (send nil :include
            (const
              (const nil :Rack) :Utils))
          (send nil :include
            (const nil :Helpers))
          (send nil :include
            (const nil :Templates))
          (send nil :attr_accessor
            (sym :app))
          (send nil :attr_reader
            (sym :template_cache))
          (def :initialize
            (args
              (optarg :app
                (nil)))
            (begin
              (super)
              (ivasgn :@app
                (lvar :app))
              (ivasgn :@template_cache
                (send
                  (const
                    (const nil :Tilt) :Cache) :new))
              (if
                (send nil :block_given?)
                (yield
                  (self)) nil)))
          (def :call
            (args
              (arg :env))
            (send
              (send nil :dup) :call!
              (lvar :env)))
          (send nil :attr_accessor
            (sym :env)
            (sym :request)
            (sym :response)
            (sym :params))
          (def :call!
            (args
              (arg :env))
            (begin
              (ivasgn :@env
                (lvar :env))
              (ivasgn :@request
                (send
                  (const nil :Request) :new
                  (lvar :env)))
              (ivasgn :@response
                (send
                  (const nil :Response) :new))
              (ivasgn :@params
                (send nil :indifferent_params
                  (send
                    (ivar :@request) :params)))
              (if
                (send
                  (send nil :settings) :reload_templates)
                (send
                  (send nil :template_cache) :clear) nil)
              (send nil :force_encoding
                (ivar :@params))
              (send
                (ivar :@response) :[]=
                (str "Content-Type")
                (nil))
              (block
                (send nil :invoke)
                (args)
                (send nil :dispatch!))
              (block
                (send nil :invoke)
                (args)
                (send nil :error_block!
                  (send
                    (send nil :response) :status)))
              (if
                (send
                  (ivar :@response) :[]
                  (str "Content-Type")) nil
                (if
                  (and
                    (send
                      (const nil :Array) :===
                      (send nil :body))
                    (send
                      (send
                        (send nil :body) :[]
                        (int 0)) :respond_to?
                      (sym :content_type)))
                  (send nil :content_type
                    (send
                      (send
                        (send nil :body) :[]
                        (int 0)) :content_type))
                  (send nil :content_type
                    (sym :html))))
              (send
                (ivar :@response) :finish)))
          (defs
            (self) :settings
            (args)
            (self))
          (def :settings
            (args)
            (send
              (send
                (self) :class) :settings))
          (def :options
            (args)
            (begin
              (send nil :warn
                (dstr
                  (str "Sinatra::Base#options is deprecated and will be removed, ")
                  (str "use #settings instead.")))
              (send nil :settings)))
          (def :halt
            (args
              (restarg :response))
            (begin
              (if
                (send
                  (send
                    (lvar :response) :length) :==
                  (int 1))
                (lvasgn :response
                  (send
                    (lvar :response) :first)) nil)
              (send nil :throw
                (sym :halt)
                (lvar :response))))
          (def :pass
            (args
              (blockarg :block))
            (send nil :throw
              (sym :pass)
              (lvar :block)))
          (def :forward
            (args)
            (begin
              (if
                (send
                  (ivar :@app) :respond_to?
                  (sym :call)) nil
                (send nil :fail
                  (str "downstream app not set")))
              (masgn
                (mlhs
                  (lvasgn :status)
                  (lvasgn :headers)
                  (lvasgn :body))
                (send
                  (ivar :@app) :call
                  (send nil :env)))
              (send
                (ivar :@response) :status=
                (lvar :status))
              (send
                (ivar :@response) :body=
                (lvar :body))
              (send
                (send
                  (ivar :@response) :headers) :merge!
                (lvar :headers))
              (nil)))
          (send nil :private)
          (def :filter!
            (args
              (arg :type)
              (optarg :base
                (send nil :settings)))
            (begin
              (if
                (send
                  (send
                    (lvar :base) :superclass) :respond_to?
                  (sym :filters))
                (send nil :filter!
                  (lvar :type)
                  (send
                    (lvar :base) :superclass)) nil)
              (block
                (send
                  (send
                    (send
                      (lvar :base) :filters) :[]
                    (lvar :type)) :each)
                (args
                  (procarg0 :args))
                (send nil :process_route
                  (splat
                    (lvar :args))))))
          (def :route!
            (args
              (optarg :base
                (send nil :settings))
              (optarg :pass_block
                (nil)))
            (begin
              (if
                (lvasgn :routes
                  (send
                    (send
                      (lvar :base) :routes) :[]
                    (send
                      (ivar :@request) :request_method)))
                (block
                  (send
                    (lvar :routes) :each)
                  (args
                    (arg :pattern)
                    (arg :keys)
                    (arg :conditions)
                    (arg :block))
                  (lvasgn :pass_block
                    (block
                      (send nil :process_route
                        (lvar :pattern)
                        (lvar :keys)
                        (lvar :conditions))
                      (args
                        (restarg :args))
                      (block
                        (send nil :route_eval)
                        (args)
                        (send
                          (lvar :block) :[]
                          (splat
                            (lvar :args))))))) nil)
              (if
                (send
                  (send
                    (lvar :base) :superclass) :respond_to?
                  (sym :routes))
                (return
                  (send nil :route!
                    (send
                      (lvar :base) :superclass)
                    (lvar :pass_block))) nil)
              (if
                (lvar :pass_block)
                (send nil :route_eval
                  (block-pass
                    (lvar :pass_block))) nil)
              (send nil :route_missing)))
          (def :route_eval
            (args)
            (send nil :throw
              (sym :halt)
              (yield)))
          (def :process_route
            (args
              (arg :pattern)
              (arg :keys)
              (arg :conditions)
              (optarg :block
                (nil))
              (optarg :values
                (array)))
            (ensure
              (begin
                (lvasgn :route
                  (send
                    (ivar :@request) :path_info))
                (if
                  (and
                    (send
                      (lvar :route) :empty?)
                    (send
                      (send
                        (send nil :settings) :empty_path_info?) :!))
                  (lvasgn :route
                    (str "/")) nil)
                (if
                  (lvasgn :match
                    (send
                      (lvar :pattern) :match
                      (lvar :route))) nil
                  (return))
                (op-asgn
                  (lvasgn :values) :+
                  (block
                    (send
                      (send
                        (send
                          (lvar :match) :captures) :to_a) :map)
                    (args
                      (procarg0 :v))
                    (if
                      (lvar :v)
                      (send nil :force_encoding
                        (send
                          (const nil :URI) :decode_www_form_component
                          (lvar :v))) nil)))
                (if
                  (send
                    (lvar :values) :any?)
                  (begin
                    (masgn
                      (mlhs
                        (lvasgn :original)
                        (ivasgn :@params))
                      (array
                        (send nil :params)
                        (send
                          (send nil :params) :merge
                          (hash
                            (pair
                              (str "splat")
                              (array))
                            (pair
                              (str "captures")
                              (lvar :values))))))
                    (block
                      (send
                        (lvar :keys) :zip
                        (lvar :values))
                      (args
                        (arg :k)
                        (arg :v))
                      (if
                        (lvar :v)
                        (if
                          (send
                            (const nil :Array) :===
                            (send
                              (ivar :@params) :[]
                              (lvar :k)))
                          (send
                            (send
                              (ivar :@params) :[]
                              (lvar :k)) :<<
                            (lvar :v))
                          (send
                            (ivar :@params) :[]=
                            (lvar :k)
                            (lvar :v))) nil))) nil)
                (block
                  (send nil :catch
                    (sym :pass))
                  (args)
                  (begin
                    (block
                      (send
                        (lvar :conditions) :each)
                      (args
                        (procarg0 :c))
                      (if
                        (send
                          (send
                            (send
                              (lvar :c) :bind
                              (self)) :call) :==
                          (false))
                        (send nil :throw
                          (sym :pass)) nil))
                    (if
                      (lvar :block)
                      (send
                        (lvar :block) :[]
                        (self)
                        (lvar :values))
                      (yield
                        (self)
                        (lvar :values))))))
              (if
                (lvar :original)
                (ivasgn :@params
                  (lvar :original)) nil)))
          (def :route_missing
            (args)
            (if
              (ivar :@app)
              (send nil :forward)
              (send nil :raise
                (const nil :NotFound))))
          (def :static!
            (args)
            (begin
              (if
                (send
                  (begin
                    (lvasgn :public_dir
                      (send
                        (send nil :settings) :public_folder))) :nil?)
                (return) nil)
              (lvasgn :public_dir
                (send
                  (const nil :File) :expand_path
                  (lvar :public_dir)))
              (lvasgn :path
                (send
                  (const nil :File) :expand_path
                  (send
                    (lvar :public_dir) :+
                    (send nil :unescape
                      (send
                        (send nil :request) :path_info)))))
              (if
                (and
                  (send
                    (lvar :path) :start_with?
                    (lvar :public_dir))
                  (send
                    (const nil :File) :file?
                    (lvar :path))) nil
                (return))
              (send
                (send nil :env) :[]=
                (str "sinatra.static_file")
                (lvar :path))
              (if
                (send
                  (send nil :settings) :static_cache_control?)
                (send nil :cache_control
                  (splat
                    (send
                      (send nil :settings) :static_cache_control))) nil)
              (send nil :send_file
                (lvar :path)
                (hash
                  (pair
                    (sym :disposition)
                    (nil))))))
          (def :indifferent_params
            (args
              (arg :object))
            (case
              (lvar :object)
              (when
                (const nil :Hash)
                (begin
                  (lvasgn :new_hash
                    (send nil :indifferent_hash))
                  (block
                    (send
                      (lvar :object) :each)
                    (args
                      (arg :key)
                      (arg :value))
                    (send
                      (lvar :new_hash) :[]=
                      (lvar :key)
                      (send nil :indifferent_params
                        (lvar :value))))
                  (lvar :new_hash)))
              (when
                (const nil :Array)
                (block
                  (send
                    (lvar :object) :map)
                  (args
                    (procarg0 :item))
                  (send nil :indifferent_params
                    (lvar :item))))
              (lvar :object)))
          (def :indifferent_hash
            (args)
            (block
              (send
                (const nil :Hash) :new)
              (args
                (arg :hash)
                (arg :key))
              (if
                (send
                  (const nil :Symbol) :===
                  (lvar :key))
                (send
                  (lvar :hash) :[]
                  (send
                    (lvar :key) :to_s)) nil)))
          (def :invoke
            (args)
            (begin
              (lvasgn :res
                (block
                  (send nil :catch
                    (sym :halt))
                  (args)
                  (yield)))
              (if
                (or
                  (send
                    (const nil :Fixnum) :===
                    (lvar :res))
                  (send
                    (const nil :String) :===
                    (lvar :res)))
                (lvasgn :res
                  (array
                    (lvar :res))) nil)
              (if
                (and
                  (send
                    (const nil :Array) :===
                    (lvar :res))
                  (send
                    (const nil :Fixnum) :===
                    (send
                      (lvar :res) :first)))
                (begin
                  (send nil :status
                    (send
                      (lvar :res) :shift))
                  (send nil :body
                    (send
                      (lvar :res) :pop))
                  (send nil :headers
                    (splat
                      (lvar :res))))
                (if
                  (send
                    (lvar :res) :respond_to?
                    (sym :each))
                  (send nil :body
                    (lvar :res)) nil))
              (nil)))
          (def :dispatch!
            (args)
            (ensure
              (rescue
                (block
                  (send nil :invoke)
                  (args)
                  (begin
                    (if
                      (and
                        (send
                          (send nil :settings) :static?)
                        (begin
                          (or
                            (send
                              (send nil :request) :get?)
                            (send
                              (send nil :request) :head?))))
                      (send nil :static!) nil)
                    (send nil :filter!
                      (sym :before))
                    (send nil :route!)))
                (resbody
                  (array
                    (const
                      (cbase) :Exception))
                  (lvasgn :boom)
                  (block
                    (send nil :invoke)
                    (args)
                    (send nil :handle_exception!
                      (lvar :boom)))) nil)
              (if
                (send
                  (send nil :env) :[]
                  (str "sinatra.static_file")) nil
                (send nil :filter!
                  (sym :after)))))
          (def :handle_exception!
            (args
              (arg :boom))
            (begin
              (send
                (ivar :@env) :[]=
                (str "sinatra.error")
                (lvar :boom))
              (if
                (send
                  (lvar :boom) :respond_to?
                  (sym :http_status))
                (send nil :status
                  (send
                    (lvar :boom) :http_status))
                (if
                  (and
                    (and
                      (send
                        (send nil :settings) :use_code?)
                      (send
                        (lvar :boom) :respond_to?
                        (sym :code)))
                    (send
                      (send
                        (lvar :boom) :code) :between?
                      (int 400)
                      (int 599)))
                  (send nil :status
                    (send
                      (lvar :boom) :code))
                  (send nil :status
                    (int 500))))
              (if
                (send
                  (send nil :status) :between?
                  (int 400)
                  (int 599)) nil
                (send nil :status
                  (int 500)))
              (if
                (send nil :server_error?)
                (begin
                  (if
                    (send
                      (send nil :settings) :dump_errors?)
                    (send nil :dump_errors!
                      (lvar :boom)) nil)
                  (if
                    (and
                      (send
                        (send nil :settings) :show_exceptions?)
                      (send
                        (send
                          (send nil :settings) :show_exceptions) :!=
                        (sym :after_handler)))
                    (send nil :raise
                      (lvar :boom)) nil)) nil)
              (if
                (send nil :not_found?)
                (begin
                  (send
                    (send nil :headers) :[]=
                    (str "X-Cascade")
                    (str "pass"))
                  (send nil :body
                    (str "<h1>Not Found</h1>"))) nil)
              (lvasgn :res
                (or
                  (send nil :error_block!
                    (send
                      (lvar :boom) :class)
                    (lvar :boom))
                  (send nil :error_block!
                    (send nil :status)
                    (lvar :boom))))
              (if
                (or
                  (lvar :res)
                  (send
                    (send nil :server_error?) :!))
                (return
                  (lvar :res)) nil)
              (if
                (or
                  (send
                    (send nil :settings) :raise_errors?)
                  (send
                    (send nil :settings) :show_exceptions?))
                (send nil :raise
                  (lvar :boom)) nil)
              (send nil :error_block!
                (const nil :Exception)
                (lvar :boom))))
          (def :error_block!
            (args
              (arg :key)
              (restarg :block_params))
            (begin
              (lvasgn :base
                (send nil :settings))
              (while
                (send
                  (lvar :base) :respond_to?
                  (sym :errors))
                (begin
                  (if
                    (lvasgn :args_array
                      (send
                        (send
                          (lvar :base) :errors) :[]
                        (lvar :key))) nil
                    (next
                      (lvasgn :base
                        (send
                          (lvar :base) :superclass))))
                  (block
                    (send
                      (lvar :args_array) :reverse_each)
                    (args
                      (procarg0 :args))
                    (begin
                      (lvasgn :first
                        (send
                          (lvar :args) :==
                          (send
                            (lvar :args_array) :first)))
                      (op-asgn
                        (lvasgn :args) :+
                        (array
                          (lvar :block_params)))
                      (lvasgn :resp
                        (send nil :process_route
                          (splat
                            (lvar :args))))
                      (if
                        (and
                          (send
                            (lvar :resp) :nil?)
                          (send
                            (lvar :first) :!)) nil
                        (return
                          (lvar :resp)))))))
              (if
                (and
                  (send
                    (lvar :key) :respond_to?
                    (sym :superclass))
                  (send
                    (send
                      (lvar :key) :superclass) :<
                    (const nil :Exception))) nil
                (return
                  (false)))
              (send nil :error_block!
                (send
                  (lvar :key) :superclass)
                (splat
                  (lvar :block_params)))))
          (def :dump_errors!
            (args
              (arg :boom))
            (begin
              (lvasgn :msg
                (send
                  (array
                    (dstr
                      (begin
                        (send
                          (lvar :boom) :class))
                      (str " - ")
                      (begin
                        (send
                          (lvar :boom) :message))
                      (str ":"))
                    (splat
                      (send
                        (lvar :boom) :backtrace))) :join
                  (str "\n\t")))
              (send
                (send
                  (ivar :@env) :[]
                  (str "rack.errors")) :puts
                (lvar :msg))))
          (sclass
            (self)
            (begin
              (send nil :attr_reader
                (sym :routes)
                (sym :filters)
                (sym :templates)
                (sym :errors))
              (def :reset!
                (args)
                (begin
                  (ivasgn :@conditions
                    (array))
                  (ivasgn :@routes
                    (hash))
                  (ivasgn :@filters
                    (hash
                      (pair
                        (sym :before)
                        (array))
                      (pair
                        (sym :after)
                        (array))))
                  (ivasgn :@errors
                    (hash))
                  (ivasgn :@middleware
                    (array))
                  (ivasgn :@prototype
                    (nil))
                  (ivasgn :@extensions
                    (array))
                  (if
                    (send
                      (send nil :superclass) :respond_to?
                      (sym :templates))
                    (ivasgn :@templates
                      (block
                        (send
                          (const nil :Hash) :new)
                        (args
                          (arg :hash)
                          (arg :key))
                        (send
                          (send
                            (send nil :superclass) :templates) :[]
                          (lvar :key))))
                    (ivasgn :@templates
                      (hash)))))
              (def :extensions
                (args)
                (if
                  (send
                    (send nil :superclass) :respond_to?
                    (sym :extensions))
                  (send
                    (begin
                      (send
                        (ivar :@extensions) :+
                        (send
                          (send nil :superclass) :extensions))) :uniq)
                  (ivar :@extensions)))
              (def :middleware
                (args)
                (if
                  (send
                    (send nil :superclass) :respond_to?
                    (sym :middleware))
                  (send
                    (send
                      (send nil :superclass) :middleware) :+
                    (ivar :@middleware))
                  (ivar :@middleware)))
              (def :set
                (args
                  (arg :option)
                  (optarg :value
                    (begin
                      (lvasgn :not_set
                        (true))))
                  (optarg :ignore_setter
                    (false))
                  (blockarg :block))
                (begin
                  (if
                    (and
                      (lvar :block)
                      (send
                        (lvar :not_set) :!))
                    (send nil :raise
                      (const nil :ArgumentError)) nil)
                  (if
                    (lvar :block)
                    (masgn
                      (mlhs
                        (lvasgn :value)
                        (lvasgn :not_set))
                      (array
                        (lvar :block)
                        (false))) nil)
                  (if
                    (lvar :not_set)
                    (begin
                      (if
                        (send
                          (lvar :option) :respond_to?
                          (sym :each)) nil
                        (send nil :raise
                          (const nil :ArgumentError)))
                      (block
                        (send
                          (lvar :option) :each)
                        (args
                          (arg :k)
                          (arg :v))
                        (send nil :set
                          (lvar :k)
                          (lvar :v)))
                      (return
                        (self))) nil)
                  (if
                    (and
                      (send nil :respond_to?
                        (dstr
                          (begin
                            (lvar :option))
                          (str "=")))
                      (send
                        (lvar :ignore_setter) :!))
                    (return
                      (send nil :__send__
                        (dstr
                          (begin
                            (lvar :option))
                          (str "="))
                        (lvar :value))) nil)
                  (lvasgn :setter
                    (block
                      (send nil :proc)
                      (args
                        (procarg0 :val))
                      (send nil :set
                        (lvar :option)
                        (lvar :val)
                        (true))))
                  (lvasgn :getter
                    (block
                      (send nil :proc)
                      (args)
                      (lvar :value)))
                  (case
                    (lvar :value)
                    (when
                      (const nil :Proc)
                      (lvasgn :getter
                        (lvar :value)))
                    (when
                      (const nil :Symbol)
                      (const nil :Fixnum)
                      (const nil :FalseClass)
                      (const nil :TrueClass)
                      (const nil :NilClass)
                      (lvasgn :getter
                        (send
                          (lvar :value) :inspect)))
                    (when
                      (const nil :Hash)
                      (lvasgn :setter
                        (block
                          (send nil :proc)
                          (args
                            (procarg0 :val))
                          (begin
                            (if
                              (send
                                (const nil :Hash) :===
                                (lvar :val))
                              (lvasgn :val
                                (send
                                  (lvar :value) :merge
                                  (lvar :val))) nil)
                            (send nil :set
                              (lvar :option)
                              (lvar :val)
                              (true)))))) nil)
                  (if
                    (lvar :setter)
                    (send nil :define_singleton_method
                      (dstr
                        (begin
                          (lvar :option))
                        (str "="))
                      (lvar :setter)) nil)
                  (if
                    (lvar :getter)
                    (send nil :define_singleton_method
                      (lvar :option)
                      (lvar :getter)) nil)
                  (if
                    (send nil :method_defined?
                      (dstr
                        (begin
                          (lvar :option))
                        (str "?"))) nil
                    (send nil :define_singleton_method
                      (dstr
                        (begin
                          (lvar :option))
                        (str "?"))
                      (dstr
                        (str "!!")
                        (begin
                          (lvar :option)))))
                  (self)))
              (def :enable
                (args
                  (restarg :opts))
                (block
                  (send
                    (lvar :opts) :each)
                  (args
                    (procarg0 :key))
                  (send nil :set
                    (lvar :key)
                    (true))))
              (def :disable
                (args
                  (restarg :opts))
                (block
                  (send
                    (lvar :opts) :each)
                  (args
                    (procarg0 :key))
                  (send nil :set
                    (lvar :key)
                    (false))))
              (def :error
                (args
                  (restarg :codes)
                  (blockarg :block))
                (begin
                  (lvasgn :args
                    (send nil :compile!
                      (str "ERROR")
                      (regexp
                        (regopt))
                      (lvar :block)))
                  (lvasgn :codes
                    (send
                      (block
                        (send
                          (lvar :codes) :map)
                        (args
                          (procarg0 :c))
                        (send nil :Array
                          (lvar :c))) :flatten))
                  (if
                    (send
                      (lvar :codes) :empty?)
                    (send
                      (lvar :codes) :<<
                      (const nil :Exception)) nil)
                  (block
                    (send
                      (lvar :codes) :each)
                    (args
                      (procarg0 :c))
                    (send
                      (begin
                        (or-asgn
                          (send
                            (ivar :@errors) :[]
                            (lvar :c))
                          (array))) :<<
                      (lvar :args)))))
              (def :not_found
                (args
                  (blockarg :block))
                (send nil :error
                  (int 404)
                  (block-pass
                    (lvar :block))))
              (def :template
                (args
                  (arg :name)
                  (blockarg :block))
                (begin
                  (masgn
                    (mlhs
                      (lvasgn :filename)
                      (lvasgn :line))
                    (send
                      (send nil :caller_locations) :first))
                  (send
                    (send nil :templates) :[]=
                    (lvar :name)
                    (array
                      (lvar :block)
                      (lvar :filename)
                      (send
                        (lvar :line) :to_i)))))
              (def :layout
                (args
                  (optarg :name
                    (sym :layout))
                  (blockarg :block))
                (send nil :template
                  (lvar :name)
                  (block-pass
                    (lvar :block))))
              (def :inline_templates=
                (args
                  (optarg :file
                    (nil)))
                (begin
                  (lvasgn :file
                    (if
                      (begin
                        (or
                          (send
                            (lvar :file) :nil?)
                          (send
                            (lvar :file) :==
                            (true))))
                      (begin
                        (or
                          (send
                            (send nil :caller_files) :first)
                          (send
                            (const nil :File) :expand_path
                            (gvar :$0))))
                      (lvar :file)))
                  (kwbegin
                    (rescue
                      (begin
                        (lvasgn :io
                          (if
                            (send
                              (const
                                (cbase) :IO) :respond_to?
                              (sym :binread))
                            (send
                              (const
                                (cbase) :IO) :binread
                              (lvar :file))
                            (send
                              (const
                                (cbase) :IO) :read
                              (lvar :file))))
                        (masgn
                          (mlhs
                            (lvasgn :app)
                            (lvasgn :data))
                          (send
                            (send
                              (lvar :io) :gsub
                              (str "\r\n")
                              (str "\n")) :split
                            (regexp
                              (str "^__END__$")
                              (regopt))
                            (int 2))))
                      (resbody
                        (array
                          (const
                            (const nil :Errno) :ENOENT)) nil
                        (masgn
                          (mlhs
                            (lvasgn :app)
                            (lvasgn :data))
                          (nil))) nil))
                  (if
                    (lvar :data)
                    (begin
                      (if
                        (and
                          (lvar :app)
                          (send
                            (lvar :app) :=~
                            (regexp
                              (str "([^\\n]*\\n)?#[^\\n]*coding: *(\\S+)")
                              (regopt :m))))
                        (lvasgn :encoding
                          (nth-ref 2))
                        (lvasgn :encoding
                          (send
                            (send nil :settings) :default_encoding)))
                      (lvasgn :lines
                        (send
                          (send
                            (lvar :app) :count
                            (str "\n")) :+
                          (int 1)))
                      (lvasgn :template
                        (nil))
                      (send nil :force_encoding
                        (lvar :data)
                        (lvar :encoding))
                      (block
                        (send
                          (lvar :data) :each_line)
                        (args
                          (procarg0 :line))
                        (begin
                          (op-asgn
                            (lvasgn :lines) :+
                            (int 1))
                          (if
                            (send
                              (lvar :line) :=~
                              (regexp
                                (str "^@@\\s*(.*\\S)\\s*$")
                                (regopt)))
                            (begin
                              (lvasgn :template
                                (send nil :force_encoding
                                  (str "")
                                  (lvar :encoding)))
                              (send
                                (send nil :templates) :[]=
                                (send
                                  (nth-ref 1) :to_sym)
                                (array
                                  (lvar :template)
                                  (lvar :file)
                                  (lvar :lines))))
                            (if
                              (lvar :template)
                              (send
                                (lvar :template) :<<
                                (lvar :line)) nil))))) nil)))
              (def :mime_type
                (args
                  (arg :type)
                  (optarg :value
                    (nil)))
                (begin
                  (if
                    (or
                      (send
                        (lvar :type) :nil?)
                      (send
                        (send
                          (lvar :type) :to_s) :include?
                        (str "/")))
                    (return
                      (lvar :type)) nil)
                  (if
                    (send
                      (send
                        (send
                          (lvar :type) :to_s) :[]
                        (int 0)) :==
                      (str ".")) nil
                    (lvasgn :type
                      (dstr
                        (str ".")
                        (begin
                          (lvar :type)))))
                  (if
                    (lvar :value) nil
                    (return
                      (send
                        (const
                          (const nil :Rack) :Mime) :mime_type
                        (lvar :type)
                        (nil))))
                  (send
                    (const
                      (const
                        (const nil :Rack) :Mime) :MIME_TYPES) :[]=
                    (lvar :type)
                    (lvar :value))))
              (def :mime_types
                (args
                  (arg :type))
                (begin
                  (lvasgn :type
                    (send nil :mime_type
                      (lvar :type)))
                  (if
                    (send
                      (lvar :type) :=~
                      (regexp
                        (str "^application/(xml|javascript)$")
                        (regopt)))
                    (array
                      (lvar :type)
                      (dstr
                        (str "text/")
                        (nth-ref 1)))
                    (array
                      (lvar :type)))))
              (def :before
                (args
                  (optarg :path
                    (nil))
                  (optarg :options
                    (hash))
                  (blockarg :block))
                (send nil :add_filter
                  (sym :before)
                  (lvar :path)
                  (lvar :options)
                  (block-pass
                    (lvar :block))))
              (def :after
                (args
                  (optarg :path
                    (nil))
                  (optarg :options
                    (hash))
                  (blockarg :block))
                (send nil :add_filter
                  (sym :after)
                  (lvar :path)
                  (lvar :options)
                  (block-pass
                    (lvar :block))))
              (def :add_filter
                (args
                  (arg :type)
                  (optarg :path
                    (nil))
                  (optarg :options
                    (hash))
                  (blockarg :block))
                (begin
                  (if
                    (send
                      (lvar :path) :respond_to?
                      (sym :each_pair))
                    (masgn
                      (mlhs
                        (lvasgn :path)
                        (lvasgn :options))
                      (array
                        (regexp
                          (regopt))
                        (lvar :path))) nil)
                  (send
                    (send
                      (send nil :filters) :[]
                      (lvar :type)) :<<
                    (send nil :compile!
                      (lvar :type)
                      (or
                        (lvar :path)
                        (regexp
                          (regopt)))
                      (lvar :block)
                      (lvar :options)))))
              (def :condition
                (args
                  (optarg :name
                    (dstr
                      (begin
                        (send
                          (send
                            (send nil :caller) :first) :[]
                          (regexp
                            (str "`.*\'")
                            (regopt))))
                      (str " condition")))
                  (blockarg :block))
                (send
                  (ivar :@conditions) :<<
                  (send nil :generate_method
                    (lvar :name)
                    (block-pass
                      (lvar :block)))))
              (def :public=
                (args
                  (arg :value))
                (begin
                  (send nil :warn
                    (str ":public is no longer used to avoid overloading Module#public, use :public_dir instead"))
                  (send nil :set
                    (sym :public_folder)
                    (lvar :value))))
              (def :public_dir=
                (args
                  (arg :value))
                (send
                  (self) :public_folder=
                  (lvar :value)))
              (def :public_dir
                (args)
                (send nil :public_folder))
              (send nil :private)
              (def :define_singleton_method
                (args
                  (arg :name)
                  (optarg :content
                    (send
                      (const nil :Proc) :new)))
                (block
                  (send
                    (begin
                      (sclass
                        (self)
                        (self))) :class_eval)
                  (args)
                  (begin
                    (if
                      (send nil :method_defined?
                        (lvar :name))
                      (send nil :undef_method
                        (lvar :name)) nil)
                    (if
                      (send
                        (const nil :String) :===
                        (lvar :content))
                      (send nil :class_eval
                        (dstr
                          (str "def ")
                          (begin
                            (lvar :name))
                          (str "() ")
                          (begin
                            (lvar :content))
                          (str "; end")))
                      (send nil :define_method
                        (lvar :name)
                        (block-pass
                          (lvar :content)))))))
              (def :host_name
                (args
                  (arg :pattern))
                (block
                  (send nil :condition)
                  (args)
                  (send
                    (lvar :pattern) :===
                    (send
                      (send nil :request) :host))))
              (def :user_agent
                (args
                  (arg :pattern))
                (block
                  (send nil :condition)
                  (args)
                  (if
                    (send
                      (send
                        (send
                          (send nil :request) :user_agent) :to_s) :=~
                      (lvar :pattern))
                    (begin
                      (send
                        (ivar :@params) :[]=
                        (sym :agent)
                        (send
                          (gvar :$~) :[]
                          (irange
                            (int 1)
                            (int -1))))
                      (true))
                    (false))))
              (send nil :alias_method
                (sym :agent)
                (sym :user_agent))
              (def :provides
                (args
                  (restarg :types))
                (begin
                  (block
                    (send
                      (lvar :types) :map!)
                    (args
                      (procarg0 :t))
                    (send nil :mime_types
                      (lvar :t)))
                  (send
                    (lvar :types) :flatten!)
                  (block
                    (send nil :condition)
                    (args)
                    (if
                      (lvasgn :type
                        (send
                          (send nil :response) :[]
                          (str "Content-Type")))
                      (or
                        (send
                          (lvar :types) :include?
                          (lvar :type))
                        (send
                          (lvar :types) :include?
                          (send
                            (lvar :type) :[]
                            (regexp
                              (str "^[^;]+")
                              (regopt)))))
                      (if
                        (lvasgn :type
                          (send
                            (send nil :request) :preferred_type
                            (lvar :types)))
                        (begin
                          (send nil :content_type
                            (lvar :type))
                          (true))
                        (false))))))
              (send nil :public)
              (def :get
                (args
                  (arg :path)
                  (optarg :opts
                    (hash))
                  (blockarg :block))
                (begin
                  (lvasgn :conditions
                    (send
                      (ivar :@conditions) :dup))
                  (send nil :route
                    (str "GET")
                    (lvar :path)
                    (lvar :opts)
                    (block-pass
                      (lvar :block)))
                  (ivasgn :@conditions
                    (lvar :conditions))
                  (send nil :route
                    (str "HEAD")
                    (lvar :path)
                    (lvar :opts)
                    (block-pass
                      (lvar :block)))))
              (def :put
                (args
                  (arg :path)
                  (optarg :opts
                    (hash))
                  (blockarg :bk))
                (send nil :route
                  (str "PUT")
                  (lvar :path)
                  (lvar :opts)
                  (block-pass
                    (lvar :bk))))
              (def :post
                (args
                  (arg :path)
                  (optarg :opts
                    (hash))
                  (blockarg :bk))
                (send nil :route
                  (str "POST")
                  (lvar :path)
                  (lvar :opts)
                  (block-pass
                    (lvar :bk))))
              (def :delete
                (args
                  (arg :path)
                  (optarg :opts
                    (hash))
                  (blockarg :bk))
                (send nil :route
                  (str "DELETE")
                  (lvar :path)
                  (lvar :opts)
                  (block-pass
                    (lvar :bk))))
              (def :head
                (args
                  (arg :path)
                  (optarg :opts
                    (hash))
                  (blockarg :bk))
                (send nil :route
                  (str "HEAD")
                  (lvar :path)
                  (lvar :opts)
                  (block-pass
                    (lvar :bk))))
              (def :options
                (args
                  (arg :path)
                  (optarg :opts
                    (hash))
                  (blockarg :bk))
                (send nil :route
                  (str "OPTIONS")
                  (lvar :path)
                  (lvar :opts)
                  (block-pass
                    (lvar :bk))))
              (def :patch
                (args
                  (arg :path)
                  (optarg :opts
                    (hash))
                  (blockarg :bk))
                (send nil :route
                  (str "PATCH")
                  (lvar :path)
                  (lvar :opts)
                  (block-pass
                    (lvar :bk))))
              (send nil :private)
              (def :route
                (args
                  (arg :verb)
                  (arg :path)
                  (optarg :options
                    (hash))
                  (blockarg :block))
                (begin
                  (if
                    (send
                      (lvar :options) :key?
                      (sym :host))
                    (send nil :host_name
                      (send
                        (lvar :options) :delete
                        (sym :host))) nil)
                  (if
                    (and
                      (send
                        (lvar :path) :==
                        (str ""))
                      (send
                        (send nil :empty_path_info) :nil?))
                    (send nil :enable
                      (sym :empty_path_info)) nil)
                  (lvasgn :signature
                    (send nil :compile!
                      (lvar :verb)
                      (lvar :path)
                      (lvar :block)
                      (lvar :options)))
                  (send
                    (begin
                      (or-asgn
                        (send
                          (ivar :@routes) :[]
                          (lvar :verb))
                        (array))) :<<
                    (lvar :signature))
                  (send nil :invoke_hook
                    (sym :route_added)
                    (lvar :verb)
                    (lvar :path)
                    (lvar :block))
                  (lvar :signature)))
              (def :invoke_hook
                (args
                  (arg :name)
                  (restarg :args))
                (block
                  (send
                    (send nil :extensions) :each)
                  (args
                    (procarg0 :e))
                  (if
                    (send
                      (lvar :e) :respond_to?
                      (lvar :name))
                    (send
                      (lvar :e) :send
                      (lvar :name)
                      (splat
                        (lvar :args))) nil)))
              (def :generate_method
                (args
                  (arg :method_name)
                  (blockarg :block))
                (begin
                  (send nil :define_method
                    (lvar :method_name)
                    (block-pass
                      (lvar :block)))
                  (lvasgn :method
                    (send nil :instance_method
                      (lvar :method_name)))
                  (send nil :remove_method
                    (lvar :method_name))
                  (lvar :method)))
              (def :compile!
                (args
                  (arg :verb)
                  (arg :path)
                  (arg :block)
                  (optarg :options
                    (hash)))
                (begin
                  (block
                    (send
                      (lvar :options) :each_pair)
                    (args
                      (arg :option)
                      (arg :args))
                    (send nil :send
                      (lvar :option)
                      (splat
                        (lvar :args))))
                  (lvasgn :method_name
                    (dstr
                      (begin
                        (lvar :verb))
                      (str " ")
                      (begin
                        (lvar :path))))
                  (lvasgn :unbound_method
                    (send nil :generate_method
                      (lvar :method_name)
                      (block-pass
                        (lvar :block))))
                  (masgn
                    (mlhs
                      (lvasgn :pattern)
                      (lvasgn :keys))
                    (send nil :compile
                      (lvar :path)))
                  (masgn
                    (mlhs
                      (lvasgn :conditions)
                      (ivasgn :@conditions))
                    (array
                      (ivar :@conditions)
                      (array)))
                  (array
                    (lvar :pattern)
                    (lvar :keys)
                    (lvar :conditions)
                    (if
                      (send
                        (send
                          (lvar :block) :arity) :!=
                        (int 0))
                      (block
                        (send nil :proc)
                        (args
                          (arg :a)
                          (arg :p))
                        (send
                          (send
                            (lvar :unbound_method) :bind
                            (lvar :a)) :call
                          (splat
                            (lvar :p))))
                      (block
                        (send nil :proc)
                        (args
                          (arg :a)
                          (arg :p))
                        (send
                          (send
                            (lvar :unbound_method) :bind
                            (lvar :a)) :call))))))
              (def :compile
                (args
                  (arg :path))
                (begin
                  (lvasgn :keys
                    (array))
                  (if
                    (send
                      (lvar :path) :respond_to?
                      (sym :to_str))
                    (begin
                      (lvasgn :pattern
                        (block
                          (send
                            (send
                              (lvar :path) :to_str) :gsub
                            (regexp
                              (str "[^\\?\\%\\\\/\\:\\*\\w]")
                              (regopt)))
                          (args
                            (procarg0 :c))
                          (send nil :encoded
                            (lvar :c))))
                      (block
                        (send
                          (lvar :pattern) :gsub!
                          (regexp
                            (str "((:\\w+)|\\*)")
                            (regopt)))
                        (args
                          (procarg0 :match))
                        (if
                          (send
                            (lvar :match) :==
                            (str "*"))
                          (begin
                            (send
                              (lvar :keys) :<<
                              (str "splat"))
                            (str "(.*?)"))
                          (begin
                            (send
                              (lvar :keys) :<<
                              (send
                                (nth-ref 2) :[]
                                (irange
                                  (int 1)
                                  (int -1))))
                            (str "([^/?#]+)"))))
                      (array
                        (regexp
                          (str "^")
                          (begin
                            (lvar :pattern))
                          (str "$")
                          (regopt))
                        (lvar :keys)))
                    (if
                      (and
                        (send
                          (lvar :path) :respond_to?
                          (sym :keys))
                        (send
                          (lvar :path) :respond_to?
                          (sym :match)))
                      (array
                        (lvar :path)
                        (send
                          (lvar :path) :keys))
                      (if
                        (and
                          (send
                            (lvar :path) :respond_to?
                            (sym :names))
                          (send
                            (lvar :path) :respond_to?
                            (sym :match)))
                        (array
                          (lvar :path)
                          (send
                            (lvar :path) :names))
                        (if
                          (send
                            (lvar :path) :respond_to?
                            (sym :match))
                          (array
                            (lvar :path)
                            (lvar :keys))
                          (send nil :raise
                            (const nil :TypeError)
                            (lvar :path))))))))
              (casgn nil :URI
                (if
                  (send
                    (const
                      (cbase) :URI) :const_defined?
                    (sym :Parser))
                  (send
                    (const
                      (const
                        (cbase) :URI) :Parser) :new)
                  (const
                    (cbase) :URI)))
              (def :encoded
                (args
                  (arg :char))
                (begin
                  (lvasgn :enc
                    (send
                      (const nil :URI) :escape
                      (lvar :char)))
                  (if
                    (send
                      (lvar :enc) :==
                      (lvar :char))
                    (lvasgn :enc
                      (dstr
                        (str "(?:")
                        (begin
                          (send
                            (const nil :Regexp) :escape
                            (lvar :enc)))
                        (str "|")
                        (begin
                          (send
                            (const nil :URI) :escape
                            (lvar :char)
                            (regexp
                              (str ".")
                              (regopt))))
                        (str ")"))) nil)
                  (if
                    (send
                      (lvar :char) :==
                      (str " "))
                    (lvasgn :enc
                      (dstr
                        (str "(?:")
                        (begin
                          (lvar :enc))
                        (str "|")
                        (begin
                          (send nil :encoded
                            (str "+")))
                        (str ")"))) nil)
                  (lvar :enc)))
              (send nil :public)
              (def :helpers
                (args
                  (restarg :extensions)
                  (blockarg :block))
                (begin
                  (if
                    (send nil :block_given?)
                    (send nil :class_eval
                      (block-pass
                        (lvar :block))) nil)
                  (if
                    (send
                      (lvar :extensions) :any?)
                    (send nil :include
                      (splat
                        (lvar :extensions))) nil)))
              (def :register
                (args
                  (restarg :extensions)
                  (blockarg :block))
                (begin
                  (if
                    (send nil :block_given?)
                    (send
                      (lvar :extensions) :<<
                      (send
                        (const nil :Module) :new
                        (block-pass
                          (lvar :block)))) nil)
                  (op-asgn
                    (ivasgn :@extensions) :+
                    (lvar :extensions))
                  (block
                    (send
                      (lvar :extensions) :each)
                    (args
                      (procarg0 :extension))
                    (begin
                      (send nil :extend
                        (lvar :extension))
                      (if
                        (send
                          (lvar :extension) :respond_to?
                          (sym :registered))
                        (send
                          (lvar :extension) :registered
                          (self)) nil)))))
              (def :development?
                (args)
                (send
                  (send nil :environment) :==
                  (sym :development)))
              (def :production?
                (args)
                (send
                  (send nil :environment) :==
                  (sym :production)))
              (def :test?
                (args)
                (send
                  (send nil :environment) :==
                  (sym :test)))
              (def :configure
                (args
                  (restarg :envs)
                  (blockarg :block))
                (if
                  (or
                    (send
                      (lvar :envs) :empty?)
                    (send
                      (lvar :envs) :include?
                      (send
                        (send nil :environment) :to_sym)))
                  (yield
                    (self)) nil))
              (def :use
                (args
                  (arg :middleware)
                  (restarg :args)
                  (blockarg :block))
                (begin
                  (ivasgn :@prototype
                    (nil))
                  (send
                    (ivar :@middleware) :<<
                    (array
                      (lvar :middleware)
                      (lvar :args)
                      (lvar :block)))))
              (def :quit!
                (args
                  (arg :server)
                  (arg :handler_name))
                (begin
                  (if
                    (send
                      (lvar :server) :respond_to?
                      (sym :stop!))
                    (send
                      (lvar :server) :stop!)
                    (send
                      (lvar :server) :stop))
                  (if
                    (send
                      (lvar :handler_name) :=~
                      (regexp
                        (str "cgi")
                        (regopt :i))) nil
                    (send
                      (gvar :$stderr) :puts
                      (str "\n== Sinatra has ended his set (crowd applauds)")))))
              (def :run!
                (args
                  (optarg :options
                    (hash)))
                (rescue
                  (begin
                    (send nil :set
                      (lvar :options))
                    (lvasgn :handler
                      (send nil :detect_rack_handler))
                    (lvasgn :handler_name
                      (send
                        (send
                          (lvar :handler) :name) :gsub
                        (regexp
                          (str ".*::")
                          (regopt))
                        (str "")))
                    (lvasgn :server_settings
                      (if
                        (send
                          (send nil :settings) :respond_to?
                          (sym :server_settings))
                        (send
                          (send nil :settings) :server_settings)
                        (hash)))
                    (block
                      (send
                        (lvar :handler) :run
                        (self)
                        (send
                          (lvar :server_settings) :merge
                          (hash
                            (pair
                              (sym :Port)
                              (send nil :port))
                            (pair
                              (sym :Host)
                              (send nil :bind)))))
                      (args
                        (procarg0 :server))
                      (begin
                        (if
                          (send
                            (lvar :handler_name) :=~
                            (regexp
                              (str "cgi")
                              (regopt :i))) nil
                          (send
                            (gvar :$stderr) :puts
                            (send
                              (dstr
                                (str "== Sinatra/")
                                (begin
                                  (const
                                    (const nil :Sinatra) :VERSION))
                                (str " has taken the stage ")) :+
                              (dstr
                                (str "on ")
                                (begin
                                  (send nil :port))
                                (str " for ")
                                (begin
                                  (send nil :environment))
                                (str " with backup from ")
                                (begin
                                  (lvar :handler_name))))))
                        (block
                          (send
                            (array
                              (sym :INT)
                              (sym :TERM)) :each)
                          (args
                            (procarg0 :sig))
                          (block
                            (send nil :trap
                              (lvar :sig))
                            (args)
                            (send nil :quit!
                              (lvar :server)
                              (lvar :handler_name))))
                        (if
                          (send
                            (lvar :server) :respond_to?
                            (sym :threaded=))
                          (send
                            (lvar :server) :threaded=
                            (send
                              (send nil :settings) :threaded)) nil)
                        (send nil :set
                          (sym :running)
                          (true))
                        (if
                          (send nil :block_given?)
                          (yield
                            (lvar :server)) nil))))
                  (resbody
                    (array
                      (const
                        (const nil :Errno) :EADDRINUSE)) nil
                    (send
                      (gvar :$stderr) :puts
                      (dstr
                        (str "== Someone is already performing on port ")
                        (begin
                          (send nil :port))
                        (str "!")))) nil))
              (def :prototype
                (args)
                (or-asgn
                  (ivasgn :@prototype)
                  (send nil :new)))
              (if
                (send nil :method_defined?
                  (sym :new!)) nil
                (alias
                  (sym :new!)
                  (sym :new)))
              (def :new
                (args
                  (restarg :args)
                  (blockarg :bk))
                (begin
                  (lvasgn :instance
                    (send nil :new!
                      (splat
                        (lvar :args))
                      (block-pass
                        (lvar :bk))))
                  (send
                    (const nil :Wrapper) :new
                    (send
                      (send nil :build
                        (lvar :instance)) :to_app)
                    (lvar :instance))))
              (def :build
                (args
                  (arg :app))
                (begin
                  (lvasgn :builder
                    (send
                      (const
                        (const nil :Rack) :Builder) :new))
                  (send nil :setup_default_middleware
                    (lvar :builder))
                  (send nil :setup_middleware
                    (lvar :builder))
                  (send
                    (lvar :builder) :run
                    (lvar :app))
                  (lvar :builder)))
              (def :call
                (args
                  (arg :env))
                (block
                  (send nil :synchronize)
                  (args)
                  (send
                    (send nil :prototype) :call
                    (lvar :env))))
              (send nil :private)
              (def :setup_default_middleware
                (args
                  (arg :builder))
                (begin
                  (send
                    (lvar :builder) :use
                    (const nil :ExtendedRack))
                  (if
                    (send nil :show_exceptions?)
                    (send
                      (lvar :builder) :use
                      (const nil :ShowExceptions)) nil)
                  (if
                    (send nil :method_override?)
                    (send
                      (lvar :builder) :use
                      (const
                        (const nil :Rack) :MethodOverride)) nil)
                  (send
                    (lvar :builder) :use
                    (const
                      (const nil :Rack) :Head))
                  (send nil :setup_logging
                    (lvar :builder))
                  (send nil :setup_sessions
                    (lvar :builder))
                  (send nil :setup_protection
                    (lvar :builder))))
              (def :setup_middleware
                (args
                  (arg :builder))
                (block
                  (send
                    (send nil :middleware) :each)
                  (args
                    (arg :c)
                    (arg :a)
                    (arg :b))
                  (send
                    (lvar :builder) :use
                    (lvar :c)
                    (splat
                      (lvar :a))
                    (block-pass
                      (lvar :b)))))
              (def :setup_logging
                (args
                  (arg :builder))
                (if
                  (send nil :logging?)
                  (begin
                    (send nil :setup_common_logger
                      (lvar :builder))
                    (send nil :setup_custom_logger
                      (lvar :builder)))
                  (if
                    (send
                      (send nil :logging) :==
                      (false))
                    (send nil :setup_null_logger
                      (lvar :builder)) nil)))
              (def :setup_null_logger
                (args
                  (arg :builder))
                (send
                  (lvar :builder) :use
                  (const
                    (const nil :Rack) :NullLogger)))
              (def :setup_common_logger
                (args
                  (arg :builder))
                (send
                  (lvar :builder) :use
                  (const
                    (const nil :Sinatra) :CommonLogger)))
              (def :setup_custom_logger
                (args
                  (arg :builder))
                (if
                  (send
                    (send nil :logging) :respond_to?
                    (sym :to_int))
                  (send
                    (lvar :builder) :use
                    (const
                      (const nil :Rack) :Logger)
                    (send nil :logging))
                  (send
                    (lvar :builder) :use
                    (const
                      (const nil :Rack) :Logger))))
              (def :setup_protection
                (args
                  (arg :builder))
                (begin
                  (if
                    (send nil :protection?) nil
                    (return))
                  (lvasgn :options
                    (if
                      (send
                        (const nil :Hash) :===
                        (send nil :protection))
                      (send
                        (send nil :protection) :dup)
                      (hash)))
                  (send
                    (lvar :options) :[]=
                    (sym :except)
                    (send nil :Array
                      (send
                        (lvar :options) :[]
                        (sym :except))))
                  (if
                    (send nil :sessions?) nil
                    (op-asgn
                      (send
                        (lvar :options) :[]
                        (sym :except)) :+
                      (array
                        (sym :session_hijacking)
                        (sym :remote_token))))
                  (or-asgn
                    (send
                      (lvar :options) :[]
                      (sym :reaction))
                    (sym :drop_session))
                  (send
                    (lvar :builder) :use
                    (const
                      (const nil :Rack) :Protection)
                    (lvar :options))))
              (def :setup_sessions
                (args
                  (arg :builder))
                (begin
                  (if
                    (send nil :sessions?) nil
                    (return))
                  (lvasgn :options
                    (hash))
                  (if
                    (send nil :session_secret?)
                    (send
                      (lvar :options) :[]=
                      (sym :secret)
                      (send nil :session_secret)) nil)
                  (if
                    (send
                      (send nil :sessions) :respond_to?
                      (sym :to_hash))
                    (send
                      (lvar :options) :merge!
                      (send
                        (send nil :sessions) :to_hash)) nil)
                  (send
                    (lvar :builder) :use
                    (const
                      (const
                        (const nil :Rack) :Session) :Cookie)
                    (lvar :options))))
              (def :detect_rack_handler
                (args)
                (begin
                  (lvasgn :servers
                    (send nil :Array
                      (send nil :server)))
                  (block
                    (send
                      (lvar :servers) :each)
                    (args
                      (procarg0 :server_name))
                    (kwbegin
                      (rescue
                        (return
                          (send
                            (const
                              (const nil :Rack) :Handler) :get
                            (send
                              (lvar :server_name) :to_s)))
                        (resbody
                          (array
                            (const nil :LoadError)
                            (const nil :NameError)) nil nil) nil)))
                  (send nil :fail
                    (dstr
                      (str "Server handler (")
                      (begin
                        (send
                          (lvar :servers) :join
                          (str ",")))
                      (str ") not found.")))))
              (def :inherited
                (args
                  (arg :subclass))
                (begin
                  (send
                    (lvar :subclass) :reset!)
                  (if
                    (send
                      (lvar :subclass) :app_file?) nil
                    (send
                      (lvar :subclass) :set
                      (sym :app_file)
                      (send
                        (send nil :caller_files) :first)))
                  (zsuper)))
              (cvasgn :@@mutex
                (send
                  (const nil :Mutex) :new))
              (def :synchronize
                (args
                  (blockarg :block))
                (if
                  (send nil :lock?)
                  (send
                    (cvar :@@mutex) :synchronize
                    (block-pass
                      (lvar :block)))
                  (yield)))
              (send nil :public)
              (casgn nil :CALLERS_TO_IGNORE
                (array
                  (regexp
                    (str "/sinatra(/(base|main|showexceptions))?\\.rb$")
                    (regopt))
                  (regexp
                    (str "lib/tilt.*\\.rb$")
                    (regopt))
                  (regexp
                    (str "^\\(.*\\)$")
                    (regopt))
                  (regexp
                    (str "rubygems/custom_require\\.rb$")
                    (regopt))
                  (regexp
                    (str "active_support")
                    (regopt))
                  (regexp
                    (str "bundler(/runtime)?\\.rb")
                    (regopt))
                  (regexp
                    (str "<internal:")
                    (regopt))
                  (regexp
                    (str "src/kernel/bootstrap/[A-Z]")
                    (regopt))))
              (if
                (defined?
                  (const nil :RUBY_IGNORE_CALLERS))
                (begin
                  (send nil :warn
                    (str "RUBY_IGNORE_CALLERS is deprecated and will no longer be supported by Sinatra 2.0"))
                  (send
                    (const nil :CALLERS_TO_IGNORE) :concat
                    (const nil :RUBY_IGNORE_CALLERS))) nil)
              (def :caller_files
                (args)
                (send
                  (send nil :cleaned_caller
                    (int 1)) :flatten))
              (def :caller_locations
                (args)
                (send nil :cleaned_caller
                  (int 2)))
              (send nil :private)
              (def :warn
                (args
                  (arg :message))
                (super
                  (send
                    (lvar :message) :+
                    (dstr
                      (str "\n\tfrom ")
                      (begin
                        (send
                          (send
                            (send nil :cleaned_caller) :first) :join
                          (str ":")))))))
              (def :cleaned_caller
                (args
                  (optarg :keep
                    (int 3)))
                (block
                  (send
                    (block
                      (send
                        (send nil :caller
                          (int 1)) :map)
                      (args
                        (procarg0 :line))
                      (send
                        (send
                          (lvar :line) :split
                          (regexp
                            (str ":(?=\\d|in )")
                            (regopt))
                          (int 3)) :[]
                        (int 0)
                        (lvar :keep))) :reject)
                  (args
                    (arg :file)
                    (restarg :_))
                  (block
                    (send
                      (const nil :CALLERS_TO_IGNORE) :any?)
                    (args
                      (procarg0 :pattern))
                    (send
                      (lvar :file) :=~
                      (lvar :pattern)))))))
          (def :force_encoding
            (args
              (restarg :args))
            (send
              (send nil :settings) :force_encoding
              (splat
                (lvar :args))))
          (if
            (defined?
              (const nil :Encoding))
            (defs
              (self) :force_encoding
              (args
                (arg :data)
                (optarg :encoding
                  (send nil :default_encoding)))
              (begin
                (if
                  (or
                    (send
                      (lvar :data) :==
                      (send nil :settings))
                    (send
                      (lvar :data) :is_a?
                      (const nil :Tempfile)))
                  (return) nil)
                (if
                  (send
                    (lvar :data) :respond_to?
                    (sym :force_encoding))
                  (send
                    (send
                      (lvar :data) :force_encoding
                      (lvar :encoding)) :encode!)
                  (if
                    (send
                      (lvar :data) :respond_to?
                      (sym :each_value))
                    (block
                      (send
                        (lvar :data) :each_value)
                      (args
                        (procarg0 :v))
                      (send nil :force_encoding
                        (lvar :v)
                        (lvar :encoding)))
                    (if
                      (send
                        (lvar :data) :respond_to?
                        (sym :each))
                      (block
                        (send
                          (lvar :data) :each)
                        (args
                          (procarg0 :v))
                        (send nil :force_encoding
                          (lvar :v)
                          (lvar :encoding))) nil)))
                (lvar :data)))
            (defs
              (self) :force_encoding
              (args
                (arg :data)
                (restarg))
              (lvar :data)))
          (send nil :reset!)
          (send nil :set
            (sym :environment)
            (send
              (begin
                (or
                  (send
                    (const nil :ENV) :[]
                    (str "RACK_ENV"))
                  (sym :development))) :to_sym))
          (send nil :set
            (sym :raise_errors)
            (block
              (send
                (const nil :Proc) :new)
              (args)
              (send nil :test?)))
          (send nil :set
            (sym :dump_errors)
            (block
              (send
                (const nil :Proc) :new)
              (args)
              (send
                (send nil :test?) :!)))
          (send nil :set
            (sym :show_exceptions)
            (block
              (send
                (const nil :Proc) :new)
              (args)
              (send nil :development?)))
          (send nil :set
            (sym :sessions)
            (false))
          (send nil :set
            (sym :logging)
            (false))
          (send nil :set
            (sym :protection)
            (true))
          (send nil :set
            (sym :method_override)
            (false))
          (send nil :set
            (sym :use_code)
            (false))
          (send nil :set
            (sym :default_encoding)
            (str "utf-8"))
          (send nil :set
            (sym :add_charset)
            (block
              (send
                (array
                  (str "javascript")
                  (str "xml")
                  (str "xhtml+xml")
                  (str "json")) :map)
              (args
                (procarg0 :t))
              (dstr
                (str "application/")
                (begin
                  (lvar :t)))))
          (send
            (send
              (send nil :settings) :add_charset) :<<
            (regexp
              (str "^text/")
              (regopt)))
          (kwbegin
            (rescue
              (begin
                (send nil :require
                  (str "securerandom"))
                (send nil :set
                  (sym :session_secret)
                  (send
                    (const nil :SecureRandom) :hex
                    (int 64))))
              (resbody
                (array
                  (const nil :LoadError)
                  (const nil :NotImplementedError)) nil
                (send nil :set
                  (sym :session_secret)
                  (send
                    (str "%064x") :%
                    (send
                      (const nil :Kernel) :rand
                      (send
                        (send
                          (int 2) :**
                          (int 256)) :-
                        (int 1)))))) nil))
          (sclass
            (self)
            (begin
              (send nil :alias_method
                (sym :methodoverride?)
                (sym :method_override?))
              (send nil :alias_method
                (sym :methodoverride=)
                (sym :method_override=))))
          (send nil :set
            (sym :run)
            (false))
          (send nil :set
            (sym :running)
            (false))
          (send nil :set
            (sym :server)
            (array
              (str "http")
              (str "webrick")))
          (send nil :set
            (sym :bind)
            (str "0.0.0.0"))
          (send nil :set
            (sym :port)
            (int 4567))
          (lvasgn :ruby_engine
            (and
              (defined?
                (const nil :RUBY_ENGINE))
              (const nil :RUBY_ENGINE)))
          (if
            (send
              (lvar :ruby_engine) :==
              (str "macruby"))
            (send
              (send nil :server) :unshift
              (str "controll_tower"))
            (begin
              (if
                (send
                  (lvar :ruby_engine) :nil?)
                (send
                  (send nil :server) :unshift
                  (str "mongrel")) nil)
              (if
                (send
                  (lvar :ruby_engine) :!=
                  (str "rbx"))
                (send
                  (send nil :server) :unshift
                  (str "puma")) nil)
              (if
                (send
                  (lvar :ruby_engine) :!=
                  (str "jruby"))
                (send
                  (send nil :server) :unshift
                  (str "thin")) nil)
              (if
                (send
                  (lvar :ruby_engine) :==
                  (str "rbx"))
                (send
                  (send nil :server) :unshift
                  (str "puma")) nil)
              (if
                (send
                  (lvar :ruby_engine) :==
                  (str "jruby"))
                (send
                  (send nil :server) :unshift
                  (str "trinidad")) nil)))
          (send nil :set
            (sym :absolute_redirects)
            (true))
          (send nil :set
            (sym :prefixed_redirects)
            (false))
          (send nil :set
            (sym :empty_path_info)
            (nil))
          (send nil :set
            (sym :app_file)
            (nil))
          (send nil :set
            (sym :root)
            (block
              (send
                (const nil :Proc) :new)
              (args)
              (and
                (send nil :app_file)
                (send
                  (const nil :File) :expand_path
                  (send
                    (const nil :File) :dirname
                    (send nil :app_file))))))
          (send nil :set
            (sym :views)
            (block
              (send
                (const nil :Proc) :new)
              (args)
              (and
                (send nil :root)
                (send
                  (const nil :File) :join
                  (send nil :root)
                  (str "views")))))
          (send nil :set
            (sym :reload_templates)
            (block
              (send
                (const nil :Proc) :new)
              (args)
              (send nil :development?)))
          (send nil :set
            (sym :lock)
            (false))
          (send nil :set
            (sym :threaded)
            (true))
          (send nil :set
            (sym :public_folder)
            (block
              (send
                (const nil :Proc) :new)
              (args)
              (and
                (send nil :root)
                (send
                  (const nil :File) :join
                  (send nil :root)
                  (str "public")))))
          (send nil :set
            (sym :static)
            (block
              (send
                (const nil :Proc) :new)
              (args)
              (and
                (send nil :public_folder)
                (send
                  (const nil :File) :exist?
                  (send nil :public_folder)))))
          (send nil :set
            (sym :static_cache_control)
            (false))
          (block
            (send nil :error
              (const
                (cbase) :Exception))
            (args)
            (begin
              (send
                (send nil :response) :status=
                (int 500))
              (send nil :content_type
                (str "text/html"))
              (str "<h1>Internal Server Error</h1>")))
          (block
            (send nil :configure
              (sym :development))
            (args)
            (begin
              (block
                (send nil :get
                  (str "/__sinatra__/:image.png"))
                (args)
                (begin
                  (lvasgn :filename
                    (send
                      (send
                        (const nil :File) :dirname
                        (--FILE--)) :+
                      (dstr
                        (str "/images/")
                        (begin
                          (send
                            (send nil :params) :[]
                            (sym :image)))
                        (str ".png"))))
                  (send nil :content_type
                    (sym :png))
                  (send nil :send_file
                    (lvar :filename))))
              (block
                (send nil :error
                  (const nil :NotFound))
                (args)
                (begin
                  (send nil :content_type
                    (str "text/html"))
                  (send
                    (begin
                      (dstr
                        (str "        <!DOCTYPE html>\n")
                        (str "        <html>\n")
                        (str "        <head>\n")
                        (str "          <style type=\"text/css\">\n")
                        (str "          body { text-align:center;font-family:helvetica,arial;font-size:22px;\n")
                        (str "            color:#888;margin:20px}\n")
                        (str "          #c {margin:0 auto;width:500px;text-align:left}\n")
                        (str "          </style>\n")
                        (str "        </head>\n")
                        (str "        <body>\n")
                        (str "          <h2>Sinatra doesn&rsquo;t know this ditty.</h2>\n")
                        (str "          <img src=\'")
                        (begin
                          (send nil :uri
                            (str "/__sinatra__/404.png")))
                        (str "\'>\n")
                        (str "          <div id=\"c\">\n")
                        (str "            Try this:\n")
                        (str "            <pre>")
                        (begin
                          (send
                            (send
                              (send nil :request) :request_method) :downcase))
                        (str " \'")
                        (begin
                          (send
                            (send nil :request) :path_info))
                        (str "\' do\n  \"Hello World\"\nend</pre>\n")
                        (str "          </div>\n")
                        (str "        </body>\n")
                        (str "        </html>\n"))) :gsub
                    (regexp
                      (str "^ {8}")
                      (regopt))
                    (str ""))))))))
      (class
        (const nil :Application)
        (const nil :Base)
        (begin
          (send nil :set
            (sym :logging)
            (block
              (send
                (const nil :Proc) :new)
              (args)
              (send
                (send nil :test?) :!)))
          (send nil :set
            (sym :method_override)
            (true))
          (send nil :set
            (sym :run)
            (block
              (send
                (const nil :Proc) :new)
              (args)
              (send
                (send nil :test?) :!)))
          (send nil :set
            (sym :session_secret)
            (block
              (send
                (const nil :Proc) :new)
              (args)
              (if
                (send nil :development?) nil
                (super))))
          (send nil :set
            (sym :app_file)
            (nil))
          (defs
            (self) :register
            (args
              (restarg :extensions)
              (blockarg :block))
            (begin
              (lvasgn :added_methods
                (send
                  (block
                    (send
                      (lvar :extensions) :map)
                    (args
                      (procarg0 :m))
                    (send
                      (lvar :m) :public_instance_methods)) :flatten))
              (send
                (const nil :Delegator) :delegate
                (splat
                  (lvar :added_methods)))
              (super
                (splat
                  (lvar :extensions))
                (block-pass
                  (lvar :block)))))))
      (module
        (const nil :Delegator)
        (begin
          (defs
            (self) :delegate
            (args
              (restarg :methods))
            (block
              (send
                (lvar :methods) :each)
              (args
                (procarg0 :method_name))
              (begin
                (block
                  (send nil :define_method
                    (lvar :method_name))
                  (args
                    (restarg :args)
                    (blockarg :block))
                  (begin
                    (if
                      (send nil :respond_to?
                        (lvar :method_name))
                      (return
                        (super
                          (splat
                            (lvar :args))
                          (block-pass
                            (lvar :block)))) nil)
                    (send
                      (send
                        (const nil :Delegator) :target) :send
                      (lvar :method_name)
                      (splat
                        (lvar :args))
                      (block-pass
                        (lvar :block)))))
                (send nil :private
                  (lvar :method_name)))))
          (send nil :delegate
            (sym :get)
            (sym :patch)
            (sym :put)
            (sym :post)
            (sym :delete)
            (sym :head)
            (sym :options)
            (sym :template)
            (sym :layout)
            (sym :before)
            (sym :after)
            (sym :error)
            (sym :not_found)
            (sym :configure)
            (sym :set)
            (sym :mime_type)
            (sym :enable)
            (sym :disable)
            (sym :use)
            (sym :development?)
            (sym :test?)
            (sym :production?)
            (sym :helpers)
            (sym :settings))
          (sclass
            (self)
            (send nil :attr_accessor
              (sym :target)))
          (send
            (self) :target=
            (const nil :Application))))
      (class
        (const nil :Wrapper) nil
        (begin
          (def :initialize
            (args
              (arg :stack)
              (arg :instance))
            (masgn
              (mlhs
                (ivasgn :@stack)
                (ivasgn :@instance))
              (array
                (lvar :stack)
                (lvar :instance))))
          (def :settings
            (args)
            (send
              (ivar :@instance) :settings))
          (def :helpers
            (args)
            (ivar :@instance))
          (def :call
            (args
              (arg :env))
            (send
              (ivar :@stack) :call
              (lvar :env)))
          (def :inspect
            (args)
            (dstr
              (str "#<")
              (begin
                (send
                  (ivar :@instance) :class))
              (str " app_file=")
              (begin
                (send
                  (send
                    (send nil :settings) :app_file) :inspect))
              (str ">")))))
      (defs
        (self) :new
        (args
          (optarg :base
            (const nil :Base))
          (optarg :options
            (hash))
          (blockarg :block))
        (begin
          (lvasgn :base
            (send
              (const nil :Class) :new
              (lvar :base)))
          (if
            (send nil :block_given?)
            (send
              (lvar :base) :class_eval
              (block-pass
                (lvar :block))) nil)
          (lvar :base)))
      (defs
        (self) :register
        (args
          (restarg :extensions)
          (blockarg :block))
        (send
          (send
            (const nil :Delegator) :target) :register
          (splat
            (lvar :extensions))
          (block-pass
            (lvar :block))))
      (defs
        (self) :helpers
        (args
          (restarg :extensions)
          (blockarg :block))
        (send
          (send
            (const nil :Delegator) :target) :helpers
          (splat
            (lvar :extensions))
          (block-pass
            (lvar :block))))
      (defs
        (self) :use
        (args
          (restarg :args)
          (blockarg :block))
        (send
          (send
            (const nil :Delegator) :target) :use
          (splat
            (lvar :args))
          (block-pass
            (lvar :block)))))))