use ExtUtils::MakeMaker;
WriteMakefile(
   NAME     => 'sknobs',
   MAKEFILE => 'sknobs.mk',
   INC      => '-I../c',
   LIBS     => ['-lm'],
   OBJECT   => '../c/sknobs.o sknobs_wrap.o',
   test     => {TESTS => 't/sknobs.t'},
);
