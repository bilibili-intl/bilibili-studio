using namespace System;
using namespace System.Collections.Generic;
using namespace System.IO;
using namespace System.Drawing;
using namespace System.Drawing.Drawing2D;
using namespace System.Drawing.Imaging;

[System.Reflection.Assembly]::LoadWithPartialName("System.Drawing")

$scriptDir = (Get-Location).ToString()

function RecursiveGetFileList([List[string]]$list, [string]$path)
{
    $dirList = [Directory]::GetDirectories($path)
    $fileList = [Directory]::GetFiles($path, "*.png")

    $list.AddRange($fileList)

    foreach($dir in $dirList)
    {
        if (($dir -eq ".") -or ($dir -eq ".."))
        {
            continue
        }
        RecursiveGetFileList $list $dir
    }
}

function ScaleTo2x([string]$src, [string]$dst)
{
	if ([File]::Exists($dst) -eq $False)
	{
        $dstDir = [Path]::GetDirectoryName($dst)
        if ([Directory]::Exists($dstDir) -eq $False)
        {
            [Directory]::CreateDirectory($dstDir)
        }
        Write-Host "Processing " $src
		$img = [Image]::FromFile($src)
		$imgx2 = New-Object Bitmap(($img."Width" * 2), ($img."Height" * 2))
		$g = [Graphics]::FromImage($imgx2)
		$g."SmoothingMode" = [SmoothingMode]::HighQuality
		$g."InterpolationMode" = [InterpolationMode]::HighQualityBicubic
		$g.DrawImage($img, [Rectangle]::FromLTRB(0, 0, $imgx2."Width", $imgx2."Height"), [Rectangle]::FromLTRB(0, 0, $img."Width", $img."Height"), [GraphicsUnit]::Pixel);
		$g.Flush()
		$img.Dispose()
		$g.Dispose()
		$imgx2.Save($dst, [ImageFormat]::Png)
		$imgx2.Dispose()
	}
}

$fileList = New-Object List[string]
RecursiveGetFileList $fileList ($scriptDir + "\default_100_percent")
foreach($src in $fileList)
{
    $dst = $src.Replace("default_100_percent", "default_200_percent")
    ScaleTo2x $src $dst
}
