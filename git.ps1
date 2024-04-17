$commit_msg = Read-Host 'Commit message'

git add * 
git commit -m "$commit_msg"
git push origin master

echo "<3"
