# Escape Room (UE5.7)

軽量さを最優先にした一人称の脱出ゲーム。部屋・ギミック・UI・入力はすべて C++ で実行時に生成し、
使用アセットはエンジン標準の基本形状（Cube / Cylinder / Sphere）だけです。

## 起動
- `EscapeRoom.uproject` をダブルクリック → エディタで ▶ Play
- エディタを使わずに起動する場合:
  `UnrealEditor.exe EscapeRoom.uproject -game -windowed -ResX=1280 -ResY=720`
- C++ を変更したら再ビルド:
  `Engine\Build\BatchFiles\Build.bat EscapeRoomEditor Win64 Development -Project=<フルパス>\EscapeRoom.uproject`

## 操作
| キー | 動作 |
|---|---|
| WASD / 矢印キー | 移動 |
| マウス | 視点 |
| E / 左クリック | 調べる・使う・メモを閉じる |
| R | クリア後にリスタート |
| Esc | 終了 |

## 攻略（ネタバレ）
1. 机の上の「研究日誌」を読む → 番号は止まった時計が知っている
2. 西の壁の時計は 3:40 → 金庫のダイヤルを **3-4-0**
3. 金庫の中の「古い鍵」で東の戸棚を開ける → 中にメモ
4. 絵の数字を 赤→青→緑→黄 の順に → キーパッドに **7295** を入力して OK
5. 開いた扉から廊下の EXIT へ

## 軽量化のポイント
- Lumen / Nanite / 仮想シャドウマップ / レイトレーシング / 距離フィールド / Substrate を無効化
- フォワードシェーディング + FXAA、DX11 / SM5
- モーションブラー / ブルーム / AO / レンズフレア オフ、60FPS 上限 + VSync
- 影を落とすライトは 1 灯だけ、他は影なし補助光
- 部屋は Static モビリティ・Tick なし。ギミックは扉が動く間だけ Tick
- UI は UMG アセットを使わず Slate を直接構築
- 色ごとにマテリアルをキャッシュ（同じ色は同じマテリアルを共有）

## ファイル構成
| ファイル | 内容 |
|---|---|
| `Source/EscapeRoom/ERGameMode.*` | 進行管理・部屋とギミックの配置（座標や答えはここで変更可） |
| `Source/EscapeRoom/ERRoom.*` | 床・壁・家具・照明 |
| `Source/EscapeRoom/ERProps.*` | メモ・鍵・時計・絵・金庫・戸棚・扉・キーパッド・出口 |
| `Source/EscapeRoom/ERCharacter.*` | 一人称プレイヤー・入力（実行時生成の Enhanced Input） |
| `Source/EscapeRoom/ERHUD.*` | HUD（目的・所持品・時間・メモ画面・クリア画面） |
| `Source/EscapeRoom/ERCommon.*` | 形状・マテリアル・テキスト・ライト生成ヘルパー |
| `Content/Maps/EscapeRoom.umap` | 空のマップ（`Tools/create_map.py` で生成） |
