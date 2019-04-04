#!/bin/sh
git checkout gh-pages
git pull
(cd ../qdecoder/src; make cleandoc; make doc)
rm -rf doc
cp -rp ../qdecoder/doc .
cp ../qdecoder/README.md index.md
git add --all doc
git status
echo "Press ENTER to PUSH"
read x
git commit -a -m "Update doc"
git push origin gh-pages
