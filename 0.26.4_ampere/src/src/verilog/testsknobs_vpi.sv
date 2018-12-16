module test;
  longint iterator;
  initial begin
    $sknobs_set_value("*ab*", 2);
    $sknobs_set_string("xyz", "hi");
    $display("%m.abc=%0d", $sknobs_get_value("%m.abc", 10));
    $display("%m.abc=%0d", $sknobs_get_value("%m.abc", 10));
    $display("xyz=%0s", $sknobs_get_string("xyz", "mydefaultstring"));
    iterator = $sknobs_iterate("list");
    while($sknobs_iterator_next(iterator))
        $display("next: %0s", $sknobs_iterator_get_string(iterator));
  end
endmodule // top
