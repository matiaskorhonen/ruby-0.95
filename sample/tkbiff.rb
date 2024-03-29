#! /usr/local/bin/ruby

if $ARGV.length == 0
  if ENV['MAIL']
    $spool = ENV['MAIL']
  else  
    $spool = '/usr/spool/mail/' + ENV['USER']
  end
else 
  $spool = $ARGV[0]
end

exit if fork
  
require "parsedate"
require "base64"

include ParseDate

class Mail
  def Mail.new(f)
    if !f.is_kind_of?(IO)
      f = open(f, "r")
      me = super
      f.close
    else
      me = super
    end
    return me
  end

  def initialize(f)
    @header = {}
    @body = []
    while f.gets()
      $_.chop!
      continue if /^From /	# skip From-line  
      break if /^$/		# end of header
      if /^(\S+):\s*(.*)/
	@header[attr = $1.capitalize] = $2
      elsif attr
	sub(/^\s*/, '')
	@header[attr] += "\n" + $_
      end
    end

    return if ! $_

    while f.gets()
      break if /^From /
      @body.push($_)
    end
  end

  def header
    return @header
  end

  def body
    return @body
  end

end

require "tkscrollbox"

$top = TkRoot.new
$top.withdraw
$list = TkScrollbox.new($top) {
  relief 'raised'
  width 80
  height 8
  setgrid 'yes'
  pack
}
TkButton.new($top) {
  text 'Dismiss'
  command proc {$top.withdraw}
  pack('fill'=>'both','expand'=>'yes')
}
$top.bind "Control-c", proc{exit}
$top.bind "Control-q", proc{exit}
$top.bind "space", proc{exit}

$spool_size = 0
def check
  size = File.size($spool)
  if size and size != $spool_size
    pop_up if size > 0
  end
  Tk.after 5000, proc{check}
end

def pop_up
  outcount = 0;
  $spool_size = File.size($spool)
  $list.delete 0, 'end'
  f = open($spool, "r")
  while !f.eof
    mail = Mail.new(f)
    date, from, subj =  mail.header['Date'], mail.header['From'], mail.header['Subject']
    continue if !date
    y = m = d = 0
    y, m, d = parsedate(date) if date
    from = "sombody@somewhere" if ! from
    subj = "(nil)" if ! subj
    from = decode_b(from)
    subj = decode_b(subj)
    $list.insert 'end', format('%-02d/%02d/%02d [%-28.28s] %s',y,m,d,from,subj)
    outcount += 1
  end
  f.close
  if outcount == 0
    $list.insert 'end', "You have no mail."
  end
  $top.deiconify
  Tk.after 2000, proc{$top.withdraw}
end

check
Tk.mainloop
