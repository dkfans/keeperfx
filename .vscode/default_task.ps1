# receive params for script (must be first line in file, other than comments)
Param( $workspaceFolder, $compileSettingsFile, $debugChoice )

# show param inputs
Write-Host "workspaceFolder: '$workspaceFolder'" -ForegroundColor DarkGray
Write-Host "compileSettingsFile: '$compileSettingsFile'" -ForegroundColor DarkGray
Write-Host "debugChoice: '$debugChoice'" -ForegroundColor DarkGray

# validate param inputs
if( -not (Test-Path $workspaceFolder))
{
    Write-Host "Invalid workspaceFolder '$workspaceFolder'. Something went wrong." -ForegroundColor Red;
    exit;
}
if( -not (Test-Path $compileSettingsFile))
{
    Write-Host "Invalid compileSettingsFile '$compileSettingsFile'. Something went wrong." -ForegroundColor Red;
    exit;
}
if( -not $debugChoice)
{
    Write-Host "Must provide a debug choice. Something went wrong." -ForegroundColor Red;
    exit;
}

# sanitize user debug choice
$debugChoice = $debugChoice.Split()[0]
Write-Host "sanitized debugChoice as '$debugChoice'" -ForegroundColor DarkGray

# write debug choice to disk
Set-Content -LiteralPath "$compileSettingsFile" -Value "$debugChoice"
Write-Host "Wrote '$debugChoice' to '$compileSettingsFile'" -ForegroundColor White

# inform user of relevant information
if ($debugChoice -like '*DEBUG=1')
{
    Write-Host 'Debug symbols will be included in the next executable you compile. Crash reports will have more information.' -ForegroundColor Yellow;
}
else
{
    Write-Host 'Debug symbols will NOT be included in the next executable you compile. Crash reports will be have less information.' -ForegroundColor Green;
}
if ($debugChoice -like 'FTEST_DEBUG=*')
{
    Write-Host 'Functional Tests will be enabled in the next executable you compile. DO NOT USE THIS EXECUTABLE FOR PUBLICLY RELEASED BUILDS!' -ForegroundColor Magenta;
}

# wipe stale obj directory
$objDir = "${workspaceFolder}\obj\";
if (Test-Path $objDir)
{
    Write-Host "Wiping outdated files in '$objDir'";
    Remove-Item -Path "$objDir\*" -Force -Recurse;
}
