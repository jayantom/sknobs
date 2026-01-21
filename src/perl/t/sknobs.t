use Test;
use sknobs;

plan tests => 7;

ok(sknobs::init(\@ARGV), 0);
ok(sknobs::get_value("argument", 0), 0);
ok(sknobs::get_string("argument", "unspecified"), "unspecified");
sknobs::set_value("argument", 999);
ok(sknobs::get_dynamic_value("argument", 0), 999);
sknobs::add("test_add", "val", "perl");
sknobs::prepend("test_add", "pre_val", "perl");
ok(sknobs::get_string("test_add", "unspecified"), "val");
$i = sknobs::iterate("test_add");
@expected = ("pre_val", "val");
while (sknobs::iterator_next($i)) {
    ok(sknobs::iterator_get_string($i), shift(@expected));
}

