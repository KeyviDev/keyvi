
#LATEX = latex
LATEX = pdflatex
BIBTEX = bibtex
MAKEINDEX = makeindex
#DVIPS = dvips -Pamz -Pcmz
DVIPS = dvips -Ppdf
LATEX2HTML = latex2html
PS2PDF = ps2pdf

all: tpie.pdf

MAIN_SRC = tpie.tex

OTHER_SRC = lgpl-3.0.tex  ami_reference.tex  errorcodes.tex  \
  acknowledgements.tex implementation.tex tuning.tex  \
  addexamples.tex applications.tex user_manual.tex sample_pgm.cpp 

tpie.dvi: $(MAIN_SRC) $(OTHER_SRC) tpie.ind tpie.bbl
	$(LATEX) $<
	$(LATEX) $< > /dev/null

html:	tpie.aux tpie.bbl tpie.dvi
	$(LATEX2HTML) tpie

tpie.ps: tpie.dvi
	$(DVIPS) -o $@ $<

#tpie.pdf: tpie.ps
#	$(PS2PDF) $<

tpie.pdf: $(MAIN_SRC) $(OTHER_SRC) tpie.ind tpie.bbl
	$(LATEX) $<
	$(LATEX) $<

tpie.idx: $(MAIN_SRC) $(OTHER_SRC)
	-rm -f $@
	$(LATEX) $<

tpie.ind: tpie.idx
	$(MAKEINDEX) -o $@ $< > /dev/null

tpie.aux: $(MAIN_SRC) $(OTHER_SRC)
	$(LATEX) $<

tpie.bbl: tpie.aux tpie.bib
	$(BIBTEX) $(<:.aux=)

clean:
	-rm -f tpie.log tpie.ilg tpie.blg tpie.bbl tpie.ind tpie.dvi 
	-rm -f tpie.idx tpie.aux tpie.toc tpie.ps tpie.pdf
	-rm -f *~
	-rm -rf tpie
