# PXF Documentation

This directory contains the book and markdown source for the PXF docs. You can build the markdown into HTML output using [Diplodic](https://diplodoc.com/).  

## Building the Documentation locally

0. install nodejs
1. Run
   ```bash
   npx -y @diplodoc/cli@next -i ./docs -o ./docs-html --output-format html && npx http-server ./docs-html -p 5005
   ```
2. local version of the documentation should be available for viewing at [http://localhost:5005](http://localhost:5005)
