# ビルド方法

MacOS 環境:

```sh
git clone https://github.com/hotwatermorning/juce-unity-plugin-test.git
cd juce-unity-plugin-test
git submodule update --init

cmake -G Xcode -B build .
cmake --build build --config Release
```

# インストール方法

`build/MyJuceUnityPlugin_artefacts/Release/Unity/MyJuceUnityPlugin.bundle` を目的の Unity プロジェクトの `Assets/Plugins` ディレクトリ以下にコピーしてください。
