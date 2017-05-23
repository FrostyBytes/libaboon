#!/usr/bin/env ruby

require("openssl");

R = Random.new(0xCAFEBABE);

B = 64;

def test_ag_()
  a = R.rand(2**64);
  puts("ag");
  puts(a);
  puts(a.to_s(16).rjust(64/4, "0"));
end

def test_id_(bits_base, bits_bignum)
  raise if ((bits_bignum % bits_base) != 0);
  a = R.rand(2**bits_bignum);
  a = a.to_s(16).rjust(bits_bignum/4, "0");
  puts("id");
  puts(bits_bignum);
  puts(a);
end

def test_cmp(bits_base, bits_bignum)
  raise if ((bits_bignum % bits_base) != 0);
  a     = R.rand(2**bits_bignum);
  b     = R.rand(2**bits_bignum);
  a_    = a.to_s(16).rjust(bits_bignum/4, "0");
  b_    = b.to_s(16).rjust(bits_bignum/4, "0");
  ab_   = ((a > b) ? ">" : ((a < b) ? "<" : "="));
  puts("cmp");
  puts(bits_bignum);
  puts(a_);
  puts(b_);
  puts(ab_);
end

def test_add(bits_base, bits_bignum)
  raise if ((bits_bignum % bits_base) != 0);
  a, b, = nil;
  loop do
    a = R.rand(2**bits_bignum);
    b = R.rand(2**bits_bignum);
    break if ((a+b) < (2**bits_bignum));
  end
  a_ = a.to_s(16).rjust(bits_bignum/4, "0");
  b_ = b.to_s(16).rjust(bits_bignum/4, "0");
  ab_ = (a+b).to_s(16).rjust(bits_bignum/4, "0");
  puts("add");
  puts(bits_bignum);
  puts(a_);
  puts(b_);
  puts(ab_);
end

def test_sub(bits_base, bits_part)
  raise if ((bits_part % bits_base) != 0);
  bits_bignum = bits_part*2;
  a     = R.rand(2**bits_part);
  b     = R.rand(2**bits_part);
  a, b, = [ a, b, ].sort.reverse;
  a_    = a.to_s(16).rjust(bits_bignum/4, "0");
  b_    = b.to_s(16).rjust(bits_bignum/4, "0");
  ab_   = (a-b).to_s(16).rjust(bits_bignum/4, "0");
  puts("sub");
  puts(bits_bignum);
  puts(a_);
  puts(b_);
  puts(ab_);
end

def test_shl(bits_base, bits_bignum)
  raise if ((bits_bignum % bits_base) != 0);
  a     = R.rand(2**bits_bignum);
  b     = R.rand(bits_bignum);
  a     = a >> b;
  a_    = a.to_s(16).rjust(bits_bignum/4, "0");
  ab_   = (a << b).to_s(16).rjust(bits_bignum/4, "0");
  puts("shl");
  puts(bits_bignum);
  puts(a_);
  puts(b);
  puts(ab_);
end

def test_shr(bits_base, bits_bignum)
  raise if ((bits_bignum % bits_base) != 0);
  a   = R.rand(2**bits_bignum);
  b   = R.rand(bits_bignum);
  a_  = a.to_s(16).rjust(bits_bignum/4, "0");
  ab_ = (a >> b).to_s(16).rjust(bits_bignum/4, "0");
  puts("shr");
  puts(bits_bignum);
  puts(a_);
  puts(b);
  puts(ab_);
end

def test_mul(bits_base, bits_bignum_small)
  raise if ((bits_bignum_small % bits_base) != 0);
  bits_bignum_large = bits_bignum_small*2;
  a   = R.rand(2**bits_bignum_small);
  b   = R.rand(2**bits_bignum_small);
  a_  = a.to_s(16).rjust(bits_bignum_small/4, "0");
  b_  = b.to_s(16).rjust(bits_bignum_small/4, "0");
  ab_ = (a*b).to_s(16).rjust(bits_bignum_large/4, "0");
  puts("mul");
  puts(bits_bignum_small);
  puts(bits_bignum_large);
  puts(a_);
  puts(b_);
  puts(ab_);
end

def test_mul_ff(bits_base, bits_bignum_small)
  raise if ((bits_bignum_small % bits_base) != 0);
  bits_bignum_large = bits_bignum_small*2;
  a   = ((2**bits_bignum_small)-1);
  b   = ((2**bits_bignum_small)-1);
  a_  = a.to_s(16).rjust(bits_bignum_small/4, "0");
  b_  = b.to_s(16).rjust(bits_bignum_small/4, "0");
  ab_ = (a*b).to_s(16).rjust(bits_bignum_large/4, "0");
  puts("mul");
  puts(bits_bignum_small);
  puts(bits_bignum_large);
  puts(a_);
  puts(b_);
  puts(ab_);
end

def justify(bits_base, x, bits)
  s = x.to_s(16);
  raise if (s.length > ((bits+4-1)/4));
  return s.rjust((((((bits)+(bits_base-1))/bits_base)*bits_base)/4), "0");
end

def test_bar(bits_base, k)
  x = R.rand(2**(k+k));
  m = R.rand(2**k) | (1 << (k-1));
  u = ((2**(2*k))/m);
  r = (x % m);
  
  x_ = justify(bits_base, x, (k+k));
  m_ = justify(bits_base, m, k);
  u_ = justify(bits_base, u, (k+1));
  #raise("\nu___=#{u}\n2**k=#{2**k}") if (u > (2**k));
  r_ = justify(bits_base, r, k);

