# random dot steraogram
# usage: rcs.rb rcs.dat

sw = 40.0	# ���̃p�^�[���̕�
dw = 78.0	# ��������� Random Character Streogram �̕�
hdw = dw / 2.0
w = 20.0	# ����̕�
h =1.0		# ��ʂƊ�ʂ̋���
d = 0.2		# �P�ʓ�����̕����オ���
ss="abcdefghijklmnopqrstuvwxyz0123456789!#$%^&*()-=\\[];'`,./"
rnd = srand()

while gets()
#  print($_)
  xr = -hdw; y = h * 1.0; maxxl = -999
  s = "";
  while xr < hdw
    x = xr * (1 + y) - y * w / 2
    i = (x / (1 + h) + sw / 2)
    if (1 < i && i < $_.length);
      c = $_[i, 1].to_i
    else
      c = 0
    end
    y = h - d * c
    xl = xr - w * y / (1 + y);
    if xl < -hdw || xl >= hdw || xl <= maxxl
      tt = rand(ss.length)
      c = ss[tt, 1]
    else
      c = s[xl + hdw, 1]
      maxxl = xl
    end
    s += c
    xr += 1
  end
  print(s, "\n")
end
      
  
    
  




  
  
  
