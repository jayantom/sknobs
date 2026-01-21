import sknobs::*;
module test;
  chandle iterator;
  initial begin
    sknobs::set_value("*ab*", 2);
    sknobs::set_string("xyz", "hi");
    $display("%m.abc=%0d", sknobs::get_value("%m.abc", 10));
    $display("%m.abc=%0d", sknobs::get_value("%m.abc", 10));
    $display("xyz=%0s", sknobs::get_string("xyz", "mydefaultstring"));
    sknobs::prepend("list", "pre");
    iterator = sknobs::iterate("list");
    while(sknobs::iterator_next(iterator))
        $display("next: %0s", sknobs::iterator_get_string(iterator));
  end
endmodule // top
