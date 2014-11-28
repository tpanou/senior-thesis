# Binary for biblatex backend {biber,bibtex}.
BACKEND = biber
# Binary for index sorting utility.
INDEX = makeindex
# Main tex document to compile.
MAIN    = main
# Presentation (beamer) file.
BEAMER  = beamer
# Directory to output files into (with trailing slash).
OUT_DIR = tmp/
# Additional parameters to pass to `xelatex'.
EXTRA   =

.PHONY: all ps

all:
	xelatex $(EXTRA) -output-directory=$(OUT_DIR) $(MAIN) && \
	xindy -L greek -C utf8 -I xindy -M $(OUT_DIR)$(MAIN) \
									-t $(OUT_DIR)$(MAIN).glg \
									-o $(OUT_DIR)$(MAIN).gls \
									   $(OUT_DIR)$(MAIN).glo && \
	$(BACKEND) --output_directory $(OUT_DIR) $(MAIN) && \
	$(INDEX) $(OUT_DIR)$(MAIN).idx && \
	xelatex $(EXTRA) -output-directory=$(OUT_DIR) $(MAIN) && \
	xelatex $(EXTRA) -output-directory=$(OUT_DIR) $(MAIN)

# .ps files are exported by Qucs. Convert them into .pdf, clipping any empty
# space around them.
# Find all .ps files and convert them into .pdf and print each matched file-name
# on the command line twice, separating them by null. Then, replace the .ps
# extension with .pdf and pass the (null-terminated) strings in pairs as
# arguments into pdfcrop. Each file-name is used twice because input and output
# files are the same.
ps:
	find -name "*.ps" -execdir ps2pdf {} \; -printf "%p\0%p\0" | \
	sed 's/.ps$\/.pdf/g' | \
	xargs -0 -n 2 --verbose pdfcrop

# Presentation
beamer:
	xelatex $(EXTRA) -output-directory=$(OUT_DIR) $(BEAMER) && \
