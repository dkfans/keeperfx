REM Bat file which deals with Mingw make bug which interprets peresec/bin/peresec as peresec /bin/peresec

REM We must shift out /bin/peresec part from command line.
SHIFT

REM Now do the actual execution of real peresec.
peresec\bin\peresec %1 %2 %3 %4 %5 %6 %7 %8 %9
