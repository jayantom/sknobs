CFLAGS = -I ../c -I $(VERA_HOME)/lib

default: knobs.vro test.vro knobs.dl
	LD_LIBRARY_PATH=. && vera_cs knobs.vro test.vro +vera_directc=knobs.dl

knobsdc.o: knobsdc.c ../c/knobs.h

knobs.dl: knobsdc.o
	$(CC) -shared -o $@ $< -L../c -lknobs -lfl

knobs.vro: knobs.vr knobs.vri
	vera cmp $< $@

test.vro: test.vr knobs.vri
	vera cmp $< $@

clean:
	$(RM) *.vro *~ *.o *.dl test.vshell
