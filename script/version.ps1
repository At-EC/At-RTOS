$VersionFile = "atos_version.h"
$VersionPath = "../kernal/include"
$FileContent = '/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef _ATOS_VERSION_H_
#define _ATOS_VERSION_H_

#ifdef __cplusplus
extern "C" {
#endif

#define ATOS_BUILD_TIME                             "2024-01-01,01:01"

#define ATOS_VERSION_STRING                         "0.0.0.0"
#define ATOS_VERSION_PRODUCTION_RELEASE_NUMBER      (0u)
#define ATOS_VERSION_OFFICIAL_RELEASE_NUMBER        (0u)
#define ATOS_VERSION_CHANGES_NUMBER                 (0u)
#define ATOS_VERSION_CATEGORIES_NUMBER              (0u)

#define ATOS_VERSION_CATEGORIES_MASK                (0x0Fu)
#define ATOS_VERSION_CATEGORIES_POS                 (0u)

#define ATOS_VERSION_CHANGES_MASK                   (0x3FFu)
#define ATOS_VERSION_CHANGES_POS                    (4u)

#define ATOS_VERSION_OFFICIAL_RELEASE_MASK          (0x3FFu)
#define ATOS_VERSION_OFFICIAL_RELEASE_POS           (14u)

#define ATOS_VERSION_PRODUCTION_RELEASE_MASK        (0xFFu)
#define ATOS_VERSION_PRODUCTION_RELEASE_POS         (24u)

#ifdef __cplusplus
}
#endif

#endif'

$TargetFile = Join-Path $VersionPath -ChildPath $VersionFile

if (-not (Test-Path -Path $TargetFile))
{
    New-Item -Path $TargetFile -ItemType File
    $FileContent | Out-File -Append $TargetFile
}

$ReadContent = [string](Get-Content -Path $TargetFile)
$MatchNumber = '[0-9]+'

$MatchChanges = '#define ATOS_VERSION_CHANGES_NUMBER                 \([0-9]+u\)'
$Range = ($ReadContent | Select-String -Pattern $MatchChanges -AllMatches).Matches.Value

$Changes = ($Range | Select-String -Pattern $MatchNumber -AllMatches).Matches.Value
$Changes = [int]$Changes
$Changes++
$Changes = [string]$Changes

$MatchProduction = '#define ATOS_VERSION_PRODUCTION_RELEASE_NUMBER      \([0-9]+u\)'
$Range = ($ReadContent | Select-String -Pattern $MatchProduction -AllMatches).Matches.Value
$Production = ($Range | Select-String -Pattern $MatchNumber -AllMatches).Matches.Value

$MatchProduction = '#define ATOS_VERSION_OFFICIAL_RELEASE_NUMBER        \([0-9]+u\)'
$Range = ($ReadContent | Select-String -Pattern $MatchProduction -AllMatches).Matches.Value
$Official = ($Range | Select-String -Pattern $MatchNumber -AllMatches).Matches.Value

$MatchProduction = '#define ATOS_VERSION_CATEGORIES_NUMBER              \([0-9]+u\)'
$Range = ($ReadContent | Select-String -Pattern $MatchProduction -AllMatches).Matches.Value
$Categories = ($Range | Select-String -Pattern $MatchNumber -AllMatches).Matches.Value

$Generate = '"' + $Production + "." + $Official + "." + $Changes + "." + $Categories + '"'

$FileContent | Out-File -Append $TargetFile

$Output = '/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */
 
#ifndef _ATOS_VERSION_H_
#define _ATOS_VERSION_H_

#ifdef __cplusplus
extern "C" {
#endif

'
$timestamp = Get-Date -Format "yyyy-mm-dd,HH:mm"
$Output += '#define ATOS_BUILD_TIME                             "2024-01-01,01:01"

' -replace '\d{4}-\d{2}-\d{2},\d{2}:\d{2}', $timestamp

$Output += '#define ATOS_VERSION_STRING                         '
$Output += $Generate + '
'
$Output += ''

$Output += '#define ATOS_VERSION_PRODUCTION_RELEASE_NUMBER      (1u)
' -replace "[0-9]", $Production
$Output += '#define ATOS_VERSION_OFFICIAL_RELEASE_NUMBER        (2u)
' -replace "[0-9]", $Official
$Output += '#define ATOS_VERSION_CHANGES_NUMBER                 (3u)
' -replace "[0-9]", $Changes
$Output += '#define ATOS_VERSION_CATEGORIES_NUMBER              (4u)
' -replace "[0-9]", $Categories

$Output += '
#define ATOS_VERSION_CATEGORIES_MASK                (0x0Fu)
#define ATOS_VERSION_CATEGORIES_POS                 (0u)

#define ATOS_VERSION_CHANGES_MASK                   (0x3FFu)
#define ATOS_VERSION_CHANGES_POS                    (4u)

#define ATOS_VERSION_OFFICIAL_RELEASE_MASK          (0x3FFu)
#define ATOS_VERSION_OFFICIAL_RELEASE_POS           (14u)

#define ATOS_VERSION_PRODUCTION_RELEASE_MASK        (0xFFu)
#define ATOS_VERSION_PRODUCTION_RELEASE_POS         (24u)

#ifdef __cplusplus
}
#endif

#endif'

$Output | Out-File -FilePath $TargetFile -Force
