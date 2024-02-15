$FileContent = '/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef _KERNAL_VERSION_H_
#define _KERNAL_VERSION_H_

#ifdef __cplusplus
extern "C" {
#endif

#define ATOS_BUILD_TIME                             "2024-01-01,01:01"
#define ATOS_COMMIT_HEAD_ID                         "255ac3c16ac993f95ea978c7efb4fefea19e8c58"
#define ATOS_VERSION_MAJOR_NUMBER                   (0u)
#define ATOS_VERSION_MINOR_NUMBER                   (0u)
#define ATOS_VERSION_PATCH_NUMBER                   (0u)

#define ATOS_VERSION_MAJOR_NUMBER_MASK              (0x03FFu)
#define ATOS_VERSION_MAJOR_NUMBER_POS               (22u)

#define ATOS_VERSION_MINOR_NUMBER_MASK              (0x03FFu)
#define ATOS_VERSION_MINOR_NUMBER_POS               (10u)

#define ATOS_VERSION_PATCH_NUMBER_MASK              (0x0FFFu)
#define ATOS_VERSION_PATCH_NUMBER_POS               (0u)

#ifdef __cplusplus
}
#endif

#endif'

$TargetFile = 'kernal_version.h'
$PackageFile = 'package.json'

<#
.PARAMETER Environment
    Target Environment (Remote and Local.) to build.
    
.PARAMETER Version
    Target Version (Major, Minor and Patch.) to build.
#>
function BuildScript
{
    [CmdletBinding(DefaultParameterSetName='Build')]
    param(
        [Parameter(Mandatory=$false, ParameterSetName='Build')]
        [ValidatePattern('^(Remote|Local)')]
        [string] $Environment= 'Local',
        
        [Parameter(Mandatory=$false, ParameterSetName='Build')]
        [ValidatePattern('^(Major|Minor|Patch)')]
        [string] $Version = 'Patch',

        [Parameter(DontShow=$true, Mandatory=$false, ParameterSetName='Build', ValueFromRemainingArguments=$true)]
        [string[]] $CustomParameters
    )

    $Private:BuildEnvironments = @('Remote', 'Local')
    if ($Environment -match "^($($BuildEnvironments -join '|'))") 
    {
        $BuildEnvironment = $Matches[1]
    }
    else 
    {
        $errmsg = "Environment '${Environment}' does not correspond to a supported build Environment. Valid build modes are: $($BuildModes -join ', ')"
        throw [System.ArgumentException]::new($errmsg, 'Environment')
    }
    
    $Private:TargetVersions = @('Major', 'Minor', 'Patch')
    if ($Version -match "^($($TargetVersions -join '|'))") 
    {
        $TargetVersion = $Matches[1]
    }
    else 
    {
        $errmsg = "Version '${Version}' does not correspond to a supported build Version. Valid build modes are: $($TargetVersions -join ', ')"
        throw [System.ArgumentException]::new($errmsg, 'Version')
    }

    if (-not (Test-Path -Path $TargetFile))
    {
        New-Item -Path $TargetFile -ItemType File 
        $FileContent | Out-File -FilePath $TargetFile -Force -Encoding utf8
    }
	
	$ReadContent = [string](Get-Content -Path $TargetFile -Raw)
	$MatchNumber = '[0-9]+'

	$MatchMajorLine = '#define ATOS_VERSION_MAJOR_NUMBER                   \([0-9]+u\)'
	$MajorLine = ($ReadContent | Select-String -Pattern $MatchMajorLine -AllMatches).Matches.Value
	$Major = ($MajorLine | Select-String -Pattern $MatchNumber -AllMatches).Matches.Value
	$Major = [int]$Major

	$MatchMinorLine = '#define ATOS_VERSION_MINOR_NUMBER                   \([0-9]+u\)'
	$MinorLine = ($ReadContent | Select-String -Pattern $MatchMinorLine -AllMatches).Matches.Value
	$Minor = ($MinorLine | Select-String -Pattern $MatchNumber -AllMatches).Matches.Value
	$Minor = [int]$Minor
	
	$MatchPatchLine = '#define ATOS_VERSION_PATCH_NUMBER                   \([0-9]+u\)'
	$PatchLine = ($ReadContent | Select-String -Pattern $MatchPatchLine -AllMatches).Matches.Value
	$Patch = ($PatchLine | Select-String -Pattern $MatchNumber -AllMatches).Matches.Value
	$Patch = [int]$Patch

	$MatchCommitId = '[0123456789abcdef]{40}'
	$ReadCommitId = ($ReadContent | Select-String -Pattern $MatchCommitId -AllMatches).Matches.Value

	$timestamp = Get-Date -Format "yyyy-MM-dd,HH:mm"
	
    $CommitHeadID = invoke-expression -command "git rev-parse HEAD"

	if ($ReadCommitId -notmatch  $CommitHeadID)
	{
		if ($Environment -match 'Local')
		{
			$Patch++
		}
		elseif ($Environment -match 'Remote')
		{
			if ($TargetVersion -match 'Minor')
			{
				$Minor++
				$Patch = [int]0
			}
			elseif ($TargetVersion -match 'Major')
			{
				$Major++
				$Minor = [int]0
				$Patch = [int]0
			}
		}	
	}
	
	$Major = [string]$Major
	$Minor = [string]$Minor
	$Patch = [string]$Patch
	
	$OutContent = $ReadContent -replace '\d{4}-\d{2}-\d{2},\d{2}:\d{2}', $timestamp
	
	$OutContent = $OutContent -replace '[0123456789abcdef]{40}', $CommitHeadID
	
	$MatchLine = $MajorLine -replace "[0-9]", $Major
	$OutContent = $OutContent -replace $MatchMajorLine, $MatchLine
	
	$MatchLine = $MinorLine -replace "[0-9]", $Minor
	$OutContent = $OutContent -replace $MatchMinorLine, $MatchLine
	
	$MatchLine = $PatchLine -replace "[0-9]", $Patch
	$OutContent = $OutContent -replace $MatchPatchLine, $MatchLine
	
	$OutContent | Out-File -FilePath $TargetFile -Force -Encoding utf8 -NoNewline
	
	$FirmwareRelease = $Major + "." + $Minor + "." + $Patch
	Write-Host $FirmwareRelease
	
	$packageContent = Get-Content -Raw  -Path $PackageFile | ConvertFrom-Json
	
	$packageContent.version = $FirmwareRelease
	$packageContent.timestamp = $timestamp
	$packageContent.commit_id = $ReadCommitId
	
	$packageString = $packageContent | ConvertTo-Json -Depth 10
	
	$packageString | Set-Content -Path $PackageFile
}
