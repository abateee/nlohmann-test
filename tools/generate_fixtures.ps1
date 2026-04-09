Add-Type -AssemblyName System.Drawing

$root = Split-Path -Parent $PSScriptRoot
$fixturesRoot = Join-Path $root "fixtures"
New-Item -ItemType Directory -Force -Path $fixturesRoot | Out-Null

$canvasSize = 800
$center = 400
$radius = 340
$background = [System.Drawing.Color]::FromArgb(250, 250, 248)
$boardLine = [System.Drawing.Color]::FromArgb(190, 190, 190)
$dartColor = [System.Drawing.Color]::FromArgb(20, 20, 20)

function New-BoardBitmap {
    $bitmap = New-Object System.Drawing.Bitmap($canvasSize, $canvasSize)
    $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
    $graphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
    $graphics.Clear($background)

    $pen = New-Object System.Drawing.Pen($boardLine, 2)
    $graphics.DrawEllipse($pen, $center - $radius, $center - $radius, $radius * 2, $radius * 2)
    $graphics.DrawEllipse($pen, $center - ($radius * 0.95), $center - ($radius * 0.95), $radius * 1.9, $radius * 1.9)
    $graphics.DrawEllipse($pen, $center - ($radius * 0.63), $center - ($radius * 0.63), $radius * 1.26, $radius * 1.26)
    $graphics.DrawEllipse($pen, $center - ($radius * 0.58), $center - ($radius * 0.58), $radius * 1.16, $radius * 1.16)
    $graphics.DrawEllipse($pen, $center - ($radius * 0.09), $center - ($radius * 0.09), $radius * 0.18, $radius * 0.18)
    $graphics.DrawEllipse($pen, $center - ($radius * 0.04), $center - ($radius * 0.04), $radius * 0.08, $radius * 0.08)

    foreach ($degree in 0..19) {
        $angle = ($degree * 18.0) * [Math]::PI / 180.0
        $x = $center + [Math]::Sin($angle) * $radius
        $y = $center - [Math]::Cos($angle) * $radius
        $graphics.DrawLine($pen, $center, $center, $x, $y)
    }

    $graphics.Dispose()
    return $bitmap
}

function Save-Bitmap {
    param(
        [System.Drawing.Bitmap]$Bitmap,
        [string]$Path
    )

    $directory = Split-Path -Parent $Path
    New-Item -ItemType Directory -Force -Path $directory | Out-Null
    $Bitmap.Save($Path, [System.Drawing.Imaging.ImageFormat]::Png)
    $Bitmap.Dispose()
}

function Convert-BoardPointToImage {
    param(
        [double]$X,
        [double]$Y
    )

    return @{
        x = $center + ($X * $radius)
        y = $center - ($Y * $radius)
    }
}

function Add-Dart {
    param(
        [System.Drawing.Bitmap]$Bitmap,
        [double]$BoardX,
        [double]$BoardY,
        [int]$Length = 85
    )

    $tip = Convert-BoardPointToImage -X $BoardX -Y $BoardY
    $dx = $tip.x - $center
    $dy = $tip.y - $center
    $norm = [Math]::Sqrt(($dx * $dx) + ($dy * $dy))
    if ($norm -lt 0.0001) {
        $dx = 0
        $dy = -1
        $norm = 1
    }

    $ux = $dx / $norm
    $uy = $dy / $norm
    $tailX = $tip.x + ($ux * $Length)
    $tailY = $tip.y + ($uy * $Length)

    $graphics = [System.Drawing.Graphics]::FromImage($Bitmap)
    $graphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
    $pen = New-Object System.Drawing.Pen($dartColor, 7)
    $pen.StartCap = [System.Drawing.Drawing2D.LineCap]::Round
    $pen.EndCap = [System.Drawing.Drawing2D.LineCap]::Round
    $graphics.DrawLine($pen, [double]$tip.x, [double]$tip.y, [double]$tailX, [double]$tailY)
    $graphics.Dispose()
}

function Add-NoiseDots {
    param(
        [System.Drawing.Bitmap]$Bitmap
    )

    $graphics = [System.Drawing.Graphics]::FromImage($Bitmap)
    $brush = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb(50, 50, 50))
    $dots = @(
        @{x=330; y=280},
        @{x=470; y=260},
        @{x=510; y=500},
        @{x=295; y=455}
    )
    foreach ($dot in $dots) {
        $graphics.FillEllipse($brush, $dot.x, $dot.y, 6, 6)
    }
    $graphics.Dispose()
}

function Add-AmbiguousBlobs {
    param(
        [System.Drawing.Bitmap]$Bitmap
    )

    $graphics = [System.Drawing.Graphics]::FromImage($Bitmap)
    $brush = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb(40, 40, 40))
    $graphics.FillRectangle($brush, 320, 320, 18, 18)
    $graphics.FillRectangle($brush, 470, 350, 18, 18)
    $graphics.Dispose()
}

function Write-JsonFile {
    param(
        [string]$Path,
        $Object
    )

    ($Object | ConvertTo-Json -Depth 8) | Set-Content -Encoding UTF8 -Path $Path
}

