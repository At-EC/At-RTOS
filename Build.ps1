<#
.PARAMETER Environment
    Target Environment (Remote and Local.) to build.
	
.PARAMETER Version
    Target Version (Major, Minor and Patch.) to build.
#>
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

. "$PSScriptRoot\.github\script\BuildScript.ps1"

BuildScript @PSBoundParameters
