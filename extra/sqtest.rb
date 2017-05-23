#!/usr/bin/env ruby

N = 100000000;

def gt_stack()
  mm = [];
  
  (1..N).each{|op|
    rr = rand(3);
    
    if (rr == 0)
      # push element
      elm = rand(1000000);
      mm << elm;
      puts("+ #{elm}");
    elsif (rr == 1)
      # pop element
      if (mm.empty?.!)
        elm = mm.pop;
        puts("- #{elm}");
      end
    elsif (rr == 2)
      puts("= #{mm.size}");
    end
  };
end

def gt_queue()
  mm = [];
  
  (1..N).each{|op|
    if (rand(2) == 0)
      # push element
      elm = rand(1000000);
      mm << elm;
      puts("+ #{elm}");
    else
      # pop element
      if (mm.empty?.!)
        elm = mm.shift;
        puts("- #{elm}");
      end
    end
  };
end

def main()
  if (ARGV[0] == "s")
    gt_stack;
  elsif (ARGV[0] == "q")
    gt_queue;
  end
end

main;
