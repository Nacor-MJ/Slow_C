$commit_msg = Read-Host 'Commit message'

if ($commit_msg -eq '') {
    return 
}

git add * 
git commit -m "$commit_msg"
git push origin master

echo "<3"
