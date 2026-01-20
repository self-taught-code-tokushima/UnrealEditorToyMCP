# Unreal Editor "Toy" MCP

## 目次

- [プロジェクト概要](#プロジェクト概要)
- [プロジェクト構成](#プロジェクト構成)
- [必要な環境](#必要な環境)
- [セットアップ手順](#セットアップ手順)
- [MCP設定ガイド](#mcp設定ガイド)
  - [Claude Desktop での設定](#claude-desktop-での設定)
  - [VS Code での設定](#vs-code-での設定)
  - [Claude Code での設定](#claude-code-での設定)
- [使い方](#使い方)
- [開発手順](#開発手順)
- [トラブルシューティング](#トラブルシューティング)

## プロジェクト概要

このプロジェクトは、Unreal Engine 5 のエディターを外部から操作するための仕組みを学ぶためのものです。

### 何ができるの？

- Claude AI が Unreal Editor と通信できる
- Python で書かれた MCP サーバーが、Unreal Editor と HTTP 通信する
- Unreal Editor のプラグインが HTTP サーバーとして動作する
- Claude から Unreal Editor の状態を確認したり、操作したりできる

### どんな仕組み？

```
Claude Desktop/VS Code/Claude Code
    ↓ (MCP プロトコル)
Python MCP Server (unreal_editor_mcp)
    ↓ (HTTP REST API)
Unreal Editor Plugin (UnrealEditorMCP)
    ↓
Unreal Engine 5 Editor
```

## プロジェクト構成

```
UnrealEditorToyMCP/
├── src/                              # Python MCP サーバーのソースコード
│   └── unreal_editor_mcp/
│       ├── server.py                 # MCP サーバーのメインファイル
│       ├── connection.py             # Unreal Editor との接続管理
│       └── tools/
│           └── editor_tools.py       # Claude から使えるツール定義
│
├── Plugins/                          # Unreal Engine プラグイン
│   └── UnrealEditorMCP/              # Unreal Editor 操作用プラグイン
│       ├── Source/                   # C++ ソースコード
│       │   └── UnrealEditorMCP/
│       │       ├── Private/
│       │       │   ├── HTTP/         # HTTP サーバー実装
│       │       │   └── UnrealEditorMCPSubsystem.cpp
│       │       └── Public/
│       └── UnrealEditorMCP.uplugin   # プラグイン定義ファイル
│
├── SampleProject/                    # Unreal Engine 5 サンプルプロジェクト
│   ├── Content/                      # ゲームアセット
│   ├── Plugins/                      # プロジェクト固有のプラグイン
│   └── SampleProject.uproject        # UE5 プロジェクトファイル
│
├── scripts/                          # 開発用スクリプト
│   └── test_mcp.py                   # MCP サーバーのテストスクリプト
│
├── .mcp.json                         # Claude Code 用 MCP 設定
├── pyproject.toml                    # Python プロジェクト設定
├── Taskfile.yml                      # タスクランナー設定
└── README.md                         # このファイル
```

## 必要な環境

### ソフトウェア

- **Unreal Engine**
- **Python**
- **uv** 
- **Task**

## セットアップ手順

### 1. リポジトリのクローン

```bash
git clone https://github.com/self-taught-code-tokushima/UnrealEditorToyMCP.git
cd UnrealEditorToyMCP
```

### 2. Python 環境のセットアップ

#### uv

```bash
# uv のインストール（まだの場合）
# Windows PowerShell:
powershell -ExecutionPolicy ByPass -c "irm https://astral.sh/uv/install.ps1 | iex"

# 依存パッケージのインストール
uv sync
```

### 3. Unreal Engine プロジェクトを開く

1. `SampleProject/SampleProject.uproject` をダブルクリック
2. Unreal Editor が起動します
3. プラグイン `UnrealEditorToyMCP` が自動的に読み込まれます

### 4. MCP サーバーの動作確認

#### ローカルでテスト

```bash
# Task を使う場合
task mcp-server-test

# 直接実行する場合
uv run python scripts/test_mcp.py
```

正常に動作すると、`pong` というメッセージが返ってきます。

もし、ここで失敗する場合には、`UnrealEditorToyMCP` で使っている `3000` ポートが他のアプリにすでに使われている等の問題が出ているかもしれません。

## MCP の設定方法

### Claude Desktop での設定

Claude Desktop で MCP サーバーを使えるようにします。

#### 設定ファイルの場所

`%APPDATA%\Claude\claude_desktop_config.json`

#### 設定内容

`claude_desktop_config.json` を以下のように編集します。

```json
{
  "mcpServers": {
    "ue_editor_mcp": {
      "type": "stdio",
      "command": "uvx",
      "args": [
        "--from",
        "git+https://github.com/self-taught-code-tokushima/UnrealEditorToyMCP",
        "unreal-editor-mcp"
      ],
      "env": {}
    }
  }
}
```

#### 設定手順

1. Claude Desktop を終了する
2. 上記の設定ファイルをテキストエディタで開く
3. 既に `mcpServers` セクションがある場合は、`ue_editor_mcp` を追加
4. ファイルを保存
5. Claude Desktop を再起動
6. Claude Desktop の設定画面で MCP サーバーが認識されているか確認

### Claude Code での設定

Claude Code（CLI）で MCP サーバーを使えるようにします。

#### 設定手順

1. プロジェクトルートで以下を実行します。

```sh
claude mcp add -s project --transport stdio ue_editor_mcp -- uvx --from git+https://github.com/self-taught-code-tokushima/UnrealEditorToyMCP unreal-editor-mcp
```

2. プロジェクトルートに `.mcp.json` が作成され、上記の設定が反映されています。

#### 動作確認

```bash
# Claude Code を起動
claude

# Claude Code のプロンプトで
> MCP サーバーのステータスを確認して
```

## 使い方

### 基本的な流れ

1. Unreal Editor を起動（SampleProject を開く）
2. Claude Desktop/VS Code/Claude Code を起動
3. Claude に指示を出す

### 使用例

#### 接続確認

```
Claude にメッセージ: "Unreal Editor との接続を確認して"
```

Claude が MCP サーバー経由で Unreal Editor に ping を送り、接続状態を確認します。

#### 利用可能なツールの確認

```
Claude にメッセージ: "どんなツールが使えるか教えて"
```

Claude が現在利用可能な MCP ツールの一覧を表示します。

## 開発手順

このプロジェクトをベースに、新しい機能を追加していく方法を説明します。

### タスクランナーを使う

このプロジェクトでは [Task](https://taskfile.dev/) を使ってよく使うコマンドを管理しています。

#### 利用可能なタスク

```bash
# タスク一覧を表示
task

# または
task --list
```

主なタスク:

- `task ue-mcp-status` - Unreal Editor MCP のステータス確認
- `task ue-mcp-tools` - 利用可能なツール一覧
- `task ue-mcp-tool-ping` - ping ツールのテスト
- `task mcp-server-test` - MCP サーバーのテスト
- `task mcp-server-run` - MCP サーバーをローカルで実行
- `task mcp-server-run-github` - GitHub から MCP サーバーを実行

### 新しいツールを追加する

#### 1. Unreal Editor プラグイン側に機能を追加

Unreal Editor 側で実行したい機能を C++ で実装します。

ファイル: `Plugins/UnrealEditorMCP/Source/UnrealEditorMCP/Private/HTTP/UnrealEditorMCPHttpServer.cpp`

#### 2. MCP サーバー側にツールを追加

Python MCP サーバーに新しいツールを定義します。

ファイル: `src/unreal_editor_mcp/tools/editor_tools.py`

```python
@mcp.tool()
async def my_new_tool(param: str) -> str:
    """
    新しいツールの説明

    Args:
        param: パラメータの説明

    Returns:
        結果の説明
    """
    conn = get_connection()
    response = await conn.call_endpoint("/my-endpoint", {"param": param})
    return response
```

#### 3. テストする

```bash
# ローカルで MCP サーバーを実行
task mcp-server-run

# または Claude Code でテスト
claude
```

### デバッグ方法

#### Python MCP サーバーのログ

ログファイル: `unrealeditormcp.log`

```bash
# ログをリアルタイムで確認（PowerShell）
Get-Content -Path unrealeditormcp.log -Wait
```

#### Unreal Editor のログ

Unreal Editor のメニュー: `Window` → `Developer Tools` → `Output Log`

`LogTemp` カテゴリでフィルタするとわかりやすいかもしれません。

## トラブルシューティング

TBD

## 参考資料

- [Model Context Protocol (MCP)](https://modelcontextprotocol.io/) - MCP の公式ドキュメント
- [Python MCP Server](https://modelcontextprotocol.io/docs/develop/build-server) - Python MCP サーバーのフレームワーク
- [Unreal Engine Documentation](https://docs.unrealengine.com/) - Unreal Engine の公式ドキュメント
- [uv](https://docs.astral.sh/uv/) - Python パッケージマネージャー
- [Task](https://taskfile.dev/) - タスクランナー

---

**Happy Learning! 🎓**
