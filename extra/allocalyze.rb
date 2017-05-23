#!/usr/bin/env ruby

def killpars(x)
  return x.gsub("(", "").gsub(")", "");
end

IO.read("/tmp/1").split("\n").each{|line|
  if (line.start_with?("lb_alloc: new: "))
    addr = line.split[2].to_i;
    size = killpars(line.split[3]).to_i;
  end
  
  if (line.start_with?("lb_alloc: old: "))
    addr = line.split[2].to_i;
    size = killpars(line.split[3]).to_i;
  end
  
  if (line.start_with?("lb_alloc_free: "))
    addr = line.split[1].to_i;
    size = killpars(line.split[2]).to_i;
  end
  
  if (line.start_with?("lb_alloc_direct_free: "))
    addr = line.split[1].to_i;
  end
  
  if (line.start_with?("lb_alloc_direct_alloc: old: "))
    addr = line.split[2].to_i;
  end
};
