# TODO:     ['configuration.rb','configure.rb'].to_autoconf

$vm_version = "0.1"

$options ||= {}

#DEBUG = false
DEBUG = true

$options[:has_gcc_3_0] = false        # we may use some gcc-3.0 options
$options[:cc] = 'gcc '

$options[:garbage_collected] = true   # say no here if you dont like Boehm GC
$options[:gc_incremental] = false

$options[:engine_debug] = true        # link-in debug build of engine?
                                      # (neccesary for tracing and profiling)

# adjust your path to metaruby
# (lgram/Marshal.rb 1.13  Mon Oct  1 01:07:25 2001 is working fine)
$options[:metaruby] = "~/dev/ruby/cvsroot/lib/metaruby"

$options[:method_lookup_cache] = :hash       # other option is :none
$options[:vmgen_spTOS] = true                # 'top of stack' - caching

$options[:link_static] = false
