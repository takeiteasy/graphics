#!/usr/bin/env ruby

maj, min, rev = File.readlines("VERSION")[0].split '.'
rev = rev[0..-2] if rev[-1] == '\n'

def modify(a, b)
  case b.downcase
  when /\d+/
    b
  when /incr?/
    (a.to_i + 1).to_s
  when /decr?/
    (a.to_i - 1).to_s
  else
    a
  end
end

Hash[$*.flat_map{|s| s.scan(/--?([^=\s]+)(?:=(\S+))?/)}].each do |k, v|
  case k.downcase
  when 'maj'
    maj = modify maj, v
  when 'min'
    min = modify min, v
  when 'rev'
    rev = modify rev, v
  end
end

File.open("VERSION", "w") do |f|
  f.write "#{maj}.#{min}.#{rev}"
end
