# I know, this is pretty much a hack...
# at least ...  I'm open to suggestions...
# markus
$methods = []
$exceptions = []
class MethodDecl < Struct.new(:class_name, :method_name, :parameters)
  def prim_name
    self.class_name.to_s + trans2c_ident(self.method_name)
  end
end


def compile2c(decl, ctx=nil)
  case decl
    when Symbol, Numeric
    printf("#{decl.to_s}")
  else
    case decl[0]
    when :class
      class_name = decl[1]
      decl[2..-1].each {
	|meth|
	method_name = meth[0]
	parameters  = meth[1][0]
	code        = meth[2]
	$methods << meth_decl = MethodDecl.new(class_name, method_name, parameters)
	
	prim_name = meth_decl.prim_name
	paramsj = parameters.join(' ')

	$stderr.puts " %20s =>  %s" % [
	  "%s#%s"%[class_name,method_name],
	  prim_name]

	print \
%Q*
#{prim_name} ( #{paramsj} rcv i_ca inpar retip oldfp -- #{paramsj} ires ) 
  if (inpar != #{parameters.size}) {
    meth_report_wrong_num_parameters(rcv, "#{method_name}", inpar);
    return(rb_undef);
  }
*
	compile2c(code, meth_decl)
	puts ""
      }
      
    when "if".intern
      cond_then_pairs = decl[1]
      first = true
      cond_then_pairs.each {|a,*b|
	if first then printf("  if ("); first = false
	else printf(" else if ("); end
	compile2c(a, ctx)
	puts(")")
	b.each{|bdy| compile2c(bdy, ctx)}
      }
      _else = decl[2]
      if _else
	puts "  else "
	compile2c(_else, ctx)
      end
    when :eql, :+, :-, :*, "/".intern, :<,
	:>, :<=, :>=,
	"&&".intern, "||".intern, :&, :|,
	:<<, :>>
      compile2c(decl[1])
      op = ({ :eql => "==" }[decl[0]]) || decl[0]
      printf " #{ op } "
      compile2c(decl[2])
      
    when :kind_of?
      printf "i_is_a("
      compile2c(decl[1])
      printf ","
      compile2c(decl[2])
      printf ")"
      
    when :let
      bindings = decl[1]
      body = decl[2..-1]
      puts "  {"
      bindings.each{|b|
	type, var_name, code = *b
	printf "  #{type} #{var_name} = "
	compile2c(code)
	puts " ; "
      }

      body.each{|b| compile2c(b, ctx) }
      
      puts "  }"

    when :value2long, :long2value, :in_imm_range
      printf("#{decl[0]}( #{decl[1]} )")
      
    when :return
      printf ("    ires = ")
      compile2c(decl[1], ctx)
      puts (";")
      #puts ("  TAIL;")

    when :error
      printf(
         %Q* error_#{decl[1]}("#{ctx.class_name}","#{ctx.method_name}", \
             rcv, other, inpar); *)

    when :true, :false
      printf( "rb_#{ decl[0].to_s }" )

    when :handle
      printf( "#{decl[1]}(sym_#{ trans2c_ident(ctx.method_name).downcase }, " +
              " rcv, other)");

    else
      puts " ?? " + decl.join(" --- ")
    end
    
    
  end
end

def trans2c_ident(x)
  {
    :+           => 'Plus',
    :-           => 'Minus',
    :*           => 'Mul',
    "/".intern   => 'Div',
    :<           => 'Lt',
    :>           => 'Gt',
    :<=>         => 'Cmp',
    '@+'.intern  => 'UPlus',
    '@-'.intern  => 'UMinus',
    :[]          => 'Slc',
    :[]=         => 'SlcAsgn',
  }[x]
end


load "../configuration.rb"
$:<< $options[:metaruby]
require "lgram/Marshal"
lgr = LGReader.new

file_ = ARGV[0]
if not file_ then puts "give file-root"; exit end

cont = lgr.input_file(File.new(file_ + ".decl"))[0]

compile2c(cont)
puts ""
puts ""


File.open(file_ + "_bi_decl.i", "w") {
  |f|
  $methods.each {
    |m|
    f.write \
            \
%Q*meth_from_builtin(\
    class_#{ m.class_name.to_s.downcase },\
    "#{ m.method_name.to_s }",\
    "#{ m.prim_name }");
*

   }
 }
