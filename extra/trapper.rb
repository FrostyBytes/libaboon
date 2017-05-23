#!/usr/bin/env ruby

system("rm -f /tmp/gdb_i /tmp/gdb_o");
system("mkfifo /tmp/gdb_i /tmp/gdb_o");
system("killall -w gdb");
system("gdb -ex 'target remote 127.0.0.1:7117' </tmp/gdb_i >/tmp/gdb_o &");

$fo = File.open("/tmp/gdb_i", "wb");
$fi = File.open("/tmp/gdb_o", "rb");

def gdb_put(l)
  $stderr.puts("gdb_out: #{l}");
  $fo.write((l + "\n"));
  $fo.flush;
end

gdb_put("b lb_breakpoint");
gdb_put("set can-use-hw-watchpoints 0");
gdb_put("c");

$state = 0;
$watchpoint_ctr = 1; # start at 1 so the 1st watchpoint gets 2, etc. this is because the breakpoint gets 1.
$watchpoint_nr = {};

def gdb_got_lb_breakpoint(x)
  $stderr.puts("lb_breakpoint #{x}");
  
  if ((x == 1) || (x == 2))
    raise if ($state != 0);
    $state = x;
    gdb_put("c");
  else
    if ($state == 1)
      raise if ($watchpoint_nr[x].nil?.!);
      $watchpoint_nr[x] = ($watchpoint_ctr += 1);
      gdb_put("watch *#{x}");
    elsif ($state == 2)
      nr = $watchpoint_nr[x];
      if (nr.nil?)
        $stderr.puts("WARNING: ignoring delete request for watchpoint #{x} not previously established");
      else
        gdb_put("delete #{nr}");
        $watchpoint_nr.delete(x);
      end
    end
    $state = 0;
    gdb_put("info breakpoints");
    $stderr.puts("INFO: my current notion: #{$watchpoint_nr}");
    gdb_put("c");
  end
end

def gdb_got(l)
  t = l.split;
  
  if ((t[0] == "Breakpoint") && (t[2] == "lb_breakpoint"))
    gdb_got_lb_breakpoint(t[3][3..-2].to_i);
  end
  
  if (l.start_with?("Program received signal ") || l.start_with?("Watchpoint "))
    gdb_put("disconnect");
    gdb_put("q");
    sleep;
  end
end

while (l = $fi.gets)
  l = l.strip;
  
  $stderr.puts("gdb_inp: #{l}");
  
  gdb_got(l);
end
