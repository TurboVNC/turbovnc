all: index.html

.PHONY: pdf
pdf: turbovnc.pdf
tex: turbovnc.tex

SRC := $(wildcard *.txt *.css *.rb *.png)
index.html: $(SRC)
turbovnc.tex: $(SRC)

index.html:
	deplate -c config.rb -m html-obfuscate-email turbovnc.txt -o $@

turbovnc.pdf: turbovnc.tex
	pdflatex $<

turbovnc.tex:
	$(RM) -f deplate.sty
	deplate -c config.rb -f latex --pdf turbovnc.txt

.PHONY: clean
clean:
	$(RM) -f index.html turbovnc.tex turbovnc.pdf turbovnc.aux \
		turbovnc.toc turbovnc.out turbovnc.lot turbovnc.lof
