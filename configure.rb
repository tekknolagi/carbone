$options = {}

load 'configuration.rb'





# ---------------------------------------------
# (don't change this file to configure carbone)

class NilClass; def to_str; ""; end end


$options[:debug_mallocs] = DEBUG
$options[:verbose_operation] = DEBUG

if not DEBUG
  $options[:compile_opts] =
    "-Wall -O3 -fomit-frame-pointer " + 
    (`uname -mp` =~ /i686/ ? " -mcpu=i686 -march=i686" : "") +
    " -DNDEBUG"

  $options[:engine_extra] = " -fno-cse-follow-jumps  -fno-cse-skip-blocks"

  if $options[:has_gcc_3_0]
    $options[:engine_extra] += " -fno-guess-branch-probability "
  end
else
  $options[:compile_opts] = "-g "
  $options[:engine_extra] = ""
end

$options[:warnings] =
  (if $options[:has_gcc_3_0] then " -Wdisabled-optimization " else "" end)

($options[:compile_opts] += " -static ";
 $options[:other_ld] = " -static ") if $options[:link_static]

puts ""
$options.each_pair{|*o| puts "%25s => %s" % o}
puts ""

# ++++++++++++++++++++++++++++++++++++++++++++++++++++++ #
# Makefile.conf                                          #
# ++++++++++++++++++++++++++++++++++++++++++++++++++++++ #
File.open("Makefile.conf", "w"){|c|c.puts(%[

CC=#{$options[ :cc ]}
GCC_CONF_OPT=#{$options[ :compile_opts ]}
VM_ENGINE_O_EXTRA_OPT=#{$options[ :engine_extra ]}
LDFLAGS_GC=#{'-lgc' if $options[ :garbage_collected ]}
LDFLAGS_OTHER=#{$options[ :other_ld ]}

CC += -Wall
# CC += -Wstrict-prototypes -Winline
# CC += #{$options[ :warnings ]}

ENGINE_DEBUG = #{$options[:engine_debug]}

])}

# -Wunreachable-code


# ++++++++++++++++++++++++++++++++++++++++++++++++++++++ #
# configuration.h                                        #
# ++++++++++++++++++++++++++++++++++++++++++++++++++++++ #
File.open("configuration.h", "w"){
  | c |
  c.puts( <<END

#define VM_VERSION "#{ $vm_version }"


// #define LGRAM_DEBUG_PARSER

#{"// fond/allocate.h:
   // check off-road memory accesses
   #define DEBOMB" if $options[:debug_mallocs]}


#define COMPILED_WITH_FLAGS "CC=#{ $options[:cc] + ' ' +
                                   $options[:compile_opts] + ' ' +
                                   $options[:other_ld] }"

#define ENGINE_COMPILED_WITH_FLAGS "CC=#{ $options[:cc] + ' ' +
                                          $options[:compile_opts] + ' ' +
                                          $options[:engine_extra]}"

#{"#define DBG_SHOW_CLASS_SIZE
   #define DBG_SHOW_METH_LOOKUP_PATH
   #define DBG_SHOW_PARAM_DUP" if $options[:verbose_operation]}

#{"#define NO_GARBAGE_COLLECTOR" if not $options[:garbage_collected]}

#{"#define GC_INCREMENTAL" if $options[:gc_incremental]}

#{"#define ENGINE_DEBUG"  if $options[:engine_debug]}


// ------------ for comp/generator.c
// the maximal number of svars that are compiled as a sequence of
// lit; methods with more svars will use allot to allocate stack
// space  (minor optimization)
#define MAX_NUM_SVARS_COMP_AS_LIT 20


// ------------ for vm/cmcache.c
// caching strategy; not yet well factored to implement an other
// CS, so can only be switched off, currently
#{"#define conf_cmcache 0"  if $options[:method_lookup_cache] == :none}
#{"#define conf_cmcache 3"  if $options[:method_lookup_cache] == :hash}


// ------------ for vm/engine.c
#{ if $options[:vmgen_spTOS] then "#define USE_spTOS 1"
                             else "#undef USE_spTOS" end }

END
)}
