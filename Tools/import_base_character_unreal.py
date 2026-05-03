import json
import os
import sys

import unreal


FBX_PATH = "/apps/git/unreal-OpenCity/Content/Props/Characters/SK_CityCharacter.fbx"
DEST_PATH = "/Game/Props/Characters"
DEST_NAME = "SK_CityCharacter"
MESH_ASSET_PATH = "/Game/Props/Characters/SK_CityCharacter.SK_CityCharacter"
SKELETON_PATH = "/Game/Props/Characters/SK_CityCharacter_Skeleton.SK_CityCharacter_Skeleton"
PHYSICS_ASSET_PATH = "/Game/Props/Characters/SK_CityCharacter_PhysicsAsset.SK_CityCharacter_PhysicsAsset"
CONTRACT_PATH = "/apps/git/unreal-OpenCity/Tools/characters/standard_contract.json"
SUCCESS_MARKER_PATH = "/apps/git/unreal-OpenCity/Saved/Logs/agent/base_character_import/import-success.json"


def fail(message: str) -> None:
    unreal.log_error(message)
    raise RuntimeError(message)


def load_contract() -> dict:
    with open(CONTRACT_PATH, "r", encoding="utf-8") as handle:
        return json.load(handle)


def ensure_parent_dir(path: str) -> None:
    os.makedirs(os.path.dirname(path), exist_ok=True)


def sync_attachment_sockets(mesh: unreal.SkeletalMesh, contract: dict) -> list[dict]:
    created_or_updated = []
    attachment_points = contract["base_character"].get("required_attachment_points", {})
    skeleton = mesh.skeleton
    if skeleton is None:
        fail("Imported mesh has no skeleton to receive attachment sockets")

    for socket_name, socket_spec in attachment_points.items():
        socket = mesh.find_socket(socket_name)
        if socket is None:
            socket = unreal.SkeletalMeshSocket(outer=skeleton, name=socket_name)
            socket.socket_name = socket_name
            mesh.add_socket(socket, True)

        socket.socket_name = socket_name
        socket.bone_name = socket_spec["parent_bone"]
        socket.relative_location = unreal.Vector(*socket_spec["location"])
        socket.relative_rotation = unreal.Rotator(*socket_spec["rotation"])
        socket.relative_scale = unreal.Vector(*socket_spec["scale"])
        created_or_updated.append(
            {
                "name": socket_name,
                "parent_bone": socket_spec["parent_bone"],
                "location": socket_spec["location"],
                "rotation": socket_spec["rotation"],
                "scale": socket_spec["scale"],
            }
        )

    return created_or_updated


def write_success_marker(imported_paths: list[str], sockets: list[dict]) -> None:
    ensure_parent_dir(SUCCESS_MARKER_PATH)
    with open(SUCCESS_MARKER_PATH, "w", encoding="utf-8") as handle:
        json.dump(
            {
                "fbx_path": FBX_PATH,
                "imported_object_paths": imported_paths,
                "sockets": sockets,
            },
            handle,
            indent=2,
        )


if not unreal.Paths.file_exists(FBX_PATH):
    fail(f"FBX does not exist: {FBX_PATH}")

contract = load_contract()

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

for asset_path in (MESH_ASSET_PATH, SKELETON_PATH, PHYSICS_ASSET_PATH):
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        if not unreal.EditorAssetLibrary.delete_asset(asset_path):
            fail(f"Failed to delete stale asset before import: {asset_path}")

task = unreal.AssetImportTask()
task.filename = FBX_PATH
task.destination_path = DEST_PATH
task.destination_name = DEST_NAME
task.automated = True
task.replace_existing = True
task.replace_existing_settings = True
task.save = True

options = unreal.FbxImportUI()
options.import_mesh = True
options.import_as_skeletal = True
options.import_animations = False
options.import_materials = False
options.import_textures = False
options.create_physics_asset = False
options.mesh_type_to_import = unreal.FBXImportType.FBXIT_SKELETAL_MESH

skeletal_data = options.skeletal_mesh_import_data
skeletal_data.set_editor_property("import_mesh_lo_ds", False)
skeletal_data.set_editor_property("update_skeleton_reference_pose", True)
skeletal_data.set_editor_property("use_t0_as_ref_pose", True)
skeletal_data.set_editor_property("preserve_smoothing_groups", True)
skeletal_data.set_editor_property(
    "normal_import_method",
    unreal.FBXNormalImportMethod.FBXNIM_IMPORT_NORMALS_AND_TANGENTS,
)

task.options = options
asset_tools.import_asset_tasks([task])

if not task.imported_object_paths:
    fail("Unreal import produced no imported_object_paths")

mesh = unreal.load_asset(MESH_ASSET_PATH)
if mesh is None:
    fail(f"Imported mesh could not be loaded: {MESH_ASSET_PATH}")

synced_sockets = sync_attachment_sockets(mesh, contract)

for path in task.imported_object_paths:
    unreal.EditorAssetLibrary.save_asset(path, only_if_is_dirty=False)

unreal.EditorAssetLibrary.save_asset(MESH_ASSET_PATH, only_if_is_dirty=False)
unreal.EditorAssetLibrary.save_asset(SKELETON_PATH, only_if_is_dirty=False)

write_success_marker(task.imported_object_paths, synced_sockets)

unreal.log(f"Imported base character assets: {task.imported_object_paths}")
unreal.log(f"Synchronized attachment sockets: {[socket['name'] for socket in synced_sockets]}")
