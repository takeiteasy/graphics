#!/usr/bin/env ruby

if $*.empty?
  puts "\n\tquick_gen.rb [header files...]\n"
  exit 1
end

vars = [*('a'..'z')]
$*.each do |f|
  next unless File.file? f and f[-2..-1] == '.h'
  `ctags -x --c-kinds=fp --sort=no #{f}`.split("\n").each do |p|
    /^(?<name>[a-zA-Z0-9_-]+)\s+prototype\s+\d+\s+\S+\s+\s+\*?(?<function>.*);$/ =~ p
    puts """#{function} {
}

"""
  end
end
