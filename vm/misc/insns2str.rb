
# hmmm, vmgen is not yet supporting reflective VM's

insns = []
File.new("../gen/rb-labels.i").each_line{ |line|
  insns << (line =~ / \( .* \)&&I_([^,]+), /x; $1) }

File.open("../gen/rb-desc.i", "w") {|f|
  f.write(insns.map { |i| " \"#{i}\", \n"})
}
