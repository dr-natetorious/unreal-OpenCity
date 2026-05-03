#!/usr/bin/env python3
"""
Export the shared base character FBX using a deterministic contract-preserving configuration.

Run:
  blender --background /apps/git/unreal-OpenCity/Content/Props/BaseCharacter.blend \
    --python Tools/characters/export_base_character_fbx.py
"""

import json
import os
import sys

import bpy


CONTRACT_PATH = "/apps/git/unreal-OpenCity/Tools/characters/standard_contract.json"


def load_contract(path):
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


def fail(message):
    print(f"[FAIL] {message}")
    raise SystemExit(1)


def ok(message):
    print(f"[OK] {message}")


def main():
    contract = load_contract(CONTRACT_PATH)
    base = contract["base_character"]
    armature_name = base["armature_name"]
    mesh_name = base["primary_mesh_object"]
    export_path = base["exported_fbx"]

    armature = bpy.data.objects.get(armature_name)
    if armature is None or armature.type != "ARMATURE":
        fail(f"Armature '{armature_name}' not found")

    mesh = bpy.data.objects.get(mesh_name)
    if mesh is None or mesh.type != "MESH":
        fail(f"Mesh '{mesh_name}' not found")

    export_dir = os.path.dirname(export_path)
    os.makedirs(export_dir, exist_ok=True)

    bpy.ops.object.select_all(action="DESELECT")
    armature.select_set(True)
    mesh.select_set(True)
    bpy.context.view_layer.objects.active = armature

    bpy.ops.export_scene.fbx(
        filepath=export_path,
        use_selection=True,
        object_types={"ARMATURE", "MESH"},
        axis_forward="-Y",
        axis_up="Z",
        apply_unit_scale=True,
        mesh_smooth_type="FACE",
        add_leaf_bones=False,
        use_armature_deform_only=False,
        bake_anim=False,
        use_mesh_modifiers=True,
        armature_nodetype="ROOT",
    )

    if not os.path.exists(export_path):
        fail(f"Export did not create {export_path}")

    ok(f"Exported shared base character to {export_path}")


if __name__ == "__main__":
    main()