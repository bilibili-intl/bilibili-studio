$!
$! Generated by genbuild.py
$!
$ libname = "swig_root:[vms.o_alpha]swig.olb"
$
$ set default SWIG_ROOT:[SOURCE.MODULES1_1]
$
$ idir :=  swig_root:[source.swig]
$ idir = idir + ",swig_root:[source.doh.include]"
$ idir = idir + ",swig_root:[source.include]"
$ idir = idir + ",swig_root:[source.preprocessor]"
$
$ iflags = "/include=(''idir', sys$disk:[])"
$ oflags = "/object=swig_root:[vms.o_alpha]
$ cflags = "''oflags'''iflags'''dflags'"
$ cxxflags = "''oflags'''iflags'''dflags'"
$
$ call make swig_root:[vms.o_alpha]allocate.obj -
	"cxx ''cxxflags'" allocate.cxx
$ call make swig_root:[vms.o_alpha]browser.obj -
	"cxx ''cxxflags'" browser.cxx
$ call make swig_root:[vms.o_alpha]contract.obj -
	"cxx ''cxxflags'" contract.cxx
$ call make swig_root:[vms.o_alpha]emit.obj -
	"cxx ''cxxflags'" emit.cxx
$ call make swig_root:[vms.o_alpha]guile.obj -
	"cxx ''cxxflags'" guile.cxx
$ call make swig_root:[vms.o_alpha]java.obj -
	"cxx ''cxxflags'" java.cxx
$ call make swig_root:[vms.o_alpha]lang.obj -
	"cxx ''cxxflags'" lang.cxx
$ call make swig_root:[vms.o_alpha]main.obj -
	"cxx ''cxxflags'" main.cxx
$ call make swig_root:[vms.o_alpha]module.obj -
	"cxx ''cxxflags'" module.cxx
$ call make swig_root:[vms.o_alpha]mzscheme.obj -
	"cxx ''cxxflags'" mzscheme.cxx
$ call make swig_root:[vms.o_alpha]ocaml.obj -
	"cxx ''cxxflags'" ocaml.cxx
$ call make swig_root:[vms.o_alpha]overload.obj -
	"cxx ''cxxflags'" overload.cxx
$ call make swig_root:[vms.o_alpha]perl5.obj -
	"cxx ''cxxflags'" perl5.cxx
$ call make swig_root:[vms.o_alpha]php4.obj -
	"cxx ''cxxflags'" php4.cxx
$ call make swig_root:[vms.o_alpha]pike.obj -
	"cxx ''cxxflags'" pike.cxx
$ call make swig_root:[vms.o_alpha]python.obj -
	"cxx ''cxxflags'" python.cxx
$ call make swig_root:[vms.o_alpha]ruby.obj -
	"cxx ''cxxflags'" ruby.cxx
$ call make swig_root:[vms.o_alpha]swigmain.obj -
	"cxx ''cxxflags'" swigmain.cxx
$ call make swig_root:[vms.o_alpha]tcl8.obj -
	"cxx ''cxxflags'" tcl8.cxx
$ call make swig_root:[vms.o_alpha]typepass.obj -
	"cxx ''cxxflags'" typepass.cxx
$ call make swig_root:[vms.o_alpha]xml.obj -
	"cxx ''cxxflags'" xml.cxx
$ exit
$!
$!
$MAKE: SUBROUTINE   !SUBROUTINE TO CHECK DEPENDENCIES
$ V = 'F$Verify(0)
$! P1 = What we are trying to make
$! P2 = Command to make it
$! P3 = Source file
$! P4 - P8  What it depends on
$
$ modname = f$parse(p3,,,"name")
$ set noon
$ set message/nofacility/noident/noseverity/notext
$ libr/lis=swig_root:[vms]swiglib.tmp/full/width=132/only='modname' 'libname'
$ set message/facility/ident/severity/text
$ on error then exit
$ open/read swigtmp swig_root:[vms]swiglib.tmp
$! skip header
$ read swigtmp r
$ read swigtmp r
$ read swigtmp r
$ read swigtmp r
$ read swigtmp r
$ read swigtmp r
$ read swigtmp r
$ read swigtmp r
$ read swigtmp r
$!
$
$ read/end=module_not_found swigtmp r
$ modfound = 1
$ Time = f$cvtime(f$extract(49, 20, r))
$ goto end_search_module
$ module_not_found:
$ modfound = 0
$
$ end_search_module:
$ close swigtmp
$ delete swig_root:[vms]swiglib.tmp;*
$
$ if modfound .eq. 0 then $ goto Makeit
$
$! Time = F$CvTime(F$File(P1,"RDT"))
$arg=3
$Loop:
$       Argument = P'arg
$       If Argument .Eqs. "" Then Goto Exit
$       El=0
$Loop2:
$       File = F$Element(El," ",Argument)
$       If File .Eqs. " " Then Goto Endl
$       AFile = ""
$Loop3:
$       OFile = AFile
$       AFile = F$Search(File)
$       If AFile .Eqs. "" .Or. AFile .Eqs. OFile Then Goto NextEl
$       If F$CvTime(F$File(AFile,"RDT")) .Ges. Time Then Goto Makeit
$       Goto Loop3
$NextEL:
$       El = El + 1
$       Goto Loop2
$EndL:
$ arg=arg+1
$ If arg .Le. 8 Then Goto Loop
$ Goto Exit
$
$Makeit:
$ VV=F$VERIFY(1)
$ 'P2' 'P3'
$ VV='F$Verify(VV)
$Exit:
$ If V Then Set Verify
$ENDSUBROUTINE