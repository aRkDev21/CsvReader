$files = Get-ChildItem "tests\*.csv"
foreach ($f in $files) {
    Write-Host "--------------------------------" -ForegroundColor Cyan
    Write-Host "Testing file: $($f.Name)"
    .\csvreader.exe $f.FullName
}