# stop somewhere after init
b obj_init

run -e ""

set $i = 0
while $i < number_of_insns
  p "- - - - - - - - - - - - - - - - - - - - - - - - - -"
  p ((char[])(insns_as_strings))+$i*200
  p { $i, \
      ((int*)vm_prim)[$i] - (int)engine, \
      ((int*)vm_prim)[$i+1] - ((int*)vm_prim)[$i] }
  # p ((char)insns_as_strings)[$i*200]
  # x ((int*)vm_prim)[0] ((int*)vm_prim)[1]
  disas ((int*)vm_prim)[$i] ((int*)vm_prim)[$i+1]
  set $i = $i + 1
end
