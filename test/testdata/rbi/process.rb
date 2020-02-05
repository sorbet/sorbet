# typed: strict

pid = Process.spawn('ls -al')
Process.wait pid
