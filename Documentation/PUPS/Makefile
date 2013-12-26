.SUFFIXES: .tex .html

SRCS= AUUGN Icons Images Makefile Manpages Usenet *.html *.tex

.tex.html:
	latex2html -no_navigation  -no_images -split 1 -link 1 $<

pups.html:	pups.tex

tidy:
	rm -f .ID_MAP
	rm -f .IMG_PARAMS
	rm -f .ORIG_MAP
	rm -f image.gif
	rm -f labels.pl
	rm -f pups.log

clean:	tidy
	rm *.html

tar: pups.html tidy
	chmod -R go+rX $(SRCS)
	tar vcf ../p.tar $(SRCS)
