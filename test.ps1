Add-Type -AssemblyName System.Drawing
$img = [System.Drawing.Image]::FromFile('assets/textures/Fire Mario/walk/fireMario.jpeg')
Write-Host $img.Width $img.Height
$img.Dispose()
