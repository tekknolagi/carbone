
$sends = 0
$prims = 0

def dbg(x) x if $DEBUG end   ## control me with ruby -d ...
fib_code = "
class Fixnum
  def fib()
    #{dbg "$prims += 1"}
    if self>1 then
      #{dbg "$prims += 3; $sends += 2"}
      (self-1).fib + (self-2).fib + 1
    else
      1
    end
  end
end
"

puts fib_code

eval fib_code

puts "fib 32 = #{ 32.fib }"
puts "sends: #{$sends}   prims: #{$prims}"
