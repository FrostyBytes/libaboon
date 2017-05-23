function f()
{
  cat <<'EOF'
amp = IO.read("/tmp/stdamp");

STACKS = {};

current_stack = nil;

amp.each_line{|l|
  l = l.strip;
  
  if (l.start_with?("lbi_print_stack for"))
    th = l.split[2].to_i;
    current_stack = [];
    STACKS[th] = [] if (STACKS[th].nil?);
    STACKS[th] << current_stack;
    next;
  end
  
  if (l == "done")
    current_stack = nil;
  end
  
  if (current_stack.nil?.!)
    current_stack << l;
  end
};

STACKS.each{|k, v|
  p [ "z=", v.size ]
  
  v[1..-1].each_slice(2){|a,b|
    if (b.nil?.!)
      if (a != b)
        File.open("/tmp/1", "wb"){|f| f.puts a; };
        File.open("/tmp/2", "wb"){|f| f.puts b; };
        raise;
      end
    end
  };
};
EOF
}

ruby -e "$(f)"
