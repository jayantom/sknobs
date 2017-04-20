use Test;
use sknobs;

plan tests => 4;

ok(sknobs::init(\@ARGV), 0);
ok(sknobs::get_value("argument", 0), 0);
ok(sknobs::get_string("argument", "unspecified"), "unspecified");
sknobs::set_value("argument", 999);
ok(sknobs::get_dynamic_value("argument", 0), 999);
