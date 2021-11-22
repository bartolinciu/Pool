kompilator=g++
standard=-std=c++17
optymalizacja=
errors=

# debug=-g
debug= -ggdb


source=code

make: main 
	main
	
main: main.o classes.o balls.o
	$(kompilator) $(standard) $(debug) $(optymalizacja) $(errors) -o $@ $^ -lSDL2 -lSDL2_image -lSDL2_ttf

%.o : $(source)/%.cpp
	$(kompilator) $(standard) $(debug) $(optymalizacja) $(errors) -c -o $@ $^ 

.PHONY: clean

__ : final-report.pdf
	echo "done"

# concatenate report with doxygen documentation
final-report.pdf : report.pdf refman.pdf
	pdftk report.pdf ./latex/refman.pdf cat output final-report.pdf

# report compilation
report.pdf : report.tex
	pdflatex report
	pdflatex report

# doxygen compilation
refman.pdf : $(source)/classes.h $(source)/classes.cpp Doxyfile
	doxygen 
	$(MAKE) -C latex
	cp ./latex/refman.pdf .
	
# generate configuration file for doxygen
Doxyfile : 
	doxygen -g 	
	mv Doxyfile doxy
	cat doxy | sed -e 's/INPUT[ \t]\+=/INPUT = $(source)/g' | sed -e 's/UML_LOOK[ \t]\+=[ \t]\+NO/UML_LOOK = YES/g' | sed -e 's/PROJECT_NAME[ \t]\+=[ \t]\+"My Project"/PROJECT_NAME = "Fundamentals of Computer Programming: Project"/g' | sed -e 's/FULL_PATH_NAMES[ \t]\+=[ \t]\+YES/FULL_PATH_NAMES = NO/g'  | sed -s 's/GENERATE_HTML[ \t]\+=[ \t]\+YES/GENERATE_HTML = NO/g' > Doxyfile 
	rm doxy 
	
# remove temporary files
clean : 	
	for f in `ls doxy Doxyfile *.aux *.log *.out *.gz *.bib *.blg *.bbl *.o *~ refman.pdf report.pdf` ; do  if [ -f $$f ] ; then rm $$f  ; fi;   done;
	if [ -d latex ] ; then rm -r latex ; fi ;
	if [ -d html ]  ; then rm -r html  ; fi ;