#
#		getopts.rb - get options
#			$Release Version: $
#			$Revision: 1.2 $
#			$Date: 1994/02/15 05:17:15 $
#			by Yasuo OHBA(STAFS Development Room)
#
# --
# �I�v�V�����̉�͂���, $OPT_?? �ɒl���Z�b�g���܂�. 
# �w��̂Ȃ��I�v�V�������w�肳�ꂽ���� nil ��Ԃ��܂�.
# ����I�������ꍇ��, �Z�b�g���ꂽ�I�v�V�����̐���Ԃ��܂�. 
#
#    getopts(single_opts, *opts)
#
#	ex. sample [options] filename
#	    options ...
#		-f -x --version --geometry 100x200 -d unix:0.0
#			    ��
#	getopts("fx", "version", "geometry:", "d:")
#
#    ������: 
#	-f �� -x (= -fx) �̗l�Ȉꕶ���̃I�v�V�����̎w������܂�. 
#	�����ň������Ȃ��Ƃ��� nil �̎w�肪�K�v�ł�. 
#    �������ȍ~:
#	�����O�l�[���̃I�v�V������, �����̔����I�v�V�����̎w������܂�. 
#	--version ��, --geometry 300x400 ��, -d host:0.0 ���ł�. 
#	�����𔺂��w��� ":" ��K���t���Ă�������. 
#
#    �I�v�V�����̎w�肪�������ꍇ, �ϐ� $OPT_?? �� non-nil ��������, ���̃I
#    �v�V�����̈������Z�b�g����܂�. 
#	-f -> $OPT_f = TRUE
#	--geometry 300x400 -> $OPT_geometry = 300x400
#
#    - �������� -- ��, ����ȍ~, �S�ăI�v�V�����̉�͂����܂���. 
#

$RCS_ID="$Header: /var/ohba/RCS/getopts.rb,v 1.2 1994/02/15 05:17:15 ohba Exp ohba $"

def getopts(single_opts, *opts)
  if (opts)
    single_colon = ""
    long_opts = []
    sc = 0
    for option in opts
      if (option.length <= 2)
	single_colon[sc, 0] = option[0, 1]
	sc += 1
      else
	long_opts.push(option)
      end
    end
  end

  opts = {}
  count = 0
  while ($ARGV.length != 0)
    compare = nil
    case $ARGV[0]
    when /^--?$/
      $ARGV.shift
      break
    when /^--.*/
      compare = $ARGV[0][2, ($ARGV[0].length - 2)]
      if (long_opts != "")
        for option in long_opts
          if (option[(option.length - 1), 1] == ":" &&
              option[0, (option.length - 1)] == compare)
            if ($ARGV.length <= 1)
	      return nil
            end
            eval("$OPT_" + compare + " = " + '$ARGV[1]')
	    opts[compare] =  TRUE
            $ARGV.shift
	    count += 1
	    break
          elsif (option == compare)
            eval("$OPT_" + compare + " = TRUE")
	    opts[compare] =  TRUE
	    count += 1
            break
          end
        end
      end
    when /^-.*/
      for index in 1..($ARGV[0].length - 1)
	compare = $ARGV[0][index, 1]
	if (single_opts && compare =~ "[" + single_opts + "]")
	  eval("$OPT_" + compare + " = TRUE")
	  opts[compare] =  TRUE
	  count += 1
	elsif (single_colon != "" && compare =~ "[" + single_colon + "]")
	  if ($ARGV[0][index..-1].length > 1)
	    eval("$OPT_" + compare + " = " + '$ARGV[0][(index + 1)..-1]')
	    opts[compare] =  TRUE
	    count += 1
	  elsif ($ARGV.length <= 1)
	    return nil
	  else
	    eval("$OPT_" + compare + " = " + '$ARGV[1]')
	    opts[compare] =  TRUE
	    $ARGV.shift
	    count = count + 1
	  end
	  break
	end
      end
    else
      break
    end

    $ARGV.shift
    if (!opts.includes(compare))
      return nil
    end
  end
  return count
end
