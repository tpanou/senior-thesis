# Binary for biblatex backend {biber,bibtex}.
BACKEND = biber
# Main tex document to compile.
MAIN    = main
# Directory to output files into.
OUT_DIR = tmp/
# Additional parameters to pass to `xelatex'.
EXTRA   = -synctex=1

.PHONY: all

all:
	xelatex $(EXTRA) -output-directory=$(OUT_DIR) $(MAIN) && \
	$(BACKEND) --output_directory tmp $(MAIN) && \
	xelatex $(EXTRA) -output-directory=$(OUT_DIR) $(MAIN) && \
	xelatex $(EXTRA) -output-directory=$(OUT_DIR) $(MAIN)
