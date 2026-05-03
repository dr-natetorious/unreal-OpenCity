import json
import os

import unreal


CONTRACT_PATH = "/apps/git/unreal-OpenCity/Tools/characters/standard_contract.json"
SUCCESS_MARKER_PATH = "/apps/git/unreal-OpenCity/Saved/Logs/agent/base_character_import/socket-sync-success.json"


def fail(message: str) -> None:
    unreal.log_error(message)
    raise RuntimeError(message)


def load_contract() -> dict:
    with open(CONTRACT_PATH, "r", encoding="utf-8") as handle:
        return json.load(handle)


def ensure_parent_dir(path: str) -> None:
    os.makedirs(os.path.dirname(path), exist_ok=True)


def load_mesh(asset_path: str) -> unreal.SkeletalMesh:
    mesh = unreal.load_asset(asset_path)
    if mesh is None:
        fail(f"Shared character mesh could not be loaded: {asset_path}")
    return mesh


def validate_import_source(mesh: unreal.SkeletalMesh, expected_fbx_path: str) -> str:
    asset_import_data = mesh.get_editor_property("asset_import_data")
    if asset_import_data is None:
        fail("Shared character mesh has no asset import data")

    imported_filename = asset_import_data.get_first_filename()
    if not imported_filename:
        fail("Shared character mesh import data has no source filename")

    normalized_import = os.path.abspath(imported_filename)
    normalized_expected = os.path.abspath(expected_fbx_path)
    if normalized_import != normalized_expected:
        fail(
            "Shared character mesh was not imported from the expected FBX: "
            f"got {normalized_import}, expected {normalized_expected}"
        )

    return normalized_import


def sync_attachment_sockets(mesh: unreal.SkeletalMesh, attachment_points: dict) -> list[dict]:
    skeleton = mesh.skeleton
    if skeleton is None:
        fail("Shared character mesh has no skeleton")

    synced = []
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

        if mesh.find_socket(socket_name) is None:
            fail(f"Socket sync did not persist {socket_name}")

        synced.append(
            {
                "name": socket_name,
                "parent_bone": socket_spec["parent_bone"],
                "location": socket_spec["location"],
                "rotation": socket_spec["rotation"],
                "scale": socket_spec["scale"],
            }
        )

    return synced


def write_success_marker(mesh_asset_path: str, imported_source: str, synced_sockets: list[dict]) -> None:
    ensure_parent_dir(SUCCESS_MARKER_PATH)
    with open(SUCCESS_MARKER_PATH, "w", encoding="utf-8") as handle:
        json.dump(
            {
                "mesh_asset_path": mesh_asset_path,
                "imported_source": imported_source,
                "synced_sockets": synced_sockets,
            },
            handle,
            indent=2,
        )


def main() -> None:
    contract = load_contract()
    base = contract["base_character"]
    mesh_asset_path = base["unreal_mesh_path"]
    expected_fbx_path = base["exported_fbx"]
    attachment_points = base.get("required_attachment_points", {})
    if not attachment_points:
        fail("Base character contract has no required_attachment_points")

    mesh = load_mesh(mesh_asset_path)
    imported_source = validate_import_source(mesh, expected_fbx_path)
    synced_sockets = sync_attachment_sockets(mesh, attachment_points)

    unreal.EditorAssetLibrary.save_asset(mesh_asset_path, only_if_is_dirty=False)
    skeleton = mesh.skeleton
    if skeleton is not None:
        unreal.EditorAssetLibrary.save_asset(skeleton.get_path_name(), only_if_is_dirty=False)

    write_success_marker(mesh_asset_path, imported_source, synced_sockets)
    unreal.log(f"Synchronized sockets for {mesh_asset_path}: {[item['name'] for item in synced_sockets]}")


main()