BASE:=..
DEST:=./html
PRJ:=Carbone Ruby VM

TEXTS:=README COPYING THANKS FEATURES TODO ChangeLog \
	doc/guide.txt doc/generator.txt doc/input_language.txt \
                 doc/gc.txt doc/proposals.txt

srcfile = $(BASE)/$(src)
target  = $(DEST)/$(subst /,__,$(src)).html

all:
	mkdir $(DEST) || true
	rm -f $(DEST)/*.html
	make all-do

all-do: $(TEXTS)
	cp $(DEST)/README.html $(DEST)/index.html


$(TEXTS):
	make page src=$@


html_head = <html><head><title>$(PRJ) -- $(src) </title> \
               <link REL="stylesheet" HREF="a.css"> </head> \
               <body>

html_foot =      <br> \
             </body></html>

d :=  <font size="-2"> $(shell date '+%Y%m%d_%H%M')</font>

nav_beg = <table><tr><td valign="top">
nav_end = </td>

cnt_beg = <td><pre>
cnt_end = </pre></td></tr></table>

E = echo >>$(target)
C = ruby -pe "gsub(/</,'&lt;'); gsub(/>/,'&gt;')" >>$(target)

page:
	echo $(src) '==>' $(target) >&2
	$(E) '$(html_head)'
	$(E) '$(nav_beg)'
	$(E) '<font type="typewriter"><font size="+1">Carbone Ruby VM</font><br>' \
               $(foreach x, $(TEXTS), \
                 '<a href="$(subst /,__,$(x)).html">$(x)</a><br>') '$(d)</font>'
	$(E) '$(nav_end)'
	$(E) '$(cnt_beg)'
	cat   $(srcfile) | $(C)
	$(E) '$(cnt_end)'
	$(E) '$(html_foot)'

linkto__sep%.html:
	$(c) "<hr>"

linkto_%.html:
	$(c) "<a href=\"$@\">$@</a>"

#www.ruby.org
#www.complang.tuwien.ac.at/anton/vmgen








#pic2plot -T ps process.pic>process.ps

#all: opt=-scale 150% 
all_png:\
	outer_contexts.png \
	process.png \
	vars_lvars.png \
	vars_svars.png


%.ps: %.pic
	pic2plot -T ps $^ >$@ $(flags)

see:
	make $(f).ps
	ggv $(f).ps 

%.png: size ?= -r110
%.png: %.pic
	pic2plot -T ps $^ \
          | gs -dNOPLATFONTS -sDEVICE=png256 -sOutputFile=$@ $(size) -

#%.png: %.ps
#	convert $(opt) $^ $@

s: $(f).png
	eeyes $(f).png


