function succeeded() {
  if [ $? -ne 0 ]; then
    echo "Try again, lol" 
    exit 1
  fi
}

sh ./tools/build_docs.sh
succeeded
git add .
succeeded
git commit -m "$*" 
succeeded
git push
succeeded
