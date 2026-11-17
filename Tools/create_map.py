# 空のマップ /Game/Maps/EscapeRoom を作成する（部屋は実行時に C++ が生成）
import unreal

path = "/Game/Maps/EscapeRoom"
if unreal.EditorAssetLibrary.does_asset_exist(path):
    unreal.log("Map already exists: " + path)
else:
    world = unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
    ok = unreal.EditorLoadingAndSavingUtils.save_map(world, path)
    unreal.log("Created map %s : %s" % (path, ok))
