#! /usr/local/bin/ruby

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

$ARGV[0] = '/usr/spool/mail/' + ENV['USER'] if $ARGV.length == 0

require "tk"
list = scroll = nil
TkFrame.new{|f|
  list = TkListbox.new(f) {
    yscroll proc{|idx|
	scroll.set *idx
    }
    relief 'raised'
#    geometry "80x5"
    width 80
    height 5
    setgrid 'yes'
    pack('side'=>'left','fill'=>'both','expand'=>'yes')
  }
  scroll = TkScrollbar.new(f) {
    command proc{|idx|
      list.yview *idx
    }
    pack('side'=>'right','fill'=>'y')
  }
  pack
}
root = Tk.root
TkButton.new(root) {
  text 'Dismiss'
  command proc {exit}
  pack('fill'=>'both','expand'=>'yes')
}
root.bind "Control-c", proc{exit}
root.bind "Control-q", proc{exit}
root.bind "space", proc{exit}

$outcount = 0;
for file in $ARGV
  continue if !File.exists?(file)
  f = open(file, "r")
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
    list.insert 'end', format('%-02d/%02d/%02d [%-28.28s] %s',y,m,d,from,subj)
    $outcount += 1
  end
  f.close
end

limit = 10000
if $outcount == 0
  list.insert 'end', "You have no mail."
  limit = 2000
end
Tk.after limit, proc{
  exit
}
Tk.mainloop
