# receive params for script (must be first line in file, other than comments)
Param( $workspaceFolder, $launchJsonFile, $compileSettingsFile )

# show param inputs
Write-Host "workspaceFolder: '$workspaceFolder'" -ForegroundColor DarkGray
Write-Host "launchJsonFile: '$launchJsonFile'" -ForegroundColor DarkGray
Write-Host "compileSettingsFile: '$compileSettingsFile'" -ForegroundColor DarkGray

# validate param inputs
if( -not (Test-Path $workspaceFolder))
{
    Write-Host "Invalid workspaceFolder '$workspaceFolder'. Something went wrong." -ForegroundColor Red;
    exit;
}
if( -not (Test-Path $launchJsonFile))
{
    Write-Host "Invalid launchJsonFile '$launchJsonFile'. Something went wrong." -ForegroundColor Red;
    exit;
}
if( -not (Test-Path $compileSettingsFile))
{
    Write-Host "Invalid compileSettingsFile '$compileSettingsFile'. Something went wrong." -ForegroundColor Red;
    exit;
}

# inform user of relevant information
Write-Host ('Source Code directory: ' + "${workspaceFolder}".Replace('\\', '/')) -ForegroundColor White;

# grab game directory via regex
$regexPattern = '\"cwd\"\s*:\s*\"(.*?)\"';
Write-Host "regexPattern: '$regexPattern'" -ForegroundColor DarkGray;

$regexResult = (Get-Content -Raw "$launchJsonFile" | Select-String -Pattern $regexPattern);

if( -not $regexResult.Matches -or $regexResult.Matches.Length -le 0 -or -not $regexResult.Matches.Groups -or $regexResult.Matches.Groups.Length -le 1 )
{
    Write-Host "The current working directory `"cwd`" could not be found in '$launchJsonFile', please edit the file and update it." -ForegroundColor Red;
    Write-Host "Example: `"cwd`": `"D:/Games/DungeonKeeper/`" (be sure to use forward slashes)." -ForegroundColor Red;
    exit;
}

$gameDir = $regexResult.Matches[0].Groups[1].Value.Replace('\\', '/');
Write-Host "Found current working directory (cwd): '$gameDir' in '$launchJsonFile'" -ForegroundColor White;
if( -not (Test-Path $gameDir) )
{
    Write-Host "Directory '$gameDir' invalid, make sure it exists on-disk and the path is spelled correctly." -ForegroundColor Red;
    Write-Host "Example: `"cwd`": `"D:/Games/DungeonKeeper/`" (be sure to use forward slashes)." -ForegroundColor Red;
    exit;
}
else
{
    Write-Host "Directory '$gameDir' valid, exists on-disk" -ForegroundColor DarkGray;
}

$debugFlag      = 'DEBUG=0';
$debugFlagFTest = 'FTEST_DEBUG=0';

$compileSetting = (Get-Content "$compileSettingsFile" -Raw).Trim();
if ($compileSetting -like '*DEBUG=1*')
{
    $debugFlag = 'DEBUG=1';
}
if ($compileSetting -like 'FTEST_DEBUG=*')
{
    $debugFlagFTest = 'FTEST_DEBUG=1';
}

if ($debugFlag -eq 'DEBUG=1')
{
    Write-Host 'Compiling with DEBUG=1' -ForegroundColor Yellow;
}
else
{
    Write-Host 'Compiling with DEBUG=0' -ForegroundColor Green;
}

if ($debugFlagFTest -eq 'FTEST_DEBUG=1')
{
    Write-Host 'Compiling with FTEST_DEBUG=1' -ForegroundColor Magenta;
}
wsl make all -j`nproc` $debugFlag $debugFlagFTest;
if ($?) {
    Write-Host 'Compilation successful!' -ForegroundColor Green;
}
else
{
    Write-Host 'Compilation failed!' -ForegroundColor Red;
    exit 1;
}
Copy-Item -Path "${workspaceFolder}\\bin\\*" -Destination $gameDir -Force;
