default:

# Verilog simulation will also depend on $(PREFIX)/$(ARCH)/lib/libsknobs.so

install: sknobs.sv
	install -D sknobs.sv $(PREFIX)/lib/verilog/sknobs.sv

clean:
	$(RM) *.o *.so *.a *~
	$(RM) -r simv*
	$(RM) -r work csrc
	$(RM) vsim_stacktrace.vstf transcript vsim.wlf
	$(RM) vcs.key

other: testvcs testquesta

# Synopsys VCS
testvcs: testsknobs_vpi.sv sknobs.tab sknobs_vcs.so
	vcs +vcs+lic+wait -sverilog +acc -o simv -P sknobs.tab sknobs_vcs.so testsknobs_vpi.sv
	./simv -q +vcs+lic+wait

sknobs_vcs_vpi.o: sknobsvpi.c
	$(CC) -Wall -c -m32 -o $@ -I$(VCS_HOME)/include -I../c $<

sknobs_vcs.so: sknobs_vcs_vpi.o
	gcc -m32 -shared  $< ../c/libsknobs.a -o $@

# Mentor QUESTA
# Works with 6.5b.  You need libsknobs.so in your LD_LIBRARY_PATH for vsim to
# link.
testquesta_vpi: sknobs_questa_vpi.so testsknobs_vpi.sv
	vlib work
	vlog testsknobs_vpi.sv
	vsim -quiet -c test -pli sknobs_questa_vpi.so -f testsknobs.knobs -do "run; quit -f"

sknobs_questa_vpi.o: sknobsvpi.c
	$(CC) -c -fPIC -g -o $@ -I$(MODEL_HOME)/include -I../c $<

# TBD use -rpath to specify installed path to libsknobs.so.
sknobs_questa_vpi.so: sknobs_questa_vpi.o ../c/libsknobs.a
	$(CC) -shared -Bsymbolic -g $< -L../c -lsknobs -o $@

testquesta_dpi: testsknobs_dpi.sv ../c/libsknobs.so sknobs.sv
	vlib work
	vlog sknobs.sv testsknobs_dpi.sv
	vsim -quiet -c test -sv_lib ../c/libsknobs -do "run; quit -f"

