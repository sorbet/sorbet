def basename(p):
  return p.rpartition("/")[-1]

def dirname(p):
  dirname, sep, fname = p.rpartition("/")
  if dirname:
    return dirname.rstrip("/")
  else:
    return sep

def dropExtension(p):
  "TODO: handle multiple . in name"
  return p.partition(".")[0]

def pipeline_tests(all_paths, filter):
    tests = {} # test_name-> {"path": String, "prefix": String, "sentinel": String}
    for path in all_paths:
      if "disabled" not in path:
        if path.endswith(".rb"):
          prefix = dropExtension(basename(path).partition("__")[0])
          test_name = dirname(path) + "/" + prefix
          #test_name = test_name.replace("/", "_")
          current = tests.get(test_name)
          if None == current:
            data = {"path": dirname(path), "prefix": prefix, "sentinel": path}
            tests[test_name] = data

    for name in tests.keys():
      path = tests[name]["path"]
      prefix = tests[name]["prefix"]
      sentinel = tests[name]["sentinel"]
      native.sh_test(
          name = "test_{}/{}".format(filter, name),
          srcs = ["test_corpus_forwarder.sh"],
          args = ["--single_test=$(location {})".format(sentinel), "--gtest_filter={}/*".format(filter)],
          data = native.glob([
                             "{}/{}*".format(path, prefix),
                         ]) + ["test_corpus_sharded"],
          size = 'small',
      )
    return ["test_{}/{}".format(filter, test) for test in tests]
