IPES := pipelining_memory.pdf pipelining_sequence.pdf pipelining_sequence_pull.pdf
PNGS := $(IPES:.pdf=.png)

all: $(PNGS)

$(PNGS): %.png: %.pdf
	iperender -png -resolution 117 $< $@
