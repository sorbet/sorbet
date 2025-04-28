# typed: strict

Open3.popen3("bundle", "install", { chdir: 'my_directory' })
Open3.popen3("bundle", { chdir: 'my_directory' })
Open3.popen3({ "VAR" => "true" }, "bundle", "install", { chdir: 'my_directory' })
Open3.popen3({ "VAR" => "true" }, "bundle", { chdir: 'my_directory' })

Open3.popen3("bundle", "install", { chdir: 'my_directory' }) do |stdin, stdout, stderr, wait_thread|
end

Open3.popen3({ "VAR" => "true" }, "bundle", "install", { chdir: 'my_directory' }) do |stdin, stdout, stderr, wait_thread|
end