function New-Scenario {
    param(
        [string]$Name,
        [hashtable]$Expected,
        [ScriptBlock]$Mutator
    )

    $scenarioDir = Join-Path $fixturesRoot $Name
    New-Item -ItemType Directory -Force -Path $scenarioDir | Out-Null

    $reference = New-BoardBitmap
    $snapshot = New-BoardBitmap
    & $Mutator $snapshot

    Save-Bitmap -Bitmap $reference -Path (Join-Path $scenarioDir "reference.png")
    Save-Bitmap -Bitmap $snapshot -Path (Join-Path $scenarioDir "snapshot.png")

    $scenario = @{
        name = $Name
        camera_id = 1
        mask = @{
            center_x = $center
            center_y = $center
            radius_px = $radius
        }
        save_debug_images = $false
    }

    $calibration = @{
        camera_id = 1
        offset_angle_deg = 0.0
        points_image = @(
            @{x = $center - $radius; y = $center - $radius},
            @{x = $center + $radius; y = $center - $radius},
            @{x = $center + $radius; y = $center + $radius},
            @{x = $center - $radius; y = $center + $radius}
        )
        points_board = @(
            @{x = -1.0; y = 1.0},
            @{x = 1.0; y = 1.0},
            @{x = 1.0; y = -1.0},
            @{x = -1.0; y = -1.0}
        )
    }

    Write-JsonFile -Path (Join-Path $scenarioDir "scenario.json") -Object $scenario
    Write-JsonFile -Path (Join-Path $scenarioDir "calibration.json") -Object $calibration
    Write-JsonFile -Path (Join-Path $scenarioDir "expected.json") -Object $Expected
}

New-Scenario -Name "single_20" -Expected @{
    event = "shot_detected"
    status = "valid"
    segment = "S20"
    score = 20
    ring = "SINGLE"
} -Mutator {
    param($bitmap)
    Add-Dart -Bitmap $bitmap -BoardX 0.0 -BoardY 0.30
}

New-Scenario -Name "double_20" -Expected @{
    event = "shot_detected"
    status = "valid"
    segment = "D20"
    score = 40
    ring = "DOUBLE"
} -Mutator {
    param($bitmap)
    Add-Dart -Bitmap $bitmap -BoardX 0.0 -BoardY 0.97
}

New-Scenario -Name "triple_20" -Expected @{
    event = "shot_detected"
    status = "valid"
    segment = "T20"
    score = 60
    ring = "TRIPLE"
} -Mutator {
    param($bitmap)
    Add-Dart -Bitmap $bitmap -BoardX 0.0 -BoardY 0.60
}

New-Scenario -Name "outer_bull" -Expected @{
    event = "shot_detected"
    status = "valid"
    segment = "OUTER_BULL"
    score = 25
    ring = "OUTER_BULL"
} -Mutator {
    param($bitmap)
    Add-Dart -Bitmap $bitmap -BoardX 0.0 -BoardY 0.07
}

New-Scenario -Name "inner_bull" -Expected @{
    event = "shot_detected"
    status = "valid"
    segment = "INNER_BULL"
    score = 50
    ring = "INNER_BULL"
} -Mutator {
    param($bitmap)
    Add-Dart -Bitmap $bitmap -BoardX 0.0 -BoardY 0.01
}

New-Scenario -Name "miss" -Expected @{
    event = "shot_invalid"
    status = "invalid"
} -Mutator {
    param($bitmap)
    Add-Dart -Bitmap $bitmap -BoardX 1.15 -BoardY 0.0
}

New-Scenario -Name "no_change" -Expected @{
    event = "shot_invalid"
    status = "invalid"
} -Mutator {
    param($bitmap)
}

New-Scenario -Name "noise_only" -Expected @{
    event = "shot_invalid"
    status = "invalid"
} -Mutator {
    param($bitmap)
    Add-NoiseDots -Bitmap $bitmap
}

New-Scenario -Name "ambiguous_contour" -Expected @{
    event = "shot_invalid"
    status = "invalid"
} -Mutator {
    param($bitmap)
    Add-AmbiguousBlobs -Bitmap $bitmap
}

$serviceConfig = @{
    execution = @{
        scenario_root = "fixtures"
        allow_single_source = $true
        debug_save_intermediates = $false
        debug_output_root = "build/debug_output"
        run_all_on_start = $true
    }
    pipeline = @{
        diff_threshold = 30
        blur_kernel_size = 5
        morph_kernel_size = 3
        min_contour_area = 40.0
        max_contour_area = 100000.0
        outlier_threshold = 0.08
        quality_floor = 0.20
    }
    backend = @{
        post_url = "http://127.0.0.1:3000/api/game301/joueurs"
        service_host = "127.0.0.1"
        service_port = 3000
        post_timeout_ms = 500
        post_retry_count = 3
    }
}

Write-JsonFile -Path (Join-Path $fixturesRoot "service_config.json") -Object $serviceConfig

