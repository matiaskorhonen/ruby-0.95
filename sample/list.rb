# Linked list example
class MyElem
  # ���֥��������������˼�ưŪ�˸ƤФ��᥽�å�
  def initialize(item)
    # @�ѿ��ϥ��󥹥����ѿ�(������פ�ʤ�)
    @data = item
    @next = nil
  end

  def data
    @data
  end

  def next
    @next
  end

  # ��obj.data = val�פȤ����Ȥ��˰��ۤ˸ƤФ��᥽�å�
  def next=(new)
    @next = new
  end
end

class MyList
  def add_to_list(obj)
    elt = MyElem.new(obj)
    if @head
      @tail.next = elt
    else
      @head = elt
    end
    @tail = elt
  end

  def each
    elt = @head
    while elt
      yield elt
      elt = elt.next
    end
  end

  # ���֥������Ȥ�ʸ������Ѵ�����᥽�å�
  # ��������������print�Ǥ�ɽ�����Ѥ��
  def to_s
    str = "<MyList:\n";
    for elt in self
      # ��str = str + elt.data.to_s + "\n"�פξ�ά��
      str += elt.data.to_s + "\n"
    end
    str += ">"
    str
  end
end

class Point
  def initialize(x, y)
    @x = x; @y = y
    self
  end

  def to_s
    sprintf("%d@%d", @x, @y)
  end
end

# ����ѿ���`$'�ǻϤޤ롥
$list1 = MyList.new
$list1.add_to_list(10)
$list1.add_to_list(20)
$list1.add_to_list(Point.new(2, 3))
$list1.add_to_list(Point.new(4, 5))
$list2 = MyList.new
$list2.add_to_list(20)
$list2.add_to_list(Point.new(4, 5))
$list2.add_to_list($list1)

# ۣ��Ǥʤ��¤�᥽�åɸƤӽФ��γ�̤Ͼ�ά�Ǥ���
print "list1:\n", $list1, "\n"
print "list2:\n", $list2, "\n"
