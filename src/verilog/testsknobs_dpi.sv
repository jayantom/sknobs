import sknobs::*;
module test;
  chandle iterator;
  initial begin
    sknobs::value#(int)::set("*ab*", 2);
    sknobs::value#(string)::set("xyz", "hi");
    $display("%m.abc=%0d", sknobs::value#(int)::get("%m.abc", 10));
    $display("%m.abc=%0d", sknobs::value#(int)::get("%m.abc", 10));
    $display("xyz=%0s", sknobs::value#(string)::get("xyz", "mydefaultstring"));
    iterator = sknobs::iterate("list");
    while(sknobs::iterator_next(iterator))
        $display("next: %0s", sknobs::iterator_get_string(iterator));
  end
endmodule // top
