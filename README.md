# ビルド方法

MacOS 環境:

```sh
cmake -G Xcode -B build .
cmake --build build --config Release
```

# インストール方法

`build/MyJuceUnityPlugin_artefacts/Release/Unity/MyJuceUnityPlugin.bundle` を目的の Unity プロジェクトの `Assets/Plugins` ディレクトリ以下にコピーする。
