# Desktop helper: find SF4/sf4e windows, click MessageBox buttons, capture screenshots.
param(
    [ValidateSet('list', 'click-yes', 'click-no', 'click-cancel', 'screenshot', 'host-flow')]
    [string]$Action = 'list',
    [string]$OutDir = "c:\Users\ptwar\Desktop\SF4e\debug-screenshots"
)

Add-Type -AssemblyName System.Windows.Forms, System.Drawing
Add-Type @"
using System; using System.Runtime.InteropServices; using System.Text;
public class Sf4eUI {
  public const int BM_CLICK = 0x00F5;
  public const int WM_COMMAND = 0x0111;
  public const int IDYES = 6; public const int IDNO = 7; public const int IDCANCEL = 2;
  [DllImport("user32.dll")] public static extern bool EnumWindows(EnumWindowsProc f, IntPtr l);
  public delegate bool EnumWindowsProc(IntPtr hWnd, IntPtr lParam);
  [DllImport("user32.dll")] public static extern int GetWindowText(IntPtr hWnd, StringBuilder s, int n);
  [DllImport("user32.dll")] public static extern int GetClassName(IntPtr hWnd, StringBuilder s, int n);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr hWnd);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hWnd);
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr hWnd, out RECT r);
  [DllImport("user32.dll")] public static extern IntPtr FindWindowEx(IntPtr parent, IntPtr after, string cls, string title);
  [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr hWnd, uint msg, IntPtr w, IntPtr l);
  public struct RECT { public int Left, Top, Right, Bottom; }
}
"@

function Get-VisibleWindows {
    $script:winList = New-Object System.Collections.Generic.List[object]
    [Sf4eUI]::EnumWindows({
        param($h, $l)
        $t = New-Object System.Text.StringBuilder 256
        $c = New-Object System.Text.StringBuilder 256
        [void][Sf4eUI]::GetWindowText($h, $t, 256)
        [void][Sf4eUI]::GetClassName($h, $c, 256)
        if ([Sf4eUI]::IsWindowVisible($h)) {
            $script:winList.Add([pscustomobject]@{ Handle = $h; Title = $t.ToString(); Class = $c.ToString() }) | Out-Null
        }
        return $true
    }, [IntPtr]::Zero) | Out-Null
    return $script:winList
}

function Find-Sf4eMessageBox {
    Get-VisibleWindows | Where-Object { $_.Title -match 'sf4e' -and $_.Class -eq '#32770' } | Select-Object -First 1
}

function Click-MessageBoxButton($hwnd, [int]$buttonId) {
    [void][Sf4eUI]::SetForegroundWindow($hwnd)
    Start-Sleep -Milliseconds 300
    # Post WM_COMMAND with button ID (works for standard MessageBox)
    [void][Sf4eUI]::PostMessage($hwnd, [Sf4eUI]::WM_COMMAND, [IntPtr]$buttonId, [IntPtr]::Zero)
}

function Save-WindowScreenshot($procName, $label) {
    New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
    $proc = Get-Process -Name $procName -ErrorAction SilentlyContinue | Select-Object -First 1
    if (-not $proc -or -not $proc.MainWindowHandle -or $proc.MainWindowHandle -eq 0) {
        return "No window for $procName"
    }
    [void][Sf4eUI]::SetForegroundWindow($proc.MainWindowHandle)
    Start-Sleep -Milliseconds 500
    $rect = New-Object Sf4eUI+RECT
    [void][Sf4eUI]::GetWindowRect($proc.MainWindowHandle, [ref]$rect)
    $w = $rect.Right - $rect.Left; $h = $rect.Bottom - $rect.Top
    if ($w -le 0 -or $h -le 0) { return "Invalid rect for $procName" }
    $bmp = New-Object System.Drawing.Bitmap $w, $h
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.CopyFromScreen($rect.Left, $rect.Top, 0, 0, (New-Object System.Drawing.Size $w, $h))
    $path = Join-Path $OutDir "$label-$(Get-Date -Format 'HHmmss').png"
    $bmp.Save($path, [System.Drawing.Imaging.ImageFormat]::Png)
    $g.Dispose(); $bmp.Dispose()
    return $path
}

switch ($Action) {
    'list' {
        Get-VisibleWindows | Where-Object {
            $_.Title -match 'sf4e|SSFIV|Street|Launcher' -or $_.Class -eq '#32770'
        } | Format-Table Handle, Title, Class -AutoSize
    }
    'click-yes' {
        $mb = Find-Sf4eMessageBox
        if ($mb) { Click-MessageBoxButton $mb.Handle ([Sf4eUI]::IDYES); "Clicked Yes on $($mb.Title)" } else { "No sf4e MessageBox found" }
    }
    'click-no' {
        $mb = Get-VisibleWindows | Where-Object { $_.Title -match 'sf4e' -and $_.Class -eq '#32770' } | Select-Object -First 1
        if ($mb) { Click-MessageBoxButton $mb.Handle ([Sf4eUI]::IDNO); "Clicked No on $($mb.Title)" } else { "No sf4e MessageBox found" }
    }
    'click-cancel' {
        $mb = Get-VisibleWindows | Where-Object { $_.Title -match 'sf4e' -and $_.Class -eq '#32770' } | Select-Object -First 1
        if ($mb) { Click-MessageBoxButton $mb.Handle ([Sf4eUI]::IDCANCEL); "Clicked Cancel" } else { "No sf4e MessageBox found" }
    }
    'screenshot' {
        Save-WindowScreenshot 'SSFIV' 'ssfiv'
    }
    'host-flow' {
        $dist = "c:\Users\ptwar\Desktop\SF4e\dist\sf4e-netplay-team-20260525"
        Stop-Process -Name SSFIV, Launcher -Force -ErrorAction SilentlyContinue
        Start-Sleep -Seconds 2
        Start-Process -FilePath "$dist\Launcher.exe" -WorkingDirectory $dist | Out-Null
        for ($i = 0; $i -lt 20; $i++) {
            Start-Sleep -Milliseconds 500
            $mb = Find-Sf4eMessageBox
            if ($mb) { break }
        }
        if (-not $mb) { "ERROR: wizard MessageBox not found"; break }
        Click-MessageBoxButton $mb.Handle ([Sf4eUI]::IDYES) | Out-Null
        "Wizard: clicked Host (Yes)"
        Start-Sleep -Seconds 2
        $mb2 = Find-Sf4eMessageBox
        if ($mb2) { Click-MessageBoxButton $mb2.Handle ([Sf4eUI]::IDYES) | Out-Null; "Room code: clicked OK" }
        for ($t = 0; $t -lt 90; $t++) {
            Start-Sleep -Seconds 1
            $ssfiv = Get-Process -Name SSFIV -ErrorAction SilentlyContinue
            if ($ssfiv) { break }
        }
        if ($ssfiv) {
            $shot = Save-WindowScreenshot 'SSFIV' 'ssfiv-host'
            "SSFIV alive PID=$($ssfiv.Id) screenshot=$shot"
        } else {
            "SSFIV NOT RUNNING after host flow (crashed or failed to start)"
            Get-Process -Name Launcher -ErrorAction SilentlyContinue | ForEach-Object { "Launcher still PID=$($_.Id)" }
        }
    }
}
