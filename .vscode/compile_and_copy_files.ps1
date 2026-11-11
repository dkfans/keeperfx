# receive params for script (must be first line in file, other than comments)
Param( $workspaceFolder, $compileSettingsFile )

# show param inputs
Write-Host "workspaceFolder: '$workspaceFolder'" -ForegroundColor DarkGray
Write-Host "compileSettingsFile: '$compileSettingsFile'" -ForegroundColor DarkGray

# validate param inputs
if( -not (Test-Path $workspaceFolder))
{
    Write-Host "Invalid workspaceFolder '$workspaceFolder'. Something went wrong." -ForegroundColor Red;
    exit;
}
# inform user of relevant information
Write-Host ('Source Code directory: ' + "${workspaceFolder}".Replace('\\', '/')) -ForegroundColor White;

$gameDir = "${workspaceFolder}/.vscode/game";
if (Test-Path $gameDir) {
    if ((Get-Item -LiteralPath $gameDir).LinkType -ne 'Junction') {
        Write-Host "Path '$gameDir' exists but is not a game directory junction." -ForegroundColor Red;
        exit 1;
    }
}
else {
    Add-Type -AssemblyName System.Windows.Forms;
    $folderBrowser = New-Object System.Windows.Forms.FolderBrowserDialog;
    $folderBrowser.Description = 'Select the Dungeon Keeper game directory';
    $folderBrowser.SelectedPath = [Environment]::GetFolderPath('Desktop');
    if ($folderBrowser.ShowDialog() -ne [System.Windows.Forms.DialogResult]::OK) {
        Write-Host 'Game directory selection cancelled.' -ForegroundColor Red;
        exit 1;
    }
    New-Item -ItemType Junction -Path $gameDir -Target $folderBrowser.SelectedPath -ErrorAction Stop > $null;
}
$gameDir = @((Get-Item -LiteralPath $gameDir).Target)[0];
Write-Host "Game directory: '$gameDir'" -ForegroundColor White;


$debugFlag      = 'DEBUG=0';
$debugFlagFTest = 'FTEST_DEBUG=0';
$heavyLog       = $false;
$jobsArg = '`nproc`';

if (-not (Test-Path $compileSettingsFile)) {
    Set-Content -LiteralPath $compileSettingsFile -Value 'DEBUG=1', 'MAKE_JOBS=0', 'HEAVYLOG=0';
}
$compileSetting = (Get-Content "$compileSettingsFile" -Raw).Trim();
if ($compileSetting -match '\bDEBUG=1\b')
{
    $debugFlag = 'DEBUG=1';
}
if ($compileSetting -match '\bFTEST_DEBUG=1\b')
{
    $debugFlagFTest = 'FTEST_DEBUG=1';
}
if ($compileSetting -match '\bHEAVYLOG=1\b')
{
    $heavyLog = $true;
}
if ($compileSetting -match '\bMAKE_JOBS=(\d+)\b')
{
    $jobsValue = [int]$Matches[1];
    if ($jobsValue -gt 0)
    {
        $jobsArg = $jobsValue;
    }
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

if ($heavyLog)
{
    Write-Host 'Compiling with HEAVYLOG=1 (BFDEBUG_LEVEL=10)' -ForegroundColor Cyan;
}

Write-Host "Compiling with jobs: $jobsArg" -ForegroundColor Cyan;


$makeTarget = if ($heavyLog) { 'heavylog' } else { 'all' };
wsl bash -c "make $makeTarget -j $jobsArg $debugFlag $debugFlagFTest";
if ($?) {
    Write-Host 'Compilation successful!' -ForegroundColor Green;
}
else
{
    Write-Host 'Compilation failed!' -ForegroundColor Red;
    exit 1;
}


try {
    Copy-Item -Path "${workspaceFolder}\\bin\\*" -Destination $gameDir -Force -ErrorAction Stop;
}
catch {
    Write-Host "Failed to copy build files to '$gameDir': $($_.Exception.Message)" -ForegroundColor Red;
    exit 1;
}