=begin
  $stderr.puts "k=#{k}";
  $stderr.puts "x=#{x}";
  $stderr.puts "m=#{m}";
  $stderr.puts "u=#{u}";
  $stderr.puts "r=#{r}";
  $stderr.puts "x_=#{x_}";
  $stderr.puts "m_=#{m_}";
  $stderr.puts "u_=#{u_}";
  $stderr.puts "r_=#{r_}";
=end
  
  puts("bar");
  puts(k);
  puts(x_);
  puts(m_);
  puts(u_);
  puts(r_);
end

def test_exp(bits_base, k)
  b = 3;
  e = R.rand(2**k);
  m = R.rand(2**k) | (1 << (k-1));
  u = ((2**(2*k))/m);
  r = b.to_bn.mod_exp(e, m).to_i;
  
  b_ = justify(bits_base, b, k);
  e_ = justify(bits_base, e, k);
  m_ = justify(bits_base, m, k);
  #$stderr.puts("k=#{k} m=#{m} u=#{u}");
  u_ = justify(bits_base, u, (k+1));
  r_ = justify(bits_base, r, k);
  
  puts("exp");
  puts(k);
  puts(b_);
  puts(e_);
  puts(m_);
  puts(u_);
  puts(r_);
end

# A181356[n] = smallest k such that 2^(2^n) - k is a safe prime
A181356 = {2=>5,3=>29,4=>269,5=>209,6=>1469,7=>15449,8=>36113,9=>38117,10=>1093337,11=>1942289,12=>10895177,13=>43644929,14=>364486013,15=>718982153,}.freeze;

# SCHNORR[m][n] = smallest i>=0 such that q*(r-i)+1 is a prime less than 2^(2^n) where q = ((2^(2^m))-(A181356[m])) and r is floor(((2^(2^n))-1)/q) (i.e., the largest candidate)
SCHNORR =\
{
   8=>{ 9=>609, 10=>1203, 11=>1089, 12=>8569, 13=>7734,  14=>19796, 15=>39384, 16=>90074, 17=>67279, }.freeze,
   9=>{         10=>39,   11=>307,  12=>6145, 13=>14851, 14=>3397,  15=>18051, 16=>43465, 17=>nil, }.freeze,
  10=>{                   11=>781,  12=>2143, 13=>747,   14=>5251,  15=>48903, 16=>14998, 17=>nil, }.freeze,
  11=>{                             12=>2055, 13=>6169,  14=>3619,  15=>5421,  16=>28425, 17=>nil, }.freeze,
  12=>{ 9=>nil, 10=>nil,  11=>nil,  12=>nil,  13=>nil,   14=>nil,   15=>nil,   16=>nil,   17=>nil, }.freeze,
}.freeze;

def genRQP(m, n)
  q = ((2**(2**m))-(A181356[m]));
  r = (((2**(2**n))-1)/q);
  i = SCHNORR[m][n];
  raise if (i.nil?);
  r -= i;
  return [ r, q, ((q*r)+1), ];
end

def test_shn(bits_base, kq__, kp__)
  kq = kq__;
  kp = kp__;
  r, q, p, = genRQP(kq, kp);
  kq = 2**kq;
  kp = 2**kp;
  g_lo = 2;
  u = ((2**(2*kp))/p);
  y = R.rand(2**kp);
  c = R.rand(2**kq);
  s = R.rand(2**kq);
  puts("shn");
  puts(kq);
  puts(kp);
  puts(g_lo);
  $stderr.puts("kp=#{kp}, siz=#{p.to_s(16).size}");
  puts(justify(bits_base, p, kp));
  puts(justify(bits_base, u, kp+1));
  puts(justify(bits_base, y, kp));
  puts(justify(bits_base, c, kq));
  puts(justify(bits_base, s, kq));
end

NK=100;
NI=100;

NK_EXP     = 10;
NK_EXP_MUL =  5;
NI_EXP     =  5;

#test_mul(B, 128*64);
#test_mul_ff(B, 128*64);

test_shn(B, 8, 15);
exit;

(1..NI).each{ test_ag_(); };

$stderr.puts("id_"); (1..NK).each{|k| (1..NI).each{|i| test_id_(B, 64*k); }; };
$stderr.puts("cmp"); (1..NK).each{|k| (1..NI).each{|i| test_cmp(B, 64*k); }; };
$stderr.puts("add"); (1..NK).each{|k| (1..NI).each{|i| test_add(B, 64*k); }; };
$stderr.puts("sub"); (1..NK).each{|k| (1..NI).each{|i| test_sub(B, 64*k); }; };
$stderr.puts("shl"); (1..NK).each{|k| (1..NI).each{|i| test_shl(B, 64*k); }; };
$stderr.puts("shr"); (1..NK).each{|k| (1..NI).each{|i| test_shr(B, 64*k); }; };
$stderr.puts("mul"); (1..NK).each{|k| (1..NI).each{|i| test_mul(B, 64*k); }; };
$stderr.puts("bar"); (1..NK).each{|k| (1..NI).each{|i| test_bar(B, 64*k); }; };
$stderr.puts("exp"); (1..NK_EXP).each{|k| k = k*NK_EXP_MUL; (1..NI_EXP).each{|i| test_exp(B, 64*k); $stderr.puts("k=#{k} i=#{i}"); }; };
