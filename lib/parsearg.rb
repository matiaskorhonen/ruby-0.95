#
#		parseargs.rb - parse arguments
#			$Release Version: $
#			$Revision: 1.3 $
#			$Date: 1994/02/15 05:16:21 $
#			by Yasuo OHBA(STAFS Development Room)
#
# --
# �����̉�͂���, $OPT_?? �ɒl���Z�b�g���܂�. 
# ����I�������ꍇ��, �Z�b�g���ꂽ�I�v�V�����̐���Ԃ��܂�. 
#
#    parseArgs(argc, single_opts, *opts)
#
#	ex. sample [options] filename
#	    options ...
#		-f -x --version --geometry 100x200 -d unix:0.0
#			    ��
#	parseArgs(1, nil, "fx", "version", "geometry:", "d:")
#
#    ������: 
#	�I�v�V�����ȊO�̍Œ�����̐�
#    ������: 
#	�I�v�V�����̕K�v���c�K���K�v�Ȃ� %TRUE �����łȂ���� %FALSE.
#    ��O����: 
#	-f �� -x (= -fx) �̗l�Ȉꕶ���̃I�v�V�����̎w������܂�. 
#	�����ň������Ȃ��Ƃ��� nil �̎w�肪�K�v�ł�. 
#    ��l�����ȍ~:
#	�����O�l�[���̃I�v�V������, �����̔����I�v�V�����̎w������܂�. 
#	--version ��, --geometry 300x400 ��, -d host:0.0 ���ł�. 
#	�����𔺂��w��� ":" ��K���t���Ă�������. 
#
#    �I�v�V�����̎w�肪�������ꍇ, �ϐ� $OPT_?? �� non-nil ��������, ���̃I
#    �v�V�����̈������Z�b�g����܂�. 
#	-f -> $OPT_f = %TRUE
#	--geometry 300x400 -> $OPT_geometry = 300x400
#
#    usage ���g�������ꍇ��, $USAGE �� usage() ���w�肵�܂�. 
#	def usage()
#	    �c
#	end
#	$USAGE = 'usage'
#    usage ��, --help ���w�肳�ꂽ��, �Ԉ�����w����������ɕ\�����܂�. 
#
#    - �������� -- ��, ����ȍ~, �S�ăI�v�V�����̉�͂����܂���. 
#

$RCS_ID="$Header: /var/ohba/RCS/parseargs.rb,v 1.3 1994/02/15 05:16:21 ohba Exp ohba $"

load("getopts.rb")

def printUsageAndExit()
  if $USAGE
    apply($USAGE)
  end
  exit()
end

def parseArgs(argc, nopt, single_opts, *opts)
  if ((noOptions = getopts(single_opts, *opts)) == nil)
    printUsageAndExit()
  end
  if (nopt && noOptions == 0)
    printUsageAndExit()
  end
  if ($ARGV.length < argc)
    printUsageAndExit()
  end
  return noOptions
end
