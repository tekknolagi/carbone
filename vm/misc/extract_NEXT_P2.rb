in_pp_if_threading_scheme = false
File.open("../engine.c", "r").each_line {|l|
  if l =~ /#if THREADING_SCHEME==/ 
    in_pp_if_threading_scheme = true
  end

  if in_pp_if_threading_scheme
    puts l if l =~ /#\s*(if|endif|define\s+NEXT_P(1|2))/
  end
    
  if l =~ /#endif/
    in_pp_if_threading_scheme = false
  end
}
