# 2BinProcessSpeedComp

C++による2値化処理の実装手法ごとの速度比較を行うベンチマークプロジェクトです。
OpenCV標準実装と、自作実装（分岐あり／分岐なし／LUT／SIMD + OpenMP）を比較します。

## 概要

画像の2値化処理は単純に見えますが、
実装方法によって 実行時間に大きな差 が生じます。

本プロジェクトでは以下の実装を比較します

- 分岐あり(三項演算子)
- 分岐なし実装
- LUT (ルックアップテーブル)
- SIMD + OpenMP (自作)
- OpenCV (cv::threshold)

AVX2有効環境での実測結果を通して、
CPU最適化がどこまで効くのかを検証します。

## 実行環境
- CPU: Intel Core Ultra 7 265KF
- OS: Windows 11
- Compiler: MSVC 19.50
- Build: Release
- SIMD: AVX2
- Parallel: OpenMP

## ビルド方法
### 1. vcpkgの準備
```powershell
git clone https://github.com/microsoft/vcpkg.git C:/tools/vcpkg
cd C:/tools/vcpkg
bootstrap-vcpkg.bat
```
※ C:/tools/vcpkg は例です。
別の場所に置く場合は後述のCMAKE_TOOLCHAIN_FILEを変更してください。

### 2. リポジトリのクローン
```powershell
git clone https://github.com/shimon0724/2BinProcessSpeedComp.git
cd 2BinProcessSpeedComp
```

### 3. CMake Presetを使用してビルド
Visual Studioを使う場合はリポジトリのルートフォルダを開き、画面上部の構成選択から
vcpkg-x64-release
を選択してビルドを開始してください。
※ 初回ビルド時にvcpkgが自動でOpenCVをインストールします。
※ .slnまたは.slnxファイルは作成されません（CMakeプロジェクトの正常な挙動です）

### 4. コマンドラインでビルドする場合
```powershell
cmake -S . -B out/build/vcpkg-x64-release -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=C:/tools/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows
cmake --build out/build/vcpkg-x64-release
```
### 5. 実行
```powershell
out/build/vcpkg-x64-release/2BinProcessSpeedComp/2BinProcessSpeedComp.exe
```

## Notes
- OpenCVのバイナリはGitHubには含まれません
- 依存ライブラリはvcpkgにより自動取得されます
- out/ディレクトリは.gitignoreされています

## vcpkgが見つからない場合
CMAKE_TOOLCHAIN_FILEのパスを自分のvcpkgの場所に合わせて修正してください。
```powershell
CMAKE_TOOLCHAIN_FILE not found
```

## 詳細はこちらで解説しています
 [OpenCVのcv::thresholdはなぜ高速なのか？ 自前の2値化を最適化して速度比較してみた](https://shimons-labo.com/opencv%e3%81%aecvthreshold%e3%81%af%e3%81%aa%e3%81%9c%e9%ab%98%e9%80%9f%e3%81%aa%e3%81%ae%e3%81%8b%e8%87%aa%e5%89%8d%e3%81%ae2%e5%80%a4%e5%8c%96%e3%82%92%e6%9c%80%e9%81%a9%e5%8c%96%e3%81%97%e3%81%a6/)