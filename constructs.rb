class A; def initialize(&block); instance_eval(&block) end end
    ==>nil
o = A.new { @a = 999; @b = 123; @w = 9999 }
    ==>#<A:0x4028461c @a=999, @w=9999, @b=123>
----
a = 9;
(proc { a })[]

----
loop { c = 8; break }; c
NameError: undefined local variable or method `c' for #<Object:0x4026ace0>
        from (irb):1
----
proc { c = 9 }; c
NameError: undefined local variable or method `c' for #<Object:0x4026ace0>
        from (irb):2
----

----
def u(ary)
  if ary.length > 2
    first,*nary = ary
    u(nary)
  else
    proc { ary }
  end
end

--
u([6,5,4,3])[]
    ==>[4, 3]

----
# proc creation:
#  * replace known local-variables with their reference.
#  * ivar: replaced by offset to the ivar in ivar-table + reference to self
#  * const.: 

class St; attr_accessor :st; end
def a(lv); r=St.new; r.st=proc{lv}; r end
y=[1,2,3,4]
m = a(y)
    ==>#<St:0x40282aec @st=#<Proc:0x40282ad8>>
m.st[]
    ==>[1, 2, 3, 4]
y
    ==>[1, 2, 3, 4]
m.st[].id
    ==>538191946
y.id
    ==>538191946
y << 1 
    ==>[1, 2, 3, 4, 1]
y << 99
    ==>[1, 2, 3, 4, 1, 99]
m.st[]   
    ==>[1, 2, 3, 4, 1, 99]
----
class A
  a = 9;
  def t
    a
  end
end
--
A.new.t
==>
NameError: undefined local variable or method `a' for #<A:0x40287880>
        from (irb):5:in `t'
        from (irb):8
----
a = 9; class A; b = a; end
NameError: undefined local variable or method `a' for A:Class
        from (irb):2
----
class A; a = 9; Q = proc { b = a; } end
A::Q[]
    ==>9
----
class A
  Q = 9;
  def t;    Q  end
  def A.tt; Q  end
end
class B < A
  def bt; t; end
  def nt; Q; end
end

--
A.new.t
    ==>9
A.tt
    ==>9
B.new.bt
    ==>9
B.new.nt
    ==>9

----
class A; end
class B < A
  def bset; @iv = "B"; end
  def bget; @iv; end
end

p = B.new
p.bset
p.bget

class A
  def aset; @iv = "A"; end
  def aget; @iv; end
end

p.aget

----

a = 9; (proc { a = 0 })[]
    ==>0
--
a
    ==>0

----

a = 9; b = ((proc{ proc{ a<<99 }}))[]
    ==>#<Proc:0x4026e060>
a = [1,2,3]
    ==>[1, 2, 3]
a
    ==>[1, 2, 3]
b
    ==>#<Proc:0x4026e060>
b[]
    ==>[1, 2, 3, 99]
a
    ==>[1, 2, 3, 99]
-----

class A; def op; proc {|x| @a = x}; end; attr_accessor :a; end
    ==>nil
a = A.new
    ==>#<A:0x40297208>
a.a = 999
    ==>999
a.op[333]
    ==>333
a.a
    ==>333
----
class A; a = 99; def A.s(x); a=x; end;   def A.g; a; end end
    ==>nil
A.s(100)
    ==>100
A.g
NameError: undefined local variable or method `a' for A:Class
        from (irb):10:in `g'
        from (irb):12
----
class Z; a = 99; R=proc{a}; W=proc{|x|a=x}; end
    ==>#<Proc:0x40244338>
Z::R[]
    ==>99
Z::R[]
    ==>99
Z::W[109]
    ==>109
Z::R[]
    ==>109
----
  f = proc{|a| f.call(a-1) }  # endless


-------------------------- modules
module Garden; end
module Sea; end

module GS; include Garden; include Sea; end
    ==>GS
module SG; include Sea; include Garden end
    ==>SG

GS.ancestors  
    ==>[GS, Sea, Garden]
SG.ancestors
    ==>[SG, Garden, Sea]

module GS_SG; include GS; include SG; end
    ==>GS_SG
module SG_GS; include SG; include GS end
    ==>SG_GS

GS_SG.ancestors
    ==>[GS_SG, SG, GS, Sea, Garden]
SG_GS.ancestors
    ==>[SG_GS, GS, SG, Garden, Sea]

------------------------ class context as proc source
class A
  a,b,c=1,2,3
  $r=proc{[a,b,c]}
  c=[9,8,7]
end
    ==>[9, 8, 7]
$r
    ==>#<Proc:0x402882a4>
$r[]
    ==>[1, 2, [9, 8, 7]]

----
class B  
  a = 1
  $a = proc { [a, a=2] }
  $b = proc { [a, a=3] }
end
    ==>#<Proc:0x4027f028>
$a[]
    ==>[1, 2]
$a[]
    ==>[2, 2]
$b[]
    ==>[2, 3]
$b[]
    ==>[3, 3]
$a[]
    ==>[3, 2]

---------------------------------------
# <markus> and if a closure is created and somebody wants to assign to
#  the parameter.... :-/ 
# <markus> tons of problems
# <matju> have you considered the problem that even though parameters
#  are received in variables that may get assigned to, doing super
#  without arguments will pick up the _original_ passed parameters ? 

class A; def m(*p); p end end 
    ==>nil
class B<A; def m(a,b) proc{[ [a,b],   super, super(a,b),\
                             [a,b=9], super, super(a,b)  ]} end end
    ==>nil
o = B.new
o.m(1,2)[]
    ==>[[1, 2], [1, 2], [1, 2], [1, 9], [1, 2], [1, 9]]


# The compiler knows whether arg-preserving-super is used.
# The compiler knows whether args get assigned new values.
# He can decide
#   * whether to preserve original args
#   * which args to preserve
# 
# The VM has to offer two different super's for the compiler:
#   * super(*explicite_arguments)
#   * arg-preserving-super

-------------------------

# closure saves super-context:

class A; def g; "rtzrtz"; end end
    ==>nil
class B<A; def g; proc{super} end end
    ==>nil
B.new.g[]
    ==>"rtzrtz"
