#requires -Version 5.1
[CmdletBinding()]
param(
    [Parameter(Mandatory)][string]$Definition,
    [Parameter(Mandatory)][string]$ExamplesRoot,
    [Parameter(Mandatory)][string]$SdkCmake,
    [Parameter(Mandatory)][string]$Validator,
    [Parameter(Mandatory)][string]$LayoutHeader,
    [Parameter(Mandatory)][string]$HostExecutable,
    [Parameter(Mandatory)][string]$NativeIncludeRoot,
    [Parameter(Mandatory)][string]$OutputRoot
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
foreach ($name in @('Definition', 'ExamplesRoot', 'SdkCmake', 'Validator', 'LayoutHeader', 'HostExecutable', 'NativeIncludeRoot')) {
    $value = Get-Variable -Name $name -ValueOnly
    Set-Variable -Name $name -Value (Resolve-Path -LiteralPath $value).ProviderPath
}
if (-not [IO.Path]::IsPathRooted($OutputRoot)) { $OutputRoot = Join-Path (Get-Location).ProviderPath $OutputRoot }
$definitionData = Get-Content -Raw -LiteralPath $Definition | ConvertFrom-Json
$expected = $definitionData.sources.examples.commit
$actual = (& git -C $ExamplesRoot rev-parse HEAD).Trim()
if ($LASTEXITCODE -ne 0 -or $actual -ne $expected) { throw 'Checkout dos exemplos difere do commit fixado' }
$dirty = @(& git -C $ExamplesRoot status --porcelain --untracked-files=no)
if ($LASTEXITCODE -ne 0 -or $dirty.Count -ne 0) { throw 'Exemplos contêm alterações locais' }
$layout = [regex]::Match((Get-Content -Raw -LiteralPath $LayoutHeader), 'LINKSPAN_OOT_LAYOUT_ID "([0-9a-f]{64})"').Groups[1].Value
if ($layout -ne $definitionData.host.layout) { throw 'SDK não corresponde ao layout do candidato' }
foreach ($path in @($SdkCmake, $Validator)) {
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { throw "Entrada ausente: $path" }
}
if ((Get-FileHash -Algorithm SHA256 -LiteralPath $HostExecutable).Hash.ToLowerInvariant() -ne $definitionData.host.executable_sha256) {
    throw 'Executável do host difere do candidato'
}
$root = [IO.Path]::GetFullPath($OutputRoot)
if (Test-Path -LiteralPath $root) { throw 'Use uma pasta de saída nova para os demos' }
[IO.Directory]::CreateDirectory($root) | Out-Null
$packageRoot = Join-Path $root 'packages'
[IO.Directory]::CreateDirectory($packageRoot) | Out-Null
$log = Join-Path $root 'build.log'
$previous = Get-Location
$processDirectory = [Environment]::CurrentDirectory
$packageHashes = [ordered]@{}

function Invoke-Checked {
    param([string]$Executable, [string[]]$Arguments)
    $previousPreference = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    try {
        $lines = & $Executable @Arguments 2>&1
        $code = $LASTEXITCODE
        $lines | Out-File -LiteralPath $log -Append -Encoding utf8
    } finally { $ErrorActionPreference = $previousPreference }
    if ($code -ne 0) {
        $lines | Select-Object -Last 12 | Write-Output
        throw "$Executable falhou ($code); consulte $log"
    }
}

try {
    Set-Location $root
    [Environment]::CurrentDirectory = $root
    $built = @{}
    foreach ($demo in $definitionData.native_examples) {
        $project = $demo.project
        $build = Join-Path $root "build-$($project.Replace('/', '-'))"
        if (-not $built.ContainsKey($project)) {
            $source = Join-Path $ExamplesRoot $project
            $configure = @('-S', $source, '-B', $build, '-A', 'x64')
            if ($demo.name -eq 'native-dash') { $configure += "-DLINKSPAN_SDK_INCLUDE=$NativeIncludeRoot" }
            else { $configure += "-DOOT_NATIVE_SDK=$SdkCmake" }
            Invoke-Checked 'cmake' $configure
            Invoke-Checked 'cmake' @('--build', $build, '--config', 'Release', '--parallel', '4')
            $built[$project] = $true
        }
        $provider = Join-Path $build 'mod\provider'
        $output = Join-Path $packageRoot "$($demo.name).zip"
        if ($demo.name -eq 'native-dash') {
            $stage = Join-Path $root 'dash-stage'
            [IO.Directory]::CreateDirectory((Join-Path $stage 'provider')) | Out-Null
            foreach ($name in @('manifest.toml', 'main.lua')) {
                Copy-Item -LiteralPath (Join-Path (Join-Path $ExamplesRoot $project) $name) -Destination $stage
            }
            Copy-Item -LiteralPath (Join-Path $provider 'native_dash.dll') -Destination (Join-Path $stage 'provider')
            Copy-Item -LiteralPath (Join-Path $ExamplesRoot 'LICENSE') -Destination $stage
            Copy-Item -LiteralPath (Join-Path $ExamplesRoot 'NOTICE.md') -Destination $stage
            . (Join-Path $ExamplesRoot 'tools\linkspan-zip.ps1')
            New-LinkSpanZip -Source $stage -Destination $output
            Invoke-Checked $Validator @($output)
        } elseif ($demo.name -eq 'cel-render-demo') {
            $stage = Join-Path $root 'cel-stage'
            [IO.Directory]::CreateDirectory((Join-Path $stage 'provider')) | Out-Null
            foreach ($name in @('manifest.toml', 'main.lua')) {
                Copy-Item -LiteralPath (Join-Path (Join-Path $ExamplesRoot $project) $name) -Destination $stage
            }
            Copy-Item -LiteralPath (Join-Path $provider 'linkspan_cel_render_demo.dll') -Destination (Join-Path $stage 'provider')
            Copy-Item -LiteralPath (Join-Path $ExamplesRoot 'LICENSE') -Destination $stage
            Copy-Item -LiteralPath (Join-Path $ExamplesRoot 'NOTICE.md') -Destination $stage
            . (Join-Path $ExamplesRoot 'tools\linkspan-zip.ps1')
            New-LinkSpanZip -Source $stage -Destination $output
            Invoke-Checked $Validator @($output)
        } else {
            $script = Join-Path $ExamplesRoot "tools\$($demo.packager)"
            $parameters = @{ ProviderDirectory = $provider; Validator = $Validator;
                LayoutHeader = $LayoutHeader; OutputDirectory = "package-$($demo.name)" }
            if ($demo.name -eq 'escape-demo') { $parameters.HostExecutable = $HostExecutable }
            $lines = & $script @parameters
            $lines | Out-File -LiteralPath $log -Append -Encoding utf8
            $pattern = $demo.package_pattern
            $packages = @(Get-ChildItem -LiteralPath (Join-Path $root "package-$($demo.name)") -Recurse -File |
                Where-Object { $_.Name -like $pattern })
            if ($packages.Count -ne 1) { throw "Esperado um pacote para $($demo.name), encontrados $($packages.Count)" }
            Copy-Item -LiteralPath $packages[0].FullName -Destination $output
            Invoke-Checked $Validator @($output)
        }
        $packageHashes[$demo.name] = (Get-FileHash -Algorithm SHA256 -LiteralPath $output).Hash.ToLowerInvariant()
        Write-Output "$($demo.name): build e pacote OK"
    }
    $lock = [ordered]@{ examples_commit = $expected; layout = $layout; packages = $packageHashes }
    [IO.File]::WriteAllText((Join-Path $packageRoot 'native-lock.json'),
        ($lock | ConvertTo-Json -Depth 4), (New-Object Text.UTF8Encoding($false)))
    Write-Output "Pacotes: $packageRoot"
} finally {
    Set-Location $previous
    [Environment]::CurrentDirectory = $processDirectory
}
