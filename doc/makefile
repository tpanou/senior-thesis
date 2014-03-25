# Binary for biblatex backend {biber,bibtex}.
BACKEND = biber
# Binary for index sorting utility.
INDEX = makeindex
# Main tex document to compile.
MAIN    = main
# Directory to output files into.
OUT_DIR = tmp/
# Additional parameters to pass to `xelatex'.
EXTRA   = -synctex=1

.PHONY: all

all:
	xelatex $(EXTRA) -output-directory=$(OUT_DIR) $(MAIN) && \
	$(BACKEND) --output_directory $(OUT_DIR) $(MAIN) && \
	$(INDEX) $(OUT_DIR)$(MAIN).idx && \
	xelatex $(EXTRA) -output-directory=$(OUT_DIR) $(MAIN) && \
	xelatex $(EXTRA) -output-directory=$(OUT_DIR) $(MAIN)
