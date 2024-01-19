# receive params for script (must be first line in file, other than comments)
Param( $workspaceFolder, $templateSettingsFile, $templateLaunchFile, $settingsJsonFile, $launchJsonFile )

# sanitize inputs
$workspaceFolder = $workspaceFolder.Replace('\', '/')
$templateSettingsFile = $templateSettingsFile.Replace('\', '/')
$templateLaunchFile = $templateLaunchFile.Replace('\', '/')
$settingsJsonFile = $settingsJsonFile.Replace('\', '/')
$launchJsonFile = $launchJsonFile.Replace('\', '/')
$driveLetter = (Split-Path -Path $workspaceFolder -Qualifier).Split(":").Get(0)
$driveLetterLower = $driveLetter.ToLower()

# show param inputs
Write-Host "workspaceFolder: '$workspaceFolder'" -ForegroundColor DarkGray
Write-Host "templateSettingsFile: '$templateSettingsFile'" -ForegroundColor DarkGray
Write-Host "templateLaunchFile: '$templateLaunchFile'" -ForegroundColor DarkGray
Write-Host "settingsJsonFile: '$settingsJsonFile'" -ForegroundColor DarkGray
Write-Host "launchJsonFile: '$launchJsonFile'" -ForegroundColor DarkGray

# validate param inputs
if( -not (Test-Path $workspaceFolder))
{
    Write-Host "Invalid workspaceFolder '$workspaceFolder'. Something went wrong." -ForegroundColor Red
    exit
}
if( -not (Test-Path $templateSettingsFile))
{
    Write-Host "Invalid templateSettingsFile '$templateSettingsFile'. Something went wrong." -ForegroundColor Red
    exit
}
if( -not (Test-Path $templateLaunchFile))
{
    Write-Host "Invalid templateLaunchFile '$templateLaunchFile'. Something went wrong." -ForegroundColor Red
    exit
}

# inform user of relevant information
Write-Host ('Source Code directory: ' + "${workspaceFolder}") -ForegroundColor White

# import windows gui stuff
Add-Type -AssemblyName System.Windows.Forms

# check for / copy settings.json
if( Test-Path $settingsJsonFile )
{
    Write-Host ("settingsJsonFile '$settingsJsonFile' exists, skipping generation") -ForegroundColor DarkGray
}
else
{
    Copy-Item "${templateSettingsFile}" $settingsJsonFile
    Write-Host ("settingsJsonFile '$settingsJsonFile' was created.") -ForegroundColor White
}

# check for / generate / copy launch.json
if( Test-Path $launchJsonFile )
{
    Write-Host ("launchJsonFile '$launchJsonFile' exists, skipping generation") -ForegroundColor DarkGray
}
else
{
    # determine KeeperFx executable path from user dialog
    $keeperFxExecutable = ""
    $doneSelectingFilepath = $false
    :fileLoop while( -not $doneSelectingFilepath )
    {
        # prompt user with file browser dialog to locate keeperfx.exe
        Write-Host "Prompting user for KeeperFX executable filepath..." -ForegroundColor DarkGray
        $FileBrowser = New-Object System.Windows.Forms.OpenFileDialog -Property @{
            InitialDirectory = [Environment]::GetFolderPath('Desktop')
            Filter = 'Installed KeeperFX executable|keeperfx.exe'
            Title = "Please select the installed Game executable (keeperfx.exe)"
        }

        $FileBrowserResult = $FileBrowser.ShowDialog((New-Object System.Windows.Forms.Form -Property @{TopMost = $true }))
        Write-Host "User selected '$FileBrowserResult'" -ForegroundColor DarkGray
        if( $FileBrowserResult -ne "OK" )
        {
            # DO NOT use this -> [System.Windows.Forms.MessageBox]::Show, because it breaks the current window focus, and consecutive windows will be in the background, confusing users
            # Instead, use custom windows form below, for a simple yes/no popup.

            $form = New-Object System.Windows.Forms.Form
            $form.Text = 'KeeperFX.exe not found'
            $form.Size = New-Object System.Drawing.Size(312,128)
            $form.AutoSize = $false
            $form.StartPosition = 'CenterScreen'
            $form.FormBorderStyle = [System.Windows.Forms.FormBorderStyle]::FixedSingle;
            $form.MinimizeBox = $false
            $form.MaximizeBox = $false

            # form icon using base64 string of 32x32 keeperfx icon
            $iconBase64 = 'iVBORw0KGgoAAAANSUhEUgAAACAAAAAgBAMAAACBVGfHAAAABGdBTUEAALGPC/xhBQAAACBjSFJNAAB6JQAAgIMAAPn/AACA6QAAdTAAAOpgAAA6mAAAF2+SX8VGAAAAG1BMVEX///8AAAD//wCAgACAgICAAAD/AADAwMD///8oIYO3AAAAAXRSTlMAQObYZgAAAAFiS0dEAIgFHUgAAAAJcEhZcwAACxMAAAsTAQCanBgAAAAHdElNRQflBRgWJxAIya1SAAAA40lEQVQoz1WRQY7DIAxF8Q1CUg5QWT1AmlnMtqp6gCj5ZLazcC6AInU/rdRjl2ZCMKysxwPMt7GmWGTqqgDWNMdCOBo6a6VuTaEQx12tfISPkojltaJzux1IRcNrM9TwplJ3i4TctU13HQBrHcb9dgf4GcAOCDPES58bciIQUS3T5P2kBEM/C+4FkHHErwZ+mrw23PAY/1Bp8EQJwverWzTg0IVTBgTmwKdegcDMiwZDBMgg/jbwkn9LN8gQILhsoPlyXmICh65K4cdAZunTQCiOwcXEKmP/Z0ZrwrDrqPPLqX4Dt+cuw4zSIHYAAAAldEVYdGRhdGU6Y3JlYXRlADIwMjEtMDUtMjRUMjI6Mzk6MDQrMDA6MDA1uUv2AAAAJXRFWHRkYXRlOm1vZGlmeQAyMDIwLTA4LTI5VDIzOjA2OjM3KzAwOjAwZ7W8mwAAAA5lWElmTU0AKgAAAAgAAAAAAAAA0lOTAAAAAElFTkSuQmCC'
            $iconBytes  = [Convert]::FromBase64String($iconBase64)
            $stream     = [System.IO.MemoryStream]::new($iconBytes, 0, $iconBytes.Length)
            $form.Icon  = [System.Drawing.Icon]::FromHandle(([System.Drawing.Bitmap]::new($stream).GetHIcon()))

            $okButton = New-Object System.Windows.Forms.Button
            $okButton.Location = New-Object System.Drawing.Point(85,55)
            $okButton.Size = New-Object System.Drawing.Size(50,25)
            $okButton.Text = 'Yes'
            $okButton.DialogResult = [System.Windows.Forms.DialogResult]::OK
            $form.AcceptButton = $okButton
            $form.Controls.Add($okButton)

            $cancelButton = New-Object System.Windows.Forms.Button
            $cancelButton.Location = New-Object System.Drawing.Point(160,55)
            $cancelButton.Size = New-Object System.Drawing.Size(50,25)
            $cancelButton.Text = 'No'
            $cancelButton.DialogResult = [System.Windows.Forms.DialogResult]::Cancel
            $form.CancelButton = $cancelButton
            $form.Controls.Add($cancelButton)

            $label = New-Object System.Windows.Forms.Label
            $label.Location = New-Object System.Drawing.Point(10,20)
            $label.Size = New-Object System.Drawing.Size(280,60)
            $label.AutoSize = $false
            $label.TextAlign = [System.Drawing.ContentAlignment]::TopCenter
            $label.Text = 'Would you like to retry selecting the installed KeeperFX Game executable?'
            $form.Controls.Add($label)
            
            $form.Topmost = $true
            $form.ActiveControl = $okButton
            $form.Add_Load({$form.Activate()})

            $result = $form.ShowDialog()

            if ($result -ne [System.Windows.Forms.DialogResult]::OK)
            {
                Write-Host "User aborted KeeperFX executable selection." -ForegroundColor Red
                Write-Host "Restart VSCode to re-open file browser, OR enter them manually inside launch.json" -ForegroundColor Red
                exit
            }
            else
            {
                Write-Host "Retrying..." -ForegroundColor DarkGray
                continue fileLoop
            }
        }
        
        # keeperfx exe filepath is validated by OpenFileDialog by default
        $keeperFxExecutable = $FileBrowser.FileName.Replace('\', '/')
        $doneSelectingFilepath = $true
    }

    Write-Host ('KeeperFX executable path: ' + "${keeperFxExecutable}") -ForegroundColor White

    # generate launch.json from template, replacing "program", "cwd" and "sourceFileMap"
    $programRegexReplace = '(\"program\"\s*:\s*\")(.*?)(\")'
    $cwdRegexReplace = '(\"cwd\"\s*:\s*\")(.*?)(\")'
    $sourceFileMapRegexReplace = '(\/\/)(\"sourceFileMap\"\s*:\s*{\s*\"\/mnt\/)(.*?)(\/\"\s*:\s*\")(.*?)(:\/\")'
    Write-Host "Generating ${launchJsonFile}" -ForegroundColor DarkGray
    $newLaunchFileContents = Get-Content ${templateLaunchFile} # read in template launch file
    $newLaunchFileContents = ($newLaunchFileContents) -replace $programRegexReplace, ('$1{0}$3' -f "${keeperFxExecutable}") # replace "program" of new launch file
    $newLaunchFileContents = ($newLaunchFileContents) -replace $cwdRegexReplace, ('$1{0}$3' -f (Split-Path -Path "${keeperFxExecutable}").Replace('\', '/')) # replace "cwd" of new launch file
    $newLaunchFileContents = ($newLaunchFileContents) -replace $sourceFileMapRegexReplace, ('$2{0}$4{1}$6' -f "${driveLetterLower}", "${driveLetter}") # replace "sourceFileMap" of new launch file

    [System.IO.File]::WriteAllLines( ${launchJsonFile}, $newLaunchFileContents) # write out new launch file
    Write-Host ("launchJsonFile '$launchJsonFile' was created.") -ForegroundColor White
}